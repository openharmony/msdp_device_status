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

CoordinationStateOut::CoordinationStateOut(const std::string& startDhid)
    : startDhid_(startDhid)
{}

int32_t CoordinationStateOut::StopCoordination(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::string srcNetworkId = networkId;
    if (srcNetworkId.empty()) {
        std::pair<std::string, std::string> prepared = CooSM->GetPreparedDevices();
        srcNetworkId = prepared.first;
    }
    int32_t ret = CooSoftbusAdapter->StopRemoteCoordination(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop coordination fail");
        return RET_ERR;
    }
    std::string taskName = "process_stop_task";
    std::function<void()> handleProcessStopFunc =
        std::bind(&CoordinationStateOut::ProcessStop, this, srcNetworkId);
    CHKPR(eventHandler_, RET_ERR);
    eventHandler_->ProxyPostTask(handleProcessStopFunc, taskName, 0);
    return RET_OK;
}

void CoordinationStateOut::ProcessStop(const std::string& srcNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string sink = COORDINATION::GetLocalDeviceId();
    std::vector<std::string> dhids = CooDevMgr->GetCoordinationDhids(startDhid_);
    if (dhids.empty()) {
        CooSM->OnStopFinish(false, srcNetworkId);
    }
    int32_t ret = DistributedAdapter->StopRemoteInput(srcNetworkId, sink, dhids, [this, srcNetworkId](bool isSuccess) {
        this->OnStopRemoteInput(isSuccess, srcNetworkId);
        });
    if (ret != RET_OK) {
        CooSM->OnStopFinish(false, srcNetworkId);
    }
}

void CoordinationStateOut::OnStopRemoteInput(bool isSuccess, const std::string &srcNetworkId)
{
    CALL_DEBUG_ENTER;
    std::string taskName = "stop_finish_task";
    std::function<void()> handleStopFinishFunc =
        std::bind(&CoordinationSM::OnStopFinish, CooSM, isSuccess, srcNetworkId);
    CHKPV(eventHandler_);
    eventHandler_->ProxyPostTask(handleStopFinishFunc, taskName, 0);
}

void CoordinationStateOut::OnKeyboardOnline(const std::string &dhid)
{
    std::pair<std::string, std::string> networkIds = CooSM->GetPreparedDevices();
    std::vector<std::string> dhids;
    dhids.push_back(dhid);
    DistributedAdapter->StartRemoteInput(networkIds.first, networkIds.second, dhids, [](bool isSuccess) {});
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
