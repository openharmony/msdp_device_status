/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "cooperate_state_free.h"

#include "cooperate_softbus_adapter.h"
#include "cooperate_sm.h"
#include "cooperate_util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateStateFree" };
} // namespace

int32_t CooperateStateFree::ActivateCooperate(
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("remoteNetworkId is empty");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    std::string localNetworkId = COOPERATE::GetLocalNetworkId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StartRemoteCooperate(localNetworkId, remoteNetworkId, true);
    if (ret != RET_OK) {
        FI_HILOGE("Start input device cooperate failed");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    std::string taskName = "process_start_task";
    std::function<void()> handleProcessStartFunc =
        std::bind(&CooperateStateFree::ProcessStart, this, remoteNetworkId, startDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t CooperateStateFree::ProcessStart(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(remoteNetworkId, startDeviceId);
}

int32_t CooperateStateFree::DeactivateCooperate(const std::string &networkId, bool isUnchained,
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
    ret = COOR_SOFTBUS_ADAPTER->StopRemoteCooperate(networkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop cooperate failed");
        return ret;
    }

    if (!preparedNetworkId.first.empty() && !preparedNetworkId.second.empty()) {
        FI_HILOGI("preparedNetworkId is not empty, first:%{public}s, second:%{public}s, networkId:%{public}s",
            preparedNetworkId.first.c_str(), preparedNetworkId.second.c_str(),
            networkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
        if (networkId == preparedNetworkId.first || networkId == preparedNetworkId.second) {
            bool ret = COOR_SM->UnchainCooperate(preparedNetworkId.first, preparedNetworkId.second);
            if (ret) {
                COOR_SM->NotifyChainRemoved();
                std::string localNetworkId = COOPERATE::GetLocalNetworkId();
                FI_HILOGD("localNetworkId:%{public}s", localNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
                COOR_SOFTBUS_ADAPTER->NotifyUnchainedResult(localNetworkId, networkId, ret);
            } else {
                FI_HILOGE("Failed to unchain cooperate");
            }
            COOR_SM->SetUnchainStatus(false);
        }
    }
    ret = COOR_SOFTBUS_ADAPTER->StopRemoteCooperateResult(networkId, false);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to stop the process");
        return ret;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
