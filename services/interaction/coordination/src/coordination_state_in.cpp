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

#include "coordination_state_in.h"

#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_message.h"
#include "coordination_sm.h"
#include "coordination_softbus_adapter.h"
#include "coordination_util.h"
#include "distributed_input_adapter.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationStateIn"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CoordinationStateIn::CoordinationStateIn(const std::string &startDeviceDhid) : startDeviceDhid_(startDeviceDhid) {}

int32_t CoordinationStateIn::ActivateCoordination(const std::string &remoteNetworkId,
    int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (remoteNetworkId.empty()) {
        FI_HILOGE("remoteNetworkId is empty");
        return COMMON_PARAMETER_ERROR;
    }
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    if (localNetworkId.empty() || remoteNetworkId == localNetworkId) {
        FI_HILOGE("Input Parameters error");
        return COMMON_PARAMETER_ERROR;
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StartRemoteCoordination(localNetworkId, remoteNetworkId, false);
    if (ret != RET_OK) {
        FI_HILOGE("Start coordination failed");
        return COOPERATOR_FAIL;
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
    CALL_INFO_TRACE;
    IContext* context = COOR_EVENT_MGR->GetIContext();
    CHKPR(context, RET_ERR);
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
    if (remoteNetworkId == originNetworkId) {
        ComeBack(remoteNetworkId, startDeviceId);
        return RET_OK;
    }
    return RelayComeBack(remoteNetworkId, startDeviceId);
}

int32_t CoordinationStateIn::DeactivateCoordination(const std::string &remoteNetworkId, bool isUnchained,
    const std::pair<std::string, std::string> &preparedNetworkId)
{
    CALL_INFO_TRACE;
    int32_t ret = COOR_SOFTBUS_ADAPTER->StopRemoteCoordination(remoteNetworkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination failed");
        return ret;
    }
    (void)(preparedNetworkId);
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc = std::bind(&CoordinationStateIn::ProcessStop, this);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

int32_t CoordinationStateIn::ProcessStop()
{
    CALL_INFO_TRACE;
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCoordinationDhids(startDeviceDhid_, true);
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_);
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(
        originNetworkId, inputDeviceDhids, [this, originNetworkId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, originNetworkId, -1);
        });
    if (ret != RET_OK) {
        FI_HILOGE("Failed to stop remote input");
        COOR_SM->OnStopFinish(false, originNetworkId);
    }
    return RET_OK;
}

void CoordinationStateIn::OnStartRemoteInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    if (!isSuccess) {
        ICoordinationState::OnStartRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        return;
    }
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
    std::vector<std::string> dhid = COOR_DEV_MGR->GetCoordinationDhids(startDeviceId);
    std::string taskName = "relay_stop_task";
    std::function<void()> handleRelayStopFunc = std::bind(&CoordinationStateIn::StopRemoteInput,
        this, originNetworkId, remoteNetworkId, dhid, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleRelayStopFunc, taskName, 0);
}

void CoordinationStateIn::StopRemoteInput(const std::string &originNetworkId,
    const std::string &remoteNetworkId, const std::vector<std::string> &dhid, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(originNetworkId, dhid,
        [this, remoteNetworkId, startDeviceId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        });
    if (ret != RET_OK) {
        COOR_SM->OnStartFinish(false, originNetworkId, startDeviceId);
    }
}

void CoordinationStateIn::OnStopRemoteInput(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    FI_HILOGI("IsSuccess:%{public}s, remoteNetworkId:%{public}s, startDeviceId:%{public}d",
        isSuccess ? "true" : "false", GetAnonyString(remoteNetworkId).c_str(), startDeviceId);
    if (COOR_SM->IsStarting()) {
        FI_HILOGD("Is starting in");
        std::string taskName = "start_finish_task";
        std::function<void()> handleStartFinishFunc = std::bind(&CoordinationSM::OnStartFinish,
            COOR_SM, isSuccess, remoteNetworkId, startDeviceId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
    } else if (COOR_SM->IsStopping()) {
        FI_HILOGD("Is stopping in");
        std::string taskName = "stop_finish_task";
        std::function<void()> handleStopFinishFunc =
            std::bind(&CoordinationSM::OnStopFinish, COOR_SM, isSuccess, remoteNetworkId);
        CHKPV(eventHandler_);
        eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
    }
}

void CoordinationStateIn::ComeBack(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCoordinationDhids(startDeviceId);
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

int32_t CoordinationStateIn::RelayComeBack(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    return PrepareAndStart(remoteNetworkId, startDeviceId);
}

void CoordinationStateIn::SetStartDeviceDhid(const std::string &startDeviceDhid)
{
    startDeviceDhid_ = startDeviceDhid;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
