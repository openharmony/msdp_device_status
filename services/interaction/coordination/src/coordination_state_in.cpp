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

#include "coordination_state_in.h"

#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_message.h"
#include "coordination_softbus_adapter.h"
#include "distributed_input_adapter.h"
#include "coordination_sm.h"
#include "coordination_util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationStateIn" };
} // namespace

CoordinationStateIn::CoordinationStateIn(const std::string &startDhid) : startDhid_(startDhid) {}

int32_t CoordinationStateIn::StartCoordination(const std::string &remoteNetworkId,
    int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("RemoteNetworkId is empty");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    std::string localNetworkId = COORDINATION::GetLocalDeviceId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    int32_t ret = CooSoftbusAdapter->StartRemoteCoordination(localNetworkId, remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Start coordination fail");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    std::string taskName = "process_start_task";
    std::function<void()> handleProcessStartFunc =
        std::bind(&CoordinationStateIn::ProcessStart, this, remoteNetworkId, startDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t CoordinationStateIn::ProcessStart(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPR(context, RET_ERR);
    std::string originNetworkId = CooDevMgr->GetOriginNetworkId(startDeviceId);
    if (remoteNetworkId == originNetworkId) {
        ComeBack(remoteNetworkId, startDeviceId);
        return RET_OK;
    } else {
        return RelayComeBack(remoteNetworkId, startDeviceId);
    }
}

int32_t CoordinationStateIn::StopCoordination(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    int32_t ret = CooSoftbusAdapter->StopRemoteCoordination(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination fail");
        return ret;
    }
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc = std::bind(&CoordinationStateIn::ProcessStop, this);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

int32_t CoordinationStateIn::ProcessStop()
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> dhids = CooDevMgr->GetCoordinationDhids(startDhid_);
    std::string sink = CooDevMgr->GetOriginNetworkId(startDhid_);
    int32_t ret = DistributedAdapter->StopRemoteInput(
        sink, dhids, [this, sink](bool isSuccess) { this->OnStopRemoteInput(isSuccess, sink, -1); });
    if (ret != RET_OK) {
        CooSM->OnStopFinish(false, sink);
    }
    return RET_OK;
}

void CoordinationStateIn::OnStartRemoteInput(bool isSuccess, const std::string &srcNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    if (!isSuccess) {
        ICoordinationState::OnStartRemoteInput(isSuccess, srcNetworkId, startDeviceId);
        return;
    }
    std::string sinkNetworkId = CooDevMgr->GetOriginNetworkId(startDeviceId);
    std::vector<std::string> dhid = CooDevMgr->GetCoordinationDhids(startDeviceId);
    std::string taskName = "relay_stop_task";
    std::function<void()> handleRelayStopFunc = std::bind(&CoordinationStateIn::StopRemoteInput,
        this, sinkNetworkId, srcNetworkId, dhid, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleRelayStopFunc, taskName, 0);
}

void CoordinationStateIn::StopRemoteInput(const std::string &sinkNetworkId,
    const std::string &srcNetworkId, const std::vector<std::string> &dhid, int32_t startDeviceId)
{
    int32_t ret = DistributedAdapter->StopRemoteInput(sinkNetworkId, dhid,
        [this, srcNetworkId, startDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, srcNetworkId, startDeviceId);
    });
    if (ret != RET_OK) {
        CooSM->OnStartFinish(false, sinkNetworkId, startDeviceId);
    }
}

void CoordinationStateIn::OnStopRemoteInput(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    if (CooSM->IsStarting()) {
        std::string taskName = "start_finish_task";
        std::function<void()> handleStartFinishFunc = std::bind(&CoordinationSM::OnStartFinish,
            CooSM, isSuccess, remoteNetworkId, startDeviceId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
    } else if (CooSM->IsStopping()) {
        std::string taskName = "stop_finish_task";
        std::function<void()> handleStopFinishFunc =
            std::bind(&CoordinationSM::OnStopFinish, CooSM, isSuccess, remoteNetworkId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
    }
}

void CoordinationStateIn::ComeBack(const std::string &sinkNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> dhids = CooDevMgr->GetCoordinationDhids(startDeviceId);
    if (dhids.empty()) {
        CooSM->OnStartFinish(false, sinkNetworkId, startDeviceId);
    }
    int32_t ret = DistributedAdapter->StopRemoteInput(sinkNetworkId, dhids,
        [this, sinkNetworkId, startDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, sinkNetworkId, startDeviceId);
            });
    if (ret != RET_OK) {
        CooSM->OnStartFinish(false, sinkNetworkId, startDeviceId);
    }
}

int32_t CoordinationStateIn::RelayComeBack(const std::string &srcNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(srcNetworkId, startDeviceId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
