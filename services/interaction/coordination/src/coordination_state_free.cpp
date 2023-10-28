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

#include "coordination_state_free.h"

#include "coordination_softbus_adapter.h"
#include "coordination_sm.h"
#include "coordination_util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationStateFree" };
} // namespace

int32_t CoordinationStateFree::ActivateCoordination(
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("RemoteNetworkId is empty");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters err");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StartRemoteCoordination(localNetworkId, remoteNetworkId, true);
    if (ret != RET_OK) {
        FI_HILOGE("Start input device coordination failed");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    std::string taskName = "process_start_task";
    std::function<void()> handleProcessStartFunc =
        std::bind(&CoordinationStateFree::ProcessStart, this, remoteNetworkId, startDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t CoordinationStateFree::ProcessStart(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(remoteNetworkId, startDeviceId);
}

int32_t CoordinationStateFree::DeactivateCoordination(const std::string &networkId, bool isUnchained,
    const std::pair<std::string, std::string> &preparedNetworkId)
{
    CALL_INFO_TRACE;
    if (!isUnchained) {
        FI_HILOGE("No stop operation is required");
        return RET_ERR;
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->OpenInputSoftbus(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to open softbus");
        return ret;
    }
    ret = COOR_SOFTBUS_ADAPTER->StopRemoteCoordination(networkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination failed");
        return ret;
    }

    if (!preparedNetworkId.first.empty() && !preparedNetworkId.second.empty()) {
        FI_HILOGD("preparedNetworkId is not empty, first:%{public}s, second:%{public}s",
            preparedNetworkId.first.c_str(), preparedNetworkId.second.c_str());
        if (networkId == preparedNetworkId.first || networkId == preparedNetworkId.second) {
            FI_HILOGD("networkId:%{public}s", networkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
            bool ret = COOR_SM->UnchainCoordination(preparedNetworkId.first, preparedNetworkId.second);
            if (ret) {
                COOR_SM->NotifyChainRemoved();
                std::string localNetworkId = COORDINATION::GetLocalNetworkId();
                FI_HILOGD("localNetworkId:%{public}s", localNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
                COOR_SOFTBUS_ADAPTER->NotifyUnchainedResult(localNetworkId, networkId, ret);
            } else {
                FI_HILOGE("Failed to unchain coordination");
            }
            COOR_SM->SetUnchainStatus(false);
        }
    }
    ret = COOR_SOFTBUS_ADAPTER->StopRemoteCoordinationResult(networkId, false);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to stop the process");
        return ret;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
