/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <linux/input.h>

#include "devicestatus_define.h"
#include "fi_log.h"
#include "input_device_cooperate_util.h"
#include "napi_constants.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceManager" };
} // namespace

DeviceManager::DeviceManager() {}
DeviceManager::~DeviceManager() {}

int32_t DeviceManager::Enable()
{
    CALL_INFO_TRACE;
    int32_t ret = InputMgr->RegisterDevListener(CHANGED_TYPE, shared_from_this());
    if (ret != RET_OK) {
        FI_HILOGE("RegisterDevListener failed");
        return ret;
    }

    ret = InputMgr->GetDeviceIds(
        [this](std::vector<int32_t> &deviceIds) {
            for (const int32_t id : deviceIds) {
                int32_t ret = InputMgr->GetDevice(id,
                    std::bind(&DeviceManager::OnDeviceInfoObtained, this, std::placeholders::_1));
                if (ret != 0) {
                    FI_HILOGE("GetDevice failed");
                }
            }
        }
    );
    if (ret != RET_OK) {
        FI_HILOGW("GetDeviceIds failed");
        return ret;
    }
    return RET_OK;
}

void DeviceManager::Disable()
{
    CALL_INFO_TRACE;
    int32_t ret = InputMgr->UnregisterDevListener(CHANGED_TYPE, shared_from_this());
    if (ret != RET_OK) {
        FI_HILOGE("UnregisterDevListener failed");
        return;
    }

    devices_.clear();
}

void DeviceManager::OnDeviceAdded(int32_t deviceId, const std::string &type)
{
    CALL_INFO_TRACE;
    int32_t ret = InputMgr->GetDevice(deviceId,
        std::bind(&DeviceManager::OnDeviceInfoObtained, this, std::placeholders::_1));
    if (ret != 0) {
        FI_HILOGE("GetDevice failed");
    }
}

void DeviceManager::OnDeviceRemoved(int32_t deviceId, const std::string &type)
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.cend()) {
        FI_HILOGI("Device(\'%{public}s\') removed", devIter->second->GetName().c_str());
        devices_.erase(devIter);
    }
}

void DeviceManager::OnDeviceInfoObtained(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev)
{
    CALL_INFO_TRACE;
    CHKPV(inputDev);
    std::shared_ptr<Device> dev = std::make_shared<Device>(inputDev);
    auto [devIter, isOk] = devices_.insert_or_assign(dev->GetId(), dev);
    if (!isOk) {
        FI_HILOGW("Device(\'%{public}s\') exists already", devIter->second->GetName().c_str());
    }
}

std::shared_ptr<Device> DeviceManager::GetDevice(int32_t id) const
{
    if (auto devIter = devices_.find(id); devIter != devices_.cend()) {
        return devIter->second;
    }
    return nullptr;
}

void DeviceManager::Dump(int fd, const std::vector<std::string> &args)
{
    CALL_DEBUG_ENTER;
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION

bool DeviceManager::IsRemote(int32_t id) const
{
    if (auto devIter = devices_.find(id); devIter != devices_.end()) {
        return devIter->second->IsRemote();
    }
    return false;
}

std::vector<std::string> DeviceManager::GetCooperateDhids(int32_t deviceId)
{
    std::vector<std::string> dhids;
    auto devIter = devices_.find(deviceId);
    if (devIter == devices_.end()) {
        FI_HILOGI("Find pointer id failed");
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

    for (const auto &[id, devPtr]: devices_) {
        const auto networkId { devPtr->IsRemote() ? devPtr->GetNetworkId() : localNetworkId };
        if (networkId != pointerNetworkId) {
            continue;
        }
        if (devPtr->GetKeyboardType() == ::OHOS::MMI::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            dhids.push_back(devPtr->GetDhid());
            FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "supportkey");
        }
    }
    return dhids;
}

std::vector<std::string> DeviceManager::GetCooperateDhids(const std::string &dhid)
{
    int32_t inputDeviceId { -1 };
    for (const auto &[id, dev] : devices_) {
        if (dev->GetDhid() == dhid) {
            inputDeviceId = id;
            break;
        }
    }
    return GetCooperateDhids(inputDeviceId);
}

std::string DeviceManager::GetOriginNetworkId(int32_t id)
{
    auto devIter = devices_.find(id);
    if (devIter == devices_.end()) {
        FI_HILOGE("Failed to search for the device: id %{public}d", id);
        return EMPTYSTR;
    }
    auto networkId = devIter->second->GetNetworkId();
    if (networkId.empty()) {
        networkId = COOPERATE::GetLocalDeviceId();
    }
    return networkId;
}

std::string DeviceManager::GetOriginNetworkId(const std::string &dhid)
{
    if (dhid.empty()) {
        return EMPTYSTR;
    }
    for (const auto &[id, dev] : devices_) {
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            return dev->GetNetworkId();
        }
    }
    return EMPTYSTR;
}

std::string DeviceManager::GetDhid(int32_t deviceId) const
{
    auto devIter = devices_.find(deviceId);
    if (devIter != devices_.end()) {
        return devIter->second->GetDhid();
    }
    return EMPTYSTR;
}

bool DeviceManager::HasLocalPointerDevice() const
{
    for (const auto &[id, dev] : devices_) {
        if (!dev->IsRemote() && dev->IsPointerDevice()) {
            return true;
        }
    }
    return false;
}

#endif // OHOS_BUILD_ENABLE_COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
