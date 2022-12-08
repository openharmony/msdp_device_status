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

#include <algorithm>

#include <linux/input.h>

#include "input_manager.h"

#include "devicestatus_define.h"
#include "fi_log.h"
#include "input_device_cooperate_util.h"
#include "napi_constants.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceManager" };
constexpr int32_t DEFAULT_WAIT_TIME_MS { 1000 };
constexpr int32_t WAIT_FOR_ONCE { 1 };
} // namespace

DeviceManager::InputDeviceListener::InputDeviceListener(DeviceManager &devMgr)
    : devMgr_(devMgr) {}

void DeviceManager::InputDeviceListener::OnDeviceAdded(int32_t deviceId, const std::string &type)
{
    devMgr_.OnDeviceAdded(deviceId, type);
}

void DeviceManager::InputDeviceListener::OnDeviceRemoved(int32_t deviceId, const std::string &type)
{
    devMgr_.OnDeviceRemoved(deviceId, type);
}

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
    inputDevListener_ = std::make_shared<InputDeviceListener>(*this);
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
    CALL_INFO_TRACE;
    CHKPR(InputMgr, RET_ERR);
    int32_t ret = InputMgr->RegisterDevListener(CHANGED_TYPE, inputDevListener_);
    if (ret != RET_OK) {
        FI_HILOGE("RegisterDevListener failed");
        return ret;
    }
    ret = InputMgr->GetDeviceIds(std::bind(&DeviceManager::OnGetDeviceIds, this, std::placeholders::_1));
    if (ret != RET_OK) {
        FI_HILOGE("GetDeviceIds failed");
        int32_t r = InputMgr->UnregisterDevListener(CHANGED_TYPE, inputDevListener_);
        if (r != RET_OK) {
            FI_HILOGE("UnregisterDevListener failed");
        }
        return ret;
    }
    return RET_OK;
}

void DeviceManager::Disable()
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::OnDisable, this));
    if (ret != RET_OK) {
        FI_HILOGE("Post sync task failed");
    }
}

int32_t DeviceManager::OnDisable()
{
    CALL_INFO_TRACE;
    int32_t ret = InputMgr->UnregisterDevListener(CHANGED_TYPE, inputDevListener_);
    if (ret != RET_OK) {
        FI_HILOGE("UnregisterDevListener failed");
        return ret;
    }
    devices_.clear();
    return RET_OK;
}

void DeviceManager::OnGetDeviceIds(std::vector<int32_t> &deviceIds)
{
    CALL_INFO_TRACE;
    CHKPV(context_);

    for (const int32_t id : deviceIds) {
        int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
            std::bind(&DeviceManager::GetDeviceAsync, this, id));
        if (ret != RET_OK) {
            FI_HILOGE("Post async task failed");
        }
    }

    int32_t timerId = context_->GetTimerManager().AddTimer(DEFAULT_WAIT_TIME_MS,
        WAIT_FOR_ONCE, std::bind(&DeviceManager::Synchronize, this));
    if (timerId < 0) {
        FI_HILOGE("Add timer failed");
    }
}

int32_t DeviceManager::Synchronize()
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    bool needSync { false };

    for (const auto &[id, dev] : devices_) {
        if (dev != nullptr) {
            continue;
        }
        needSync = true;
        CHKPR(InputMgr, RET_ERR);
        int32_t ret = InputMgr->GetDevice(id,
            std::bind(&DeviceManager::AddDevice, this, std::placeholders::_1));
        if (ret != 0) {
            FI_HILOGE("GetDevice failed");
        }
    }
    if (needSync) {
        int32_t ret = context_->GetTimerManager().AddTimer(DEFAULT_WAIT_TIME_MS,
            WAIT_FOR_ONCE, std::bind(&DeviceManager::Synchronize, this));
        if (ret != RET_OK) {
            FI_HILOGE("Add timer failed");
        }
    }
    return RET_OK;
}

void DeviceManager::OnDeviceAdded(int32_t deviceId, const std::string &type)
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::GetDeviceAsync, this, deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
}

void DeviceManager::OnDeviceRemoved(int32_t deviceId, const std::string &type)
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnRemoveDevice, this, deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
}

int32_t DeviceManager::GetDeviceAsync(int32_t deviceId)
{
    CALL_INFO_TRACE;
    if (devices_.find(deviceId) == devices_.end()) {
        devices_.emplace(deviceId, nullptr);
    }

    CHKPR(InputMgr, RET_ERR);
    int32_t ret = InputMgr->GetDevice(deviceId,
        std::bind(&DeviceManager::AddDevice, this, std::placeholders::_1));
    if (ret != 0) {
        FI_HILOGE("GetDevice failed");
    }
    return ret;
}

