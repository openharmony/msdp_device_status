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

#include "cooperate_util.h"

#include "cooperate_sm.h"
#include "softbus_bus_center.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace COOPERATE {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateUtil" };
} // namespace
std::string GetLocalNetworkId()
{
    CALL_DEBUG_ENTER;
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t ret = GetLocalNodeDeviceInfo(FI_PKG_NAME, localNode.get());
    if (ret != RET_OK) {
        FI_HILOGE("GetLocalNodeDeviceInfo ret:%{public}d", ret);
        return {};
    }
    std::string networkId(localNode->networkId, sizeof(localNode->networkId));
    FI_HILOGD("GetLocalNodeDeviceInfo networkId:%{public}s", networkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
    return localNode->networkId;
}
} // namespace COOPERATE
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
