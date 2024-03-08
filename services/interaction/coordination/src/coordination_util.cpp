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

#include "coordination_util.h"

#include "coordination_sm.h"
#include "softbus_bus_center.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace COORDINATION {
namespace {
const std::string PKG_NAME_PREFIX { "DBinderBus_Dms_" };
} // namespace
std::string GetLocalNetworkId()
{
    CALL_DEBUG_ENTER;
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t ret = GetLocalNodeDeviceInfo(FI_PKG_NAME, localNode.get());
    if (ret != RET_OK) {
        FI_HILOGE("Get local node device info, ret:%{public}d", ret);
        return {};
    }
    std::string networkId(localNode->networkId, sizeof(localNode->networkId));
    FI_HILOGD("Get local node device info, networkId:%{public}s", GetAnonyString(networkId).c_str());
    return localNode->networkId;
}

std::string GetCurrentPackageName()
{
    return PKG_NAME_PREFIX + std::to_string(getpid());
}

std::string GetUdidByNetworkId(const std::string &networkId)
{
    std::string udid { "" };
    if (DSTB_HARDWARE.GetUdidByNetworkId(GetCurrentPackageName(), networkId, udid) != RET_OK) {
        FI_HILOGE("GetUdidByNetworkId failed, networkId:%{public}s, udid:%{public}s",
            GetAnonyString(networkId).c_str(), GetAnonyString(udid).c_str());
    }
    return udid;
}

std::string GetLocalUdid()
{
    auto packageName = GetCurrentPackageName();
    OHOS::DistributedHardware::DmDeviceInfo dmDeviceInfo;
    if (int32_t errCode = RET_OK; (errCode = DSTB_HARDWARE.GetLocalDeviceInfo(packageName, dmDeviceInfo)) != RET_OK) {
        FI_HILOGE("GetLocalBasicInfo failed, errCode:%{public}d", errCode);
        return {};
    }
    return COORDINATION::GetUdidByNetworkId(dmDeviceInfo.networkId);
}
} // namespace COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
