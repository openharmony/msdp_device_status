/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "device_manager.h"

#include <cstring>
#include <regex>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/stat.h>

#include "devicestatus_define.h"
#include "fi_log.h"
#include "device.h"
#include "input_device_cooperate_util.h"
#include "napi_constants.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceManager" };
constexpr int32_t MAX_N_EVENTS { 64 };
constexpr size_t EXPECTED_N_SUBMATCHES { 2 };
constexpr size_t EXPECTED_SUBMATCH { 1 };
} // namespace

DeviceManager::HotplugHandler::HotplugHandler(DeviceManager &devMgr)
    : devMgr_(devMgr)
{}

void DeviceManager::HotplugHandler::AddInputDevice(const std::string &devNode)
{
    devMgr_.AddInputDevice(devNode);
}

void DeviceManager::HotplugHandler::RemoveInputDevice(const std::string &devNode)
{
    devMgr_.RemoveInputDevice(devNode);
}

DeviceManager::DeviceManager()
    : hotplug_(*this)
{}

int32_t DeviceManager::Init(IContext *context)
{
    CALL_INFO_TRACE;
    CHKPR(context, RET_ERR);
    int32_t ret = context->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::OnInit, this, context));
    if (ret != RET_OK) {
        FI_HILOGE("Post sync task failed");
    }
    return ret;
}

int32_t DeviceManager::OnInit(IContext *context)
{
    CALL_INFO_TRACE;
    CHKPR(context, RET_ERR);
    context_ = context;
    monitor_.SetInputDevMgr(&hotplug_);
    enumerator_.SetInputDevMgr(&hotplug_);
    return RET_OK;
}

int32_t DeviceManager::Enable()
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::OnEnable, this));
    if (ret != RET_OK) {
        FI_HILOGE("Post sync task failed");
    }
    return ret;
}

int32_t DeviceManager::OnEnable()
{
    CALL_DEBUG_ENTER;
    int32_t ret = EpollCreate();
    if (ret != RET_OK) {
        FI_HILOGE("EpollCreate failed");
        return ret;
    }
    ret = monitor_.Enable();
    if (ret != RET_OK) {
        FI_HILOGE("Failed to enable monitor");
        goto CLOSE_EPOLL;
    }
    ret = EpollAdd(&monitor_);
    if (ret != RET_OK) {
        FI_HILOGE("EpollAdd failed");
        goto DISABLE_MONITOR;
    }
    enumerator_.ScanInputDevices();
    return RET_OK;

DISABLE_MONITOR:
    monitor_.Disable();

CLOSE_EPOLL:
    EpollClose();
    return ret;
}

int32_t DeviceManager::Disable()
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::OnDisable, this));
    if (ret != RET_OK) {
        FI_HILOGE("PostSyncTask failed");
    }
    return ret;
}

int32_t DeviceManager::OnDisable()
{
    EpollDel(&monitor_);
    monitor_.Disable();
    EpollClose();
    return RET_OK;
}

std::shared_ptr<IDevice> DeviceManager::FindInputDevice(const std::string &devPath)
{
    for (const auto &[id, dev] : devices_) {
        if (dev->GetDevPath() == devPath) {
            return dev;
        }
    }
    return nullptr;
}

int32_t DeviceManager::ParseDeviceId(const std::string &devNode)
{
    CALL_DEBUG_ENTER;
    std::regex pattern("^event(\\d+)$");
    std::smatch mr;

    if (std::regex_match(devNode, mr, pattern)) {
        if (mr.ready() && mr.size() == EXPECTED_N_SUBMATCHES) {
            return std::stoi(mr[EXPECTED_SUBMATCH].str());
        }
    }
    return -1;
}

