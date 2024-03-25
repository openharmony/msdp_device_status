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

#undef LOG_TAG
#define LOG_TAG "CoordinationStateFree"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

int32_t CoordinationStateFree::ActivateCoordination(
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("The remoteNetworkId is empty");
        return COMMON_PARAMETER_ERROR;
    }
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return COMMON_PARAMETER_ERROR;
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StartRemoteCoordination(localNetworkId, remoteNetworkId, true);
    if (ret != RET_OK) {
        FI_HILOGE("Start input device coordination failed");
        return COOPERATOR_FAIL;
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
        FI_HILOGI("No stop operation is required");
        return RET_ERR;
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->OpenInputSoftbus(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to open p2p channel");
        return ret;
    }
    ret = COOR_SOFTBUS_ADAPTER->StopRemoteCoordination(networkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination failed");
        return ret;
    }

    if (!preparedNetworkId.first.empty() && !preparedNetworkId.second.empty()) {
        FI_HILOGI("preparedNetworkId is not empty, first:%{public}s, second:%{public}s, networkId:%{public}s",
            GetAnonyString(preparedNetworkId.first).c_str(), GetAnonyString(preparedNetworkId.second).c_str(),
            GetAnonyString(networkId).c_str());
        if (networkId == preparedNetworkId.first || networkId == preparedNetworkId.second) {
            bool ret = COOR_SM->UnchainCoordination(preparedNetworkId.first, preparedNetworkId.second);
            if (ret) {
                COOR_SM->NotifyChainRemoved();
                std::string localNetworkId = COORDINATION::GetLocalNetworkId();
                FI_HILOGI("LocalNetworkId:%{public}s, remoteNetworkId:%{public}s",
                    GetAnonyString(localNetworkId).c_str(), GetAnonyString(networkId).c_str());
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
