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

#include "cooperate_state_in.h"

#include "cooperate_device_manager.h"
#include "cooperate_event_manager.h"
#include "cooperate_sm.h"
#include "cooperate_softbus_adapter.h"
#include "cooperate_util.h"
#include "distributed_input_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateStateIn" };
} // namespace

CooperateStateIn::CooperateStateIn(const std::string &startDeviceDhid) : startDeviceDhid_(startDeviceDhid) {}

int32_t CooperateStateIn::ActivateCooperate(const std::string &remoteNetworkId,
    int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("RemoteNetworkId is empty");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    std::string localNetworkId = COOPERATE::GetLocalNetworkId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR);
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StartRemoteCooperate(localNetworkId, remoteNetworkId, false);
    if (ret != RET_OK) {
        FI_HILOGE("Start cooperate failed");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    std::string taskName = "process_start_task";
    std::function<void()> handleProcessStartFunc =
        std::bind(&CooperateStateIn::ProcessStart, this, remoteNetworkId, startDeviceId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStartFunc, taskName, 0);
    return RET_OK;
}

int32_t CooperateStateIn::ProcessStart(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    auto* context = COOR_EVENT_MGR->GetIContext();
    CHKPR(context, RET_ERR);
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
    if (remoteNetworkId == originNetworkId) {
        ComeBack(remoteNetworkId, startDeviceId);
        return RET_OK;
    }
    return RelayComeBack(remoteNetworkId, startDeviceId);
}

int32_t CooperateStateIn::DeactivateCooperate(const std::string &remoteNetworkId, bool isUnchained,
    const std::pair<std::string, std::string> &preparedNetworkId)
{
    CALL_DEBUG_ENTER;
    int32_t ret = COOR_SOFTBUS_ADAPTER->StopRemoteCooperate(remoteNetworkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop cooperate failed");
        return ret;
    }
    (void)(preparedNetworkId);
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc = std::bind(&CooperateStateIn::ProcessStop, this);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

int32_t CooperateStateIn::ProcessStop()
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCooperateDhids(startDeviceDhid_, true);
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_);
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(
        originNetworkId, inputDeviceDhids, [this, originNetworkId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, originNetworkId, -1);
        });
    if (ret != RET_OK) {
        FI_HILOGE("Stop remote input failed");
        COOR_SM->OnStopFinish(false, originNetworkId);
    }
    return RET_OK;
}

void CooperateStateIn::OnStartRemoteInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    if (!isSuccess) {
        ICooperateState::OnStartRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        return;
    }
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
    std::vector<std::string> dhid = COOR_DEV_MGR->GetCooperateDhids(startDeviceId);
    std::string taskName = "relay_stop_task";
    std::function<void()> handleRelayStopFunc = std::bind(&CooperateStateIn::StopRemoteInput,
        this, originNetworkId, remoteNetworkId, dhid, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleRelayStopFunc, taskName, 0);
}

void CooperateStateIn::StopRemoteInput(const std::string &originNetworkId,
    const std::string &remoteNetworkId, const std::vector<std::string> &dhid, int32_t startDeviceId)
{
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(originNetworkId, dhid,
        [this, remoteNetworkId, startDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        });
    if (ret != RET_OK) {
        COOR_SM->OnStartFinish(false, originNetworkId, startDeviceId);
    }
}

void CooperateStateIn::OnStopRemoteInput(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    if (COOR_SM->IsStarting()) {
        std::string taskName = "start_finish_task";
        std::function<void()> handleStartFinishFunc = std::bind(&CooperateSM::OnStartFinish,
            COOR_SM, isSuccess, remoteNetworkId, startDeviceId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
    } else if (COOR_SM->IsStopping()) {
        std::string taskName = "stop_finish_task";
        std::function<void()> handleStopFinishFunc =
            std::bind(&CooperateSM::OnStopFinish, COOR_SM, isSuccess, remoteNetworkId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
    }
}

void CooperateStateIn::ComeBack(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCooperateDhids(startDeviceId);
    if (inputDeviceDhids.empty()) {
        COOR_SM->OnStartFinish(false, remoteNetworkId, startDeviceId);
    }
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(remoteNetworkId, inputDeviceDhids,
        [this, remoteNetworkId, startDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        });
    if (ret != RET_OK) {
        COOR_SM->OnStartFinish(false, remoteNetworkId, startDeviceId);
    }
}

int32_t CooperateStateIn::RelayComeBack(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    return PrepareAndStart(remoteNetworkId, startDeviceId);
}

void CooperateStateIn::SetStartDeviceDhid(const std::string &startDeviceDhid)
{
    startDeviceDhid_ = startDeviceDhid;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
