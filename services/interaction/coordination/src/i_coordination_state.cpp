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

#include "i_coordination_state.h"

#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_sm.h"
#include "devicestatus_define.h"
#include "distributed_input_adapter.h"

#undef LOG_TAG
#define LOG_TAG "ICoordinationState"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
const std::string THREAD_NAME { "coordination_state" };
} // namespace

ICoordinationState::ICoordinationState()
{
    runner_ = AppExecFwk::EventRunner::Create(THREAD_NAME);
    CHKPL(runner_);
    eventHandler_ = std::make_shared<CoordinationEventHandler>(runner_);
}

int32_t ICoordinationState::PrepareAndStart(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    int32_t ret = RET_ERR;
    std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
    if (NeedPrepare(remoteNetworkId, originNetworkId)) {
        COOR_SM->UpdatePreparedDevices(remoteNetworkId, originNetworkId);
        ret = D_INPUT_ADAPTER->PrepareRemoteInput(remoteNetworkId, originNetworkId,
            [this, remoteNetworkId, startDeviceId](bool isSuccess) {
                this->OnPrepareDistributedInput(isSuccess, remoteNetworkId, startDeviceId);
            });
        if (ret != RET_OK) {
            FI_HILOGE("Failed to prepare remote input");
            COOR_SM->OnStartFinish(false, originNetworkId, startDeviceId);
            COOR_SM->UpdatePreparedDevices("", "");
        }
    } else {
        ret = StartRemoteInput(startDeviceId);
        if (ret != RET_OK) {
            FI_HILOGE("Failed to start remoteNetworkId input");
            COOR_SM->OnStartFinish(false, originNetworkId, startDeviceId);
        }
    }
    return ret;
}

void ICoordinationState::OnPrepareDistributedInput(bool isSuccess, const std::string &remoteNetworkId,
    int32_t startDeviceId)
{
    FI_HILOGI("The isSuccess state:%{public}s", isSuccess ? "true" : "false");
    if (!isSuccess) {
        COOR_SM->UpdatePreparedDevices("", "");
        COOR_SM->OnStartFinish(false, remoteNetworkId, startDeviceId);
        return;
    }
    std::string taskName = "start_dinput_task";
    std::function<void()> handleStartDinputFunc =
        std::bind(&ICoordinationState::StartRemoteInput, this, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStartDinputFunc, taskName, 0);
}

int32_t ICoordinationState::StartRemoteInput(int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::pair<std::string, std::string> networkIds = COOR_SM->GetPreparedDevices();
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCoordinationDhids(startDeviceId);
    if (inputDeviceDhids.empty()) {
        COOR_SM->OnStartFinish(false, networkIds.first, startDeviceId);
        return COOPERATOR_FAIL;
    }
    int32_t ret = D_INPUT_ADAPTER->StartRemoteInput(networkIds.first, networkIds.second, inputDeviceDhids,
        [this, remoteNetworkId = networkIds.first, startDeviceId](bool isSuccess) {
            this->OnStartRemoteInput(isSuccess, remoteNetworkId, startDeviceId);
        });
    if (ret != RET_OK) {
        COOR_SM->OnStartFinish(false, networkIds.first, startDeviceId);
        return COOPERATOR_FAIL;
    }
    return RET_OK;
}

void ICoordinationState::OnStartRemoteInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    std::string taskName = "start_finish_task";
    std::function<void()> handleStartFinishFunc =
        std::bind(&CoordinationSM::OnStartFinish, COOR_SM, isSuccess, remoteNetworkId, startDeviceId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStartFinishFunc, taskName, 0);
}

bool ICoordinationState::NeedPrepare(const std::string &remoteNetworkId, const std::string &originNetworkId)
{
    CALL_DEBUG_ENTER;
    std::pair<std::string, std::string> prepared = COOR_SM->GetPreparedDevices();
    bool isNeed = !(remoteNetworkId == prepared.first && originNetworkId == prepared.second);
    FI_HILOGI("Need prepare?:%{public}s", isNeed ? "true" : "false");
    return isNeed;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
