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

#include "i_coordination_state.h"

#include "coordination_event_manager.h"
#include "devicestatus_define.h"
#include "distributed_input_adapter.h"
#include "coordination_sm.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "ICoordinationState" };
} // namespace

ICoordinationState::ICoordinationState()
{
    runner_ = AppExecFwk::EventRunner::Create(true);
    CHKPL(runner_);
    eventHandler_ = std::make_shared<CoordinationEventHandler>(runner_);
}

int32_t ICoordinationState::PrepareAndStart(const std::string &srcNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPR(context, RET_ERR);
    std::string sinkNetworkId = context->GetDeviceManager().GetOriginNetworkId(startDeviceId);
    int32_t ret = RET_ERR;
    if (NeedPrepare(srcNetworkId, sinkNetworkId)) {
        CooSM->UpdatePreparedDevices(srcNetworkId, sinkNetworkId);
        ret = DistributedAdapter->PrepareRemoteInput(
            srcNetworkId, sinkNetworkId, [this, srcNetworkId, startDeviceId](bool isSuccess) {
                this->OnPrepareDistributedInput(isSuccess, srcNetworkId, startDeviceId);
            });
        if (ret != RET_OK) {
            FI_HILOGE("Prepare remote input fail");
            CooSM->OnStartFinish(false, sinkNetworkId, startDeviceId);
            CooSM->UpdatePreparedDevices("", "");
        }
    } else {
        ret = StartRemoteInput(startDeviceId);
        if (ret != RET_OK) {
            FI_HILOGE("Start remoteNetworkId input fail");
            CooSM->OnStartFinish(false, sinkNetworkId, startDeviceId);
        }
    }
    return ret;
}

void ICoordinationState::OnPrepareDistributedInput(
    bool isSuccess, const std::string &srcNetworkId, int32_t startDeviceId)
{
    FI_HILOGI("isSuccess: %{public}s", isSuccess ? "true" : "false");
    if (!isSuccess) {
        CooSM->UpdatePreparedDevices("", "");
        CooSM->OnStartFinish(false, srcNetworkId, startDeviceId);
    } else {
        std::string taskName = "start_dinput_task";
        std::function<void()> handleStartDinputFunc =
            std::bind(&ICoordinationState::StartRemoteInput, this, startDeviceId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStartDinputFunc, taskName, 0);
    }
}

int32_t ICoordinationState::StartRemoteInput(int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::pair<std::string, std::string> networkIds = CooSM->GetPreparedDevices();
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPR(context, RET_ERR);
    std::vector<std::string> dhids = context->GetDeviceManager().GetCoordinationDhids(startDeviceId);
    if (dhids.empty()) {
        CooSM->OnStartFinish(false, networkIds.first, startDeviceId);
        return static_cast<int32_t>(CoordinationMessage::DEVICE_ID_ERROR);
    }
    int32_t ret = DistributedAdapter->StartRemoteInput(
        networkIds.first, networkIds.second, dhids, [this, src = networkIds.first, startDeviceId](bool isSuccess) {
            this->OnStartRemoteInput(isSuccess, src, startDeviceId);
        });
    if (ret != RET_OK) {
        CooSM->OnStartFinish(false, networkIds.first, startDeviceId);
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    return RET_OK;
}

void ICoordinationState::OnStartRemoteInput(
    bool isSuccess, const std::string &srcNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::string taskName = "start_finish_task";
    std::function<void()> handleStartFinishFunc =
        std::bind(&CoordinationSM::OnStartFinish, CooSM, isSuccess, srcNetworkId, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
}

bool ICoordinationState::NeedPrepare(const std::string &srcNetworkId, const std::string &sinkNetworkId)
{
    CALL_DEBUG_ENTER;
    std::pair<std::string, std::string> prepared = CooSM->GetPreparedDevices();
    bool isNeed =  !(srcNetworkId == prepared.first && sinkNetworkId == prepared.second);
    FI_HILOGI("NeedPrepare?: %{public}s", isNeed ? "true" : "false");
    return isNeed;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
