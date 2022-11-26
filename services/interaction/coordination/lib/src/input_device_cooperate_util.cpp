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

#include "input_device_cooperate_util.h"

#include "softbus_bus_center.h"

#include "config_multimodal.h"
#include "devicestatus_hilog_wrapper.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace COOPERATE {
namespace {
} // namespace
std::string GetLocalDeviceId()
{
    auto localNode = std::make_unique<NodeBasicInfo>();
    CHKPS(localNode, SERVICE);
    int32_t ret = GetLocalNodeDeviceInfo(MMI_DINPUT_PKG_NAME, localNode.get());
    if (ret != RET_OK) {
        DEV_HILOGE(SERVICE, "GetLocalNodeDeviceInfo ret:%{public}d", ret);
        return {};
    }
    return localNode->networkId;
}
} // namespace COOPERATE
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
