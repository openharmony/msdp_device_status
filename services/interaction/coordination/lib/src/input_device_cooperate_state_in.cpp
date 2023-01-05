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

#include "input_device_cooperate_state_in.h"

#include "cooperate_event_manager.h"
#include "coordination_message.h"
#include "device_cooperate_softbus_adapter.h"
#include "distributed_input_adapter.h"
#include "coordination_sm.h"
#include "input_device_cooperate_util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InputDeviceCooperateStateIn" };
} // namespace

InputDeviceCooperateStateIn::InputDeviceCooperateStateIn(const std::string &startDhid) : startDhid_(startDhid) {}

int32_t InputDeviceCooperateStateIn::StartInputDeviceCoordination(const std::string &remoteNetworkId,
    int32_t startInputDeviceId)
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
        std::bind(&InputDeviceCooperateStateIn::ProcessStart, this, remoteNetworkId, startInputDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t InputDeviceCooperateStateIn::ProcessStart(const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPR(context, RET_ERR);
    std::string originNetworkId = context->GetDeviceManager().GetOriginNetworkId(startInputDeviceId);
    if (remoteNetworkId == originNetworkId) {
        ComeBack(remoteNetworkId, startInputDeviceId);
        return RET_OK;
    } else {
        return RelayComeBack(remoteNetworkId, startInputDeviceId);
    }
}

int32_t InputDeviceCooperateStateIn::StopInputDeviceCoordination(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    int32_t ret = DevCoordinationSoftbusAdapter->StopRemoteCoordination(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop input device coordination fail");
        return ret;
    }
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc = std::bind(&InputDeviceCooperateStateIn::ProcessStop, this);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

int32_t InputDeviceCooperateStateIn::ProcessStop()
{
    CALL_DEBUG_ENTER;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPR(context, RET_ERR);
    IDeviceManager &devMgr = context->GetDeviceManager();

    std::vector<std::string> dhids = devMgr.GetCoordinationDhids(startDhid_);
    std::string sink = devMgr.GetOriginNetworkId(startDhid_);
    int32_t ret = DistributedAdapter->StopRemoteInput(
        sink, dhids, [this, sink](bool isSuccess) { this->OnStopRemoteInput(isSuccess, sink, -1); });
    if (ret != RET_OK) {
        InputDevCooSM->OnStopFinish(false, sink);
    }
    return RET_OK;
}

void InputDeviceCooperateStateIn::OnStartRemoteInput(
    bool isSuccess, const std::string &srcNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    if (!isSuccess) {
        ICoordinationState::OnStartRemoteInput(isSuccess, srcNetworkId, startInputDeviceId);
        return;
    }
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    IDeviceManager &devMgr = context->GetDeviceManager();

    std::string sinkNetworkId = devMgr.GetOriginNetworkId(startInputDeviceId);
    std::vector<std::string> dhid = devMgr.GetCoordinationDhids(startInputDeviceId);

    std::string taskName = "relay_stop_task";
    std::function<void()> handleRelayStopFunc = std::bind(&InputDeviceCooperateStateIn::StopRemoteInput,
        this, sinkNetworkId, srcNetworkId, dhid, startInputDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleRelayStopFunc, taskName, 0);
}

void InputDeviceCooperateStateIn::StopRemoteInput(const std::string &sinkNetworkId,
    const std::string &srcNetworkId, const std::vector<std::string> &dhid, int32_t startInputDeviceId)
{
    int32_t ret = DistributedAdapter->StopRemoteInput(sinkNetworkId, dhid,
        [this, srcNetworkId, startInputDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, srcNetworkId, startInputDeviceId);
    });
    if (ret != RET_OK) {
        InputDevCooSM->OnStartFinish(false, sinkNetworkId, startInputDeviceId);
    }
}

void InputDeviceCooperateStateIn::OnStopRemoteInput(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    if (InputDevCooSM->IsStarting()) {
        std::string taskName = "start_finish_task";
        std::function<void()> handleStartFinishFunc = std::bind(&CoordinationSM::OnStartFinish,
            InputDevCooSM, isSuccess, remoteNetworkId, startInputDeviceId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
    } else if (InputDevCooSM->IsStopping()) {
        std::string taskName = "stop_finish_task";
        std::function<void()> handleStopFinishFunc =
            std::bind(&CoordinationSM::OnStopFinish, InputDevCooSM, isSuccess, remoteNetworkId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
    }
}

void InputDeviceCooperateStateIn::ComeBack(const std::string &sinkNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    std::vector<std::string> dhids = context->GetDeviceManager().GetCoordinationDhids(startInputDeviceId);
    if (dhids.empty()) {
       InputDevCooSM->OnStartFinish(false, sinkNetworkId, startInputDeviceId);
    }
    int32_t ret = DistributedAdapter->StopRemoteInput(sinkNetworkId, dhids,
        [this, sinkNetworkId, startInputDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, sinkNetworkId, startInputDeviceId);
            });
    if (ret != RET_OK) {
        InputDevCooSM->OnStartFinish(false, sinkNetworkId, startInputDeviceId);
    }
}

int32_t InputDeviceCooperateStateIn::RelayComeBack(const std::string &srcNetworkId, int32_t startInputDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(srcNetworkId, startInputDeviceId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
