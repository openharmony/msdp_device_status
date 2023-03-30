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

#include "coordination_state_out.h"

#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_message.h"
#include "coordination_softbus_adapter.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "distributed_input_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationStateOut" };
} // namespace

CoordinationStateOut::CoordinationStateOut(const std::string& startDeviceDhid)
    : startDeviceDhid_(startDeviceDhid)
{}

int32_t CoordinationStateOut::StopCoordination(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string tempRemoteNetworkId = remoteNetworkId;
    if (tempRemoteNetworkId.empty()) {
        std::pair<std::string, std::string> prepared = CooSM->GetPreparedDevices();
        tempRemoteNetworkId = prepared.first;
    }
    int32_t ret = CooSoftbusAdapter->StopRemoteCoordination(tempRemoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination fail");
        return RET_ERR;
    }
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc =
        std::bind(&CoordinationStateOut::ProcessStop, this, tempRemoteNetworkId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

void CoordinationStateOut::ProcessStop(const std::string& remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    std::vector<std::string> inputDeviceDhids = CooDevMgr->GetCoordinationDhids(startDeviceDhid_);
    if (inputDeviceDhids.empty()) {
        CooSM->OnStopFinish(false, remoteNetworkId);
    }
    int32_t ret = DistributedAdapter->StopRemoteInput(remoteNetworkId, localNetworkId, inputDeviceDhids,
        [this, remoteNetworkId](bool isSuccess) {
            this->OnStopRemoteInput(isSuccess, remoteNetworkId);
        });
    if (ret != RET_OK) {
        CooSM->OnStopFinish(false, remoteNetworkId);
    }
}

void CoordinationStateOut::OnStopRemoteInput(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string taskName = "stop_finish_task";
    std::function<void()> handleStopFinishFunc =
        std::bind(&CoordinationSM::OnStopFinish, CooSM, isSuccess, remoteNetworkId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
}

void CoordinationStateOut::OnKeyboardOnline(const std::string &dhid,
    const std::pair<std::string, std::string> &networkIds)
{
    std::vector<std::string> inputDeviceDhids;
    inputDeviceDhids.push_back(dhid);
    DistributedAdapter->StartRemoteInput(networkIds.first, networkIds.second, inputDeviceDhids, [](bool isSuccess) {});
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