std::shared_ptr<IDevice> DeviceManager::AddInputDevice(const std::string &devNode)
{
    CALL_INFO_TRACE;
    const std::string devPath { DEV_INPUT_PATH + devNode };
    struct stat statbuf;

    if (stat(devPath.c_str(), &statbuf) != 0) {
        FI_HILOGD("Invalid device path: %{public}s", devPath.c_str());
        return nullptr;
    }
    if (!S_ISCHR(statbuf.st_mode)) {
        FI_HILOGD("Not character device: %{public}s", devPath.c_str());
        return nullptr;
    }

    int32_t deviceId = ParseDeviceId(devNode);
    if (deviceId < 0) {
        FI_HILOGE("Parsing device name failed: %{public}s", devNode.c_str());
        return nullptr;
    }

    std::shared_ptr<IDevice> dev = FindInputDevice(devPath);
    if (dev != nullptr) {
        FI_HILOGD("Already exists: %{public}s", devPath.c_str());
        return dev;
    }

    const std::string lSysPath { SYS_INPUT_PATH + devNode };
    char rpath[PATH_MAX];
    if (realpath(lSysPath.c_str(), rpath) == nullptr) {
        FI_HILOGD("Invalid sysPath: %{public}s", lSysPath.c_str());
        return nullptr;
    }

    dev = std::make_shared<Device>(deviceId);
    dev->SetDevPath(devPath);
    dev->SetSysPath(std::string(rpath));

    if (dev->Open() != RET_OK) {
        FI_HILOGE("Unable to open \'%{public}s\'", devPath.c_str());
        return nullptr;
    }
    auto ret = devices_.emplace(dev->GetId(), dev);
    if (ret.second) {
        FI_HILOGD("\'%{public}s\' added", dev->GetName().c_str());
        OnInputDeviceAdded(dev);
        return dev;
    }
    return nullptr;
}

std::shared_ptr<IDevice> DeviceManager::RemoveInputDevice(const std::string &devNode)
{
    CALL_INFO_TRACE;
    const std::string devPath { DEV_INPUT_PATH + devNode };

    for (auto devIter = devices_.begin(); devIter != devices_.end(); ++devIter) {
        std::shared_ptr<IDevice> dev = devIter->second;
        CHKPC(dev);
        if (dev->GetDevPath() == devPath) {
            devices_.erase(devIter);
            FI_HILOGD("\'%{public}s\' removed", dev->GetName().c_str());
            dev->Close();
            OnInputDeviceRemoved(dev);
            return dev;
        }
    }
    return nullptr;
}

void DeviceManager::OnInputDeviceAdded(std::shared_ptr<IDevice> dev)
{
    FI_HILOGI("add device %{public}d: %{public}s", dev->GetId(), dev->GetDevPath().c_str());
    FI_HILOGI("  sysPath:       \"%{public}s\"", dev->GetSysPath().c_str());
    FI_HILOGI("  bus:           %{public}04x", dev->GetBus());
    FI_HILOGI("  vendor:        %{public}04x", dev->GetVendor());
    FI_HILOGI("  product:       %{public}04x", dev->GetProduct());
    FI_HILOGI("  version:       %{public}04x", dev->GetVersion());
    FI_HILOGI("  name:          \"%{public}s\"", dev->GetName().c_str());
    FI_HILOGI("  location:      \"%{public}s\"", dev->GetPhys().c_str());
    FI_HILOGI("  unique id:     \"%{public}s\"", dev->GetUniq().c_str());
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    FI_HILOGI("  Dhid:          \"%{public}s\"", dev->GetDhid().c_str());
    FI_HILOGI("  Network id:    \"%{public}s\"", dev->GetNetworkId().c_str());
    FI_HILOGI("  local/remote:  \"%{public}s\"", dev->IsRemote() ? "Remote Device" : "Local Device");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    FI_HILOGI("  is pointer:    %{public}s", dev->IsPointerDevice() ? "True" : "False");
    FI_HILOGI("  is keyboard:   %{public}s", dev->IsKeyboard() ? "True" : "False");

    for (auto observer : observers_) {
        observer->OnDeviceAdded(dev);
    }
}

void DeviceManager::OnInputDeviceRemoved(std::shared_ptr<IDevice> dev)
{
    for (auto observer : observers_) {
        observer->OnDeviceRemoved(dev);
    }
}

int32_t DeviceManager::EpollCreate()
{
    CALL_DEBUG_ENTER;
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        FI_HILOGE("epoll_create1 failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceManager::EpollAdd(IEpollEventSource *source)
{
    CALL_DEBUG_ENTER;
    CHKPR(source, RET_ERR);
    struct epoll_event ev {
        .events = EPOLLIN | EPOLLHUP | EPOLLERR,
        .data.ptr = source,
    };
    int32_t ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, source->GetFd(), &ev);
    if (ret != 0) {
        FI_HILOGE("epoll_ctl failed: %{public}s", strerror(errno));
        return RET_ERR;
    }
    return RET_OK;
}

void DeviceManager::EpollDel(IEpollEventSource *source)
{
    CALL_DEBUG_ENTER;
    CHKPV(source);
    int32_t ret = epoll_ctl(epollFd_, EPOLL_CTL_DEL, source->GetFd(), nullptr);
    if (ret != 0) {
        FI_HILOGE("epoll_ctl failed: %{public}s", strerror(errno));
    }
}

void DeviceManager::EpollClose()
{
    CALL_DEBUG_ENTER;
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
}

void DeviceManager::Dispatch(const struct epoll_event &ev)
{
    CALL_DEBUG_ENTER;
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        CHKPV(context_);
        int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
            std::bind(&DeviceManager::OnEpollDispatch, this));
        if (ret != RET_OK) {
            FI_HILOGE("PostAsyncTask failed");
        }
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup: %{public}s", strerror(errno));
    }
}

