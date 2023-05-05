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

#include "devicestatus_agent.h"

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "stationary_callback.h"
#include "stationary_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
void DeviceStatusAgent::DeviceStatusAgentCallback::OnDeviceStatusChanged(
    const Data& devicestatusData)
{
    DEV_HILOGI(INNERKIT, "type:%{public}d, value:%{public}d", static_cast<Type>(devicestatusData.type),
        static_cast<OnChangedValue>(devicestatusData.value));
    std::shared_ptr<DeviceStatusAgent> agent = agent_.lock();
    if (agent == nullptr) {
        DEV_HILOGE(SERVICE, "agent is nullptr");
        return;
    }
    agent->agentEvent_->OnEventResult(devicestatusData);
}

int32_t DeviceStatusAgent::SubscribeAgentEvent(const Type& type,
    const ActivityEvent& event,
    const ReportLatencyNs& latency,
    const std::shared_ptr<DeviceStatusAgent::DeviceStatusAgentEvent>& agentEvent)
{
    DEV_HILOGD(INNERKIT, "Enter");

    if (agentEvent == nullptr) {
        return ERR_INVALID_VALUE;
    }
    if (type > Type::TYPE_INVALID && type <= Type::TYPE_LID_OPEN && event > ActivityEvent::EVENT_INVALID
        && event <= ActivityEvent::ENTER_EXIT) {
        RegisterServiceEvent(type, event, latency);
        agentEvent_ = agentEvent;
    } else {
        return ERR_INVALID_VALUE;
    }
    return RET_OK;
}

int32_t DeviceStatusAgent::UnsubscribeAgentEvent(const Type& type, const ActivityEvent& event)
{
    if (type > Type::TYPE_INVALID && type <= Type::TYPE_LID_OPEN && event > ActivityEvent::EVENT_INVALID
        && event <= ActivityEvent::ENTER_EXIT) {
        UnRegisterServiceEvent(type, event);
        return RET_OK;
    }
    return ERR_INVALID_VALUE;
}

void DeviceStatusAgent::RegisterServiceEvent(const Type& type, const ActivityEvent& event,
    const ReportLatencyNs& latency)
{
    DEV_HILOGD(INNERKIT, "Enter");
    callback_ = new (std::nothrow) DeviceStatusAgentCallback(shared_from_this());
    StationaryManager::GetInstance()->SubscribeCallback(type, event, latency, callback_);
}

void DeviceStatusAgent::UnRegisterServiceEvent(const Type& type,
    const ActivityEvent& event)
{
    DEV_HILOGD(INNERKIT, "Enter");
    StationaryManager::GetInstance()->UnsubscribeCallback(type, event, callback_);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