void DeviceManager::AddDevice(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev)
{
    CALL_INFO_TRACE;
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DeviceManager::OnAddDevice, this, inputDev));
    if (ret != RET_OK) {
        FI_HILOGE("PostAsyncTask failed");
    }
}

int32_t DeviceManager::OnAddDevice(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev)
{
    CALL_INFO_TRACE;
    CHKPR(inputDev, RET_ERR);
    std::shared_ptr<Device> dev = std::make_shared<Device>(inputDev);

    auto devIter = devices_.find(dev->GetId());
    bool needNotify { (devIter == devices_.end()) || (devIter->second == nullptr) };

    auto [tIter, isOk] = devices_.insert_or_assign(dev->GetId(), dev);
    if (isOk || needNotify) {
        FI_HILOGI("Add \'%{public}s\'", dev->GetName().c_str());
    } else {
        FI_HILOGW("There exists \'%{public}s\'", dev->GetName().c_str());
    }

    if (needNotify) {
        for (auto observer : observers_) {
            observer->OnDeviceAdded(dev);
        }
    }
    return RET_OK;
}

int32_t DeviceManager::OnRemoveDevice(int32_t deviceId)
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.cend()) {
        std::shared_ptr<Device> dev = devIter->second;
        devices_.erase(devIter);

        if (dev != nullptr) {
            FI_HILOGI("Device(\'%{public}s\') removed", dev->GetName().c_str());
            for (auto observer : observers_) {
                observer->OnDeviceRemoved(dev);
            }
        }
    } else {
        FI_HILOGD("Device not found");
    }
    return RET_OK;
}

int32_t DeviceManager::OnAddDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    auto ret = observers_.insert(observer);
    if (!ret.second) {
        FI_HILOGW("Observer added already");
    }
    return RET_OK;
}

int32_t DeviceManager::OnRemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer)
{
    CALL_INFO_TRACE;
    CHKPR(observer, RET_ERR);
    observers_.erase(observer);
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
            FI_HILOGI("isremote: %{public}s", devIter->second->IsRemote() ? "true" : "false");
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

std::vector<std::string> DeviceManager::GetCooperateDhids(int32_t deviceId) const
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
    std::shared_ptr<Device> dev = devIter->second;
    if (!dev->IsPointerDevice()) {
        FI_HILOGI("Not pointer device");
        return dhids;
    }
    dhids.push_back(dev->GetDhid());
    FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "pointer");

    const std::string localNetworkId { COOPERATE::GetLocalDeviceId() };
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
        if (dev->GetKeyboardType() == ::OHOS::MMI::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            dhids.push_back(dev->GetDhid());
            FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "supportkey");
        }
    }

    for (const auto &dhid : dhids) {
        FI_HILOGI("dhid: %{public}s", dhid.c_str());
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

std::vector<std::string> DeviceManager::GetCooperateDhids(const std::string &dhid) const
{
    if (context_ == nullptr) {
        FI_HILOGE("context_ is nullptr");
        return std::vector<std::string>();
    }
    std::packaged_task<std::vector<std::string>(const std::string &)> task {
        std::bind(&DeviceManager::OnGetCooperateDhids, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetDelegateTasks().PostSyncTask(
        std::bind(&DeviceManager::RunGetCooperateDhids, this, std::ref(task), std::cref(dhid)));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return std::vector<std::string>();
    }
    return fu.get();
}

std::vector<std::string> DeviceManager::OnGetCooperateDhids(const std::string &dhid) const
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
    return GetCooperateDhids(inputDeviceId);
}

int32_t DeviceManager::RunGetCooperateDhids(
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
    FI_HILOGI("id:%{public}d", id);
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
        networkId = COOPERATE::GetLocalDeviceId();
    }
    FI_HILOGI("OriginNetworkId:%{public}s", networkId.c_str());
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
    FI_HILOGI("----------dhid:%{public}s", dhid.c_str());
    if (dhid.empty()) {
        return EMPTYSTR;
    }
    for (const auto &[id, dev] : devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            FI_HILOGI("tOriginNetworkId:%{public}s", dev->GetNetworkId().c_str());
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
    FI_HILOGI("deviceId: %{public}d", deviceId);
    if (auto devIter = devices_.find(deviceId); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            FI_HILOGI("dhid: %{public}s", devIter->second->GetDhid().c_str());
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