int32_t DeviceManager::OnEpollDispatch()
{
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epoll_wait(epollFd_, evs, MAX_N_EVENTS, 0);
    if (cnt < 0) {
        FI_HILOGE("epoll_wait failed");
        return RET_ERR;
    }
    for (int32_t index = 0; index < cnt; ++index) {
        IEpollEventSource *source = reinterpret_cast<IEpollEventSource *>(evs[index].data.ptr);
        CHKPC(source);
        if ((evs[index].events & EPOLLIN) == EPOLLIN) {
            source->Dispatch(evs[index]);
        } else if ((evs[index].events & (EPOLLHUP | EPOLLERR)) != 0) {
            FI_HILOGE("Epoll hangup: %{public}s", strerror(errno));
        }
    }
    return RET_OK;
}

std::shared_ptr<IDevice> DeviceManager::GetDevice(int32_t id) const
{
    CHKPP(context_);
    std::packaged_task<std::shared_ptr<IDevice>(int32_t)> task {
        std::bind(&DeviceManager::OnGetDevice, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetDevice, this, std::ref(task), id));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return nullptr;
    }
    return fu.get();
}

std::shared_ptr<IDevice> DeviceManager::OnGetDevice(int32_t id) const
{
    if (auto devIter = devices_.find(id); devIter != devices_.cend()) {
        return devIter->second;
    }
    return nullptr;
}

int32_t DeviceManager::RunGetDevice(std::packaged_task<std::shared_ptr<IDevice>(int32_t)> &task,
                                    int32_t id) const
{
    task(id);
    return RET_OK;
}

int32_t DeviceManager::AddDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnAddDeviceObserver, this, observer));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
    }
    return ret;
}

int32_t DeviceManager::OnAddDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    auto ret = observers_.insert(observer);
    if (!ret.second) {
        FI_HILOGW("Observer is duplicated");
    }
    return RET_OK;
}

void DeviceManager::RemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnRemoveDeviceObserver, this, observer));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
    }
}

int32_t DeviceManager::OnRemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    observers_.erase(observer);
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION

bool DeviceManager::IsRemote(int32_t id) const
{
    CHKPR(context_, false);
    std::packaged_task<bool(int32_t)> task { std::bind(&DeviceManager::OnIsRemote, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunIsRemote, this, std::ref(task), id));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return false;
    }
    return fu.get();
}

bool DeviceManager::OnIsRemote(int32_t id) const
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(id); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->IsRemote();
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return false;
}

int32_t DeviceManager::RunIsRemote(std::packaged_task<bool(int32_t)> &task, int32_t id) const
{
    task(id);
    return RET_OK;
}

std::vector<std::string> DeviceManager::GetCoordinationDhids(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return std::vector<std::string>();
    }
    std::packaged_task<std::vector<std::string>(int32_t)> task {
        std::bind(&DeviceManager::OnGetCoopDhids, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetGetCoopDhids, this, std::ref(task), deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return std::vector<std::string>();
    }
    return fu.get();
}

std::vector<std::string> DeviceManager::OnGetCoopDhids(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    std::vector<std::string> dhids;
    auto devIter = devices_.find(deviceId);
    if (devIter == devices_.end()) {
        FI_HILOGI("Find pointer id failed");
        return dhids;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Device is unsynchronized");
        return dhids;
    }
    std::shared_ptr<IDevice> dev = devIter->second;
    if (!dev->IsPointerDevice()) {
        FI_HILOGI("Not pointer device");
        return dhids;
    }
    dhids.push_back(dev->GetDhid());
    FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "pointer");

    const std::string localNetworkId { COORDINATION::GetLocalDeviceId() };
    const auto pointerNetworkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };

    for (const auto &[id, dev]: devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        const auto networkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };
        if (networkId != pointerNetworkId) {
            continue;
        }
        if (dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            dhids.push_back(dev->GetDhid());
            FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "supportkey");
        }
    }
    return dhids;
}

int32_t DeviceManager::RunGetGetCoopDhids(
    std::packaged_task<std::vector<std::string>(int32_t)> &task,
    int32_t deviceId) const
{
    task(deviceId);
    return RET_OK;
}

