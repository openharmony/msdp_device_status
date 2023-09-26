/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cstring>
#include <regex>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/stat.h>

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_util.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "device.h"
#include "devicestatus_define.h"
#include "fi_log.h"
#include "napi_constants.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceManager" };
constexpr int32_t MAX_N_EVENTS { 64 };
constexpr size_t EXPECTED_N_SUBMATCHES { 2 };
constexpr size_t EXPECTED_SUBMATCH { 1 };
} // namespace

DeviceManager::HotplugHandler::HotplugHandler(DeviceManager &devMgr)
    : devMgr_(devMgr)
{}

void DeviceManager::HotplugHandler::AddDevice(const std::string &devNode)
{
    devMgr_.AddDevice(devNode);
}

void DeviceManager::HotplugHandler::RemoveDevice(const std::string &devNode)
{
    devMgr_.RemoveDevice(devNode);
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
    monitor_.SetDeviceMgr(&hotplug_);
    enumerator_.SetDeviceMgr(&hotplug_);
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
    enumerator_.ScanDevices();
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

std::shared_ptr<IDevice> DeviceManager::FindDevice(const std::string &devPath)
{
    auto tIter = std::find_if(devices_.cbegin(), devices_.cend(),
        [devPath](const auto &item) {
            return ((item.second != nullptr) && (item.second->GetDevPath() == devPath));
        });
    return (tIter != devices_.cend() ? tIter->second : nullptr);
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
    return RET_ERR;
}

std::shared_ptr<IDevice> DeviceManager::AddDevice(const std::string &devNode)
{
    CALL_INFO_TRACE;
    const std::string SYS_INPUT_PATH { "/sys/class/input/" };
    const std::string devPath { DEV_INPUT_PATH + devNode };
    struct stat statbuf;

    if (stat(devPath.c_str(), &statbuf) != 0) {
        FI_HILOGD("Invalid device path:%{public}s", devPath.c_str());
        return nullptr;
    }
    if (!S_ISCHR(statbuf.st_mode)) {
        FI_HILOGD("Not character device:%{public}s", devPath.c_str());
        return nullptr;
    }

    int32_t deviceId = ParseDeviceId(devNode);
    if (deviceId < 0) {
        FI_HILOGE("Parsing device name failed:%{public}s", devNode.c_str());
        return nullptr;
    }

    std::shared_ptr<IDevice> dev = FindDevice(devPath);
    if (dev != nullptr) {
        FI_HILOGD("Already exists:%{public}s", devPath.c_str());
        return dev;
    }

    const std::string lSysPath { SYS_INPUT_PATH + devNode };
    char rpath[PATH_MAX];
    if (realpath(lSysPath.c_str(), rpath) == nullptr) {
        FI_HILOGD("Invalid sysPath:%{public}s", lSysPath.c_str());
        return nullptr;
    }

    dev = std::make_shared<Device>(deviceId);
    dev->SetDevPath(devPath);
    dev->SetSysPath(std::string(rpath));
    if (dev->Open() != RET_OK) {
        FI_HILOGE("Unable to open \'%{public}s\'", devPath.c_str());
        return nullptr;
    }
    auto ret = devices_.insert_or_assign(dev->GetId(), dev);
    if (ret.second) {
        FI_HILOGD("\'%{public}s\' added", dev->GetName().c_str());
        OnDeviceAdded(dev);
    }
    return dev;
}

std::shared_ptr<IDevice> DeviceManager::RemoveDevice(const std::string &devNode)
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
            OnDeviceRemoved(dev);
            return dev;
        }
    }
    FI_HILOGD("\'%{public}s\' was not found", devNode.c_str());
    return nullptr;
}

void DeviceManager::OnDeviceAdded(std::shared_ptr<IDevice> dev)
{
    CHKPV(dev);
    FI_HILOGI("Add device %{public}d:%{public}s", dev->GetId(), dev->GetDevPath().c_str());
    FI_HILOGI("  sysPath:       \"%{public}s\"", dev->GetSysPath().c_str());
    FI_HILOGI("  bus:           %{public}04x", dev->GetBus());
    FI_HILOGI("  vendor:        %{public}04x", dev->GetVendor());
    FI_HILOGI("  product:       %{public}04x", dev->GetProduct());
    FI_HILOGI("  version:       %{public}04x", dev->GetVersion());
    FI_HILOGI("  name:          \"%{public}s\"", dev->GetName().c_str());
    FI_HILOGI("  location:      \"%{public}s\"", dev->GetPhys().c_str());
    FI_HILOGI("  unique id:     \"%{public}s\"", dev->GetUniq().c_str());
    FI_HILOGI("  is pointer:    %{public}s", dev->IsPointerDevice() ? "True" : "False");
    FI_HILOGI("  is keyboard:   %{public}s", dev->IsKeyboard() ? "True" : "False");

    for (const auto &observer : observers_) {
        std::shared_ptr<IDeviceObserver> ptr = observer.lock();
        CHKPC(ptr);
        ptr->OnDeviceAdded(dev);
    }
}

void DeviceManager::OnDeviceRemoved(std::shared_ptr<IDevice> dev)
{
    for (const auto &observer : observers_) {
        std::shared_ptr<IDeviceObserver> ptr = observer.lock();
        CHKPC(ptr);
        ptr->OnDeviceRemoved(dev);
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
    struct epoll_event ev {};
    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    ev.data.ptr = source;
    int32_t ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, source->GetFd(), &ev);
    if (ret != 0) {
        FI_HILOGE("epoll_ctl failed:%{public}s", strerror(errno));
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
        FI_HILOGE("epoll_ctl failed:%{public}s", strerror(errno));
    }
}

void DeviceManager::EpollClose()
{
    CALL_DEBUG_ENTER;
    if (epollFd_ >= 0) {
        if (close(epollFd_) < 0) {
            FI_HILOGE("Close epoll fd failed, error:%{public}s, epollFd_:%{public}d", strerror(errno), epollFd_);
        }
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
        FI_HILOGE("Epoll hangup:%{public}s", strerror(errno));
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
            FI_HILOGE("Epoll hangup:%{public}s", strerror(errno));
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
    FI_HILOGE("Device id not found");
    return nullptr;
}

int32_t DeviceManager::RunGetDevice(std::packaged_task<std::shared_ptr<IDevice>(int32_t)> &task,
                                    int32_t id) const
{
    task(id);
    return RET_OK;
}

void DeviceManager::RetriggerHotplug(std::weak_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnRetriggerHotplug, this, observer));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
    }
}

int32_t DeviceManager::OnRetriggerHotplug(std::weak_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    std::shared_ptr<IDeviceObserver> ptr = observer.lock();
    CHKPR(ptr, RET_ERR);
    std::for_each(devices_.cbegin(), devices_.cend(),
        [ptr] (const auto &item) {
            if (item.second != nullptr) {
                ptr->OnDeviceAdded(item.second);
            }
        });
    return RET_OK;
}

int32_t DeviceManager::AddDeviceObserver(std::weak_ptr<IDeviceObserver> observer)
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

int32_t DeviceManager::OnAddDeviceObserver(std::weak_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    auto ret = observers_.insert(observer);
    if (!ret.second) {
        FI_HILOGW("Observer is duplicated");
    }
    return RET_OK;
}

void DeviceManager::RemoveDeviceObserver(std::weak_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnRemoveDeviceObserver, this, observer));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
    }
}

int32_t DeviceManager::OnRemoveDeviceObserver(std::weak_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    observers_.erase(observer);
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS