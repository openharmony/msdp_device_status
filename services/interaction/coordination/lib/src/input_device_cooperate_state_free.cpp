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

#include "input_device_cooperate_state_free.h"

#include "device_cooperate_softbus_adapter.h"
#include "input_device_cooperate_sm.h"
#include "input_device_cooperate_util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InputDeviceCooperateStateFree" };
} // namespace

int32_t InputDeviceCooperateStateFree::StartInputDeviceCoordination(
    const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("RemoteNetworkId is empty");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_DEVICE_ERROR);
    }
    std::string localNetworkId = COORDINATION::GetLocalDeviceId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_DEVICE_ERROR);
    }
    int32_t ret = DevCoordinationSoftbusAdapter->StartRemoteCoordination(localNetworkId, remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Start input device coordination fail");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    std::string taskName = "process_start_task";
    std::function<void()> handleProcessStartFunc =
        std::bind(&InputDeviceCooperateStateFree::ProcessStart, this, remoteNetworkId, startInputDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t InputDeviceCooperateStateFree::ProcessStart(const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(remoteNetworkId, startInputDeviceId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
