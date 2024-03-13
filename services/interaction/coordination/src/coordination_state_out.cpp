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

#include "coordination_state_out.h"

#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_message.h"
#include "coordination_sm.h"
#include "coordination_softbus_adapter.h"
#include "coordination_util.h"
#include "distributed_input_adapter.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationStateOut"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CoordinationStateOut::CoordinationStateOut(const std::string &startDeviceDhid)
    : startDeviceDhid_(startDeviceDhid)
{}

int32_t CoordinationStateOut::DeactivateCoordination(const std::string &remoteNetworkId, bool isUnchained,
    const std::pair<std::string, std::string> &preparedNetworkId)
{
    CALL_INFO_TRACE;
    std::string tempRemoteNetworkId = remoteNetworkId;
    if (tempRemoteNetworkId.empty()) {
        tempRemoteNetworkId = preparedNetworkId.first;
    }
    int32_t ret = COOR_SOFTBUS_ADAPTER->StopRemoteCoordination(tempRemoteNetworkId, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination failed");
        return RET_ERR;
    }
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc =
        std::bind(&CoordinationStateOut::ProcessStop, this, tempRemoteNetworkId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

void CoordinationStateOut::ProcessStop(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    std::vector<std::string> inputDeviceDhids = COOR_DEV_MGR->GetCoordinationDhids(startDeviceDhid_, false);
    if (inputDeviceDhids.empty()) {
        COOR_SM->OnStopFinish(false, remoteNetworkId);
    }
    int32_t ret = D_INPUT_ADAPTER->StopRemoteInput(remoteNetworkId, localNetworkId, inputDeviceDhids,
        [this, remoteNetworkId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, remoteNetworkId);
        });
    if (ret != RET_OK) {
        COOR_SM->OnStopFinish(false, remoteNetworkId);
    }
}

void CoordinationStateOut::OnStopRemoteInput(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string taskName = "stop_finish_task";
    std::function<void()> handleStopFinishFunc =
        std::bind(&CoordinationSM::OnStopFinish, COOR_SM, isSuccess, remoteNetworkId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
}

void CoordinationStateOut::OnKeyboardOnline(const std::string &dhid,
    const std::pair<std::string, std::string> &networkIds)
{
    std::vector<std::string> inputDeviceDhids;
    inputDeviceDhids.push_back(dhid);
    D_INPUT_ADAPTER->StartRemoteInput(networkIds.first, networkIds.second, inputDeviceDhids, [](bool isSuccess) {});
}

void CoordinationStateOut::SetStartDeviceDhid(const std::string &startDeviceDhid)
{
    startDeviceDhid_ = startDeviceDhid;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
