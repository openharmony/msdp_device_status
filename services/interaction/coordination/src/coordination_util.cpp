/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "coordination_util.h"

#include "unistd.h"

#include "device_manager.h"
#include "dm_device_info.h"

#include "device_manager.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationUtil"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace COORDINATION {
namespace {
const std::string PKG_NAME_PREFIX { "DBinderBus_Dms_" };
#define DSTB_HARDWARE DistributedHardware::DeviceManager::GetInstance()
} // namespace

std::string GetCurrentPackageName()
{
    return PKG_NAME_PREFIX + std::to_string(getpid());
}

std::string GetLocalNetworkId()
{
    auto packageName = GetCurrentPackageName();
    OHOS::DistributedHardware::DmDeviceInfo dmDeviceInfo;
    if (int32_t errCode = DSTB_HARDWARE.GetLocalDeviceInfo(packageName, dmDeviceInfo); errCode != RET_OK) {
        FI_HILOGE("GetLocalBasicInfo failed, errCode:%{public}d", errCode);
        return {};
    }
    FI_HILOGD("LocalNetworkId:%{public}s", GetAnonyString(dmDeviceInfo.networkId).c_str());
    return dmDeviceInfo.networkId;
}

std::string GetLocalUdId()
{
    auto localNetworkId = GetLocalNetworkId();
    auto localUdId = GetUdIdByNetworkId(localNetworkId);
    FI_HILOGD("LocalNetworkId:%{public}s, localUdId:%{public}s",
        GetAnonyString(localNetworkId).c_str(), GetAnonyString(localUdId).c_str());
    return localUdId;
}

std::string GetUdIdByNetworkId(const std::string &networkId)
{
    std::string udId { "Empty" };
    if (int32_t errCode = DSTB_HARDWARE.GetUdidByNetworkId(GetCurrentPackageName(), networkId, udId);
        errCode != RET_OK) {
        FI_HILOGE("GetUdIdByNetworkId failed, errCode:%{public}d, networkId:%{public}s, udId:%{public}s", errCode,
            GetAnonyString(networkId).c_str(), GetAnonyString(udId).c_str());
    }
    return udId;
}

} // namespace COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