std::vector<std::string> DeviceManager::GetCoordinationDhids(const std::string &dhid) const
{
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return std::vector<std::string>();
    }
    std::packaged_task<std::vector<std::string>(const std::string &)> task {
        std::bind(&DeviceManager::OnGetCoordinationDhids, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetCoordinationDhids, this, std::ref(task), std::cref(dhid)));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return std::vector<std::string>();
    }
    return fu.get();
}

std::vector<std::string> DeviceManager::OnGetCoordinationDhids(const std::string &dhid) const
{
    int32_t inputDeviceId { -1 };
    for (const auto &[id, dev] : devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        if (dev->GetDhid() == dhid) {
            inputDeviceId = id;
            break;
        }
    }
    return GetCoordinationDhids(inputDeviceId);
}

int32_t DeviceManager::RunGetCoordinationDhids(
    std::packaged_task<std::vector<std::string>(const std::string &)> &task,
    const std::string &dhid) const
{
    task(dhid);
    return RET_OK;
}

std::string DeviceManager::GetOriginNetworkId(int32_t id) const
{
    CALL_INFO_TRACE;
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return EMPTYSTR;
    }
    std::packaged_task<std::string(int32_t)> task {
        std::bind(&DeviceManager::OnGetOriginNetId, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetOriginNetId, this, std::ref(task), id));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return EMPTYSTR;
    }
    return fu.get();
}

std::string DeviceManager::OnGetOriginNetId(int32_t id) const
{
    CALL_INFO_TRACE;
    auto devIter = devices_.find(id);
    if (devIter == devices_.end()) {
        FI_HILOGE("Failed to search for the device: id %{public}d", id);
        return EMPTYSTR;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Device is unsynchronized");
        return EMPTYSTR;
    }
    auto networkId = devIter->second->GetNetworkId();
    if (networkId.empty()) {
        networkId = COORDINATION::GetLocalDeviceId();
    }
    return networkId;
}

int32_t DeviceManager::RunGetOriginNetId(std::packaged_task<std::string(int32_t)> &task, int32_t id) const
{
    task(id);
    return RET_OK;
}

std::string DeviceManager::GetOriginNetworkId(const std::string &dhid) const
{
    CALL_INFO_TRACE;
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return EMPTYSTR;
    }
    std::packaged_task<std::string(const std::string &)> task {
        std::bind(&DeviceManager::OnGetOriginNetworkId, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetOriginNetworkId, this, std::ref(task), std::cref(dhid)));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return EMPTYSTR;
    }
    return fu.get();
}

std::string DeviceManager::OnGetOriginNetworkId(const std::string &dhid) const
{
    CALL_INFO_TRACE;
    if (dhid.empty()) {
        return EMPTYSTR;
    }
    for (const auto &[id, dev] : devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            return dev->GetNetworkId();
        }
    }
    return EMPTYSTR;
}

int32_t DeviceManager::RunGetOriginNetworkId(std::packaged_task<std::string(const std::string &)> &task,
                                             const std::string &dhid) const
{
    task(dhid);
    return RET_OK;
}

std::string DeviceManager::GetDhid(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return EMPTYSTR;
    }
    std::packaged_task<std::string(int32_t)> task {
        std::bind(&DeviceManager::OnGetDhid, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetDhid, this, std::ref(task), deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return EMPTYSTR;
    }
    return fu.get();
}

std::string DeviceManager::OnGetDhid(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->GetDhid();
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return EMPTYSTR;
}

int32_t DeviceManager::RunGetDhid(std::packaged_task<std::string(int32_t)> &task, int32_t deviceId) const
{
    task(deviceId);
    return RET_OK;
}

bool DeviceManager::HasLocalPointerDevice() const
{
    CHKPR(context_, false);
    std::packaged_task<bool()> task { std::bind(&DeviceManager::OnHasLocalPointerDevice, this) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunHasLocalPointerDevice, this, std::ref(task)));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return false;
    }
    return fu.get();
}

bool DeviceManager::OnHasLocalPointerDevice() const
{
    for (const auto &[id, dev] : devices_) {
        if (dev != nullptr) {
            if (!dev->IsRemote() && dev->IsPointerDevice()) {
                return true;
            }
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return false;
}

int32_t DeviceManager::RunHasLocalPointerDevice(std::packaged_task<bool()> &task) const
{
    task();
    return RET_OK;
}
#endif // OHOS_BUILD_ENABLE_COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS