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
#include "devicestatus_client.h"

#include "idevicestatus_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
void DeviceStatusAgent::DeviceStatusAgentCallback::OnDeviceStatusChanged(
    const DeviceStatusDataUtils::DeviceStatusData& devicestatusData)
{
    DEV_HILOGI(INNERKIT, "type=%{public}d, value=%{public}d",
        static_cast<DeviceStatusDataUtils::DeviceStatusType>(devicestatusData.type),
        static_cast<DeviceStatusDataUtils::DeviceStatusValue>(devicestatusData.value));
    std::shared_ptr<DeviceStatusAgent> agent = agent_.lock();
    if (agent == nullptr) {
        DEV_HILOGE(SERVICE, "agent is nullptr");
        return;
    }
    agent->agentEvent_->OnEventResult(devicestatusData);
}

int32_t DeviceStatusAgent::SubscribeAgentEvent(const DeviceStatusDataUtils::DeviceStatusType& type,
    const std::shared_ptr<DeviceStatusAgent::DeviceStatusAgentEvent>& agentEvent)
{
    DEV_HILOGI(INNERKIT, "Enter");
    
    if (agentEvent == nullptr) {
        return ERR_INVALID_VALUE;
    }
    if (type > DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID
        && type <= DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN) {
        RegisterServiceEvent(type);
        agentEvent_ = agentEvent;
    } else {
        return ERR_INVALID_VALUE;
    }
    return ERR_OK;
}

int32_t DeviceStatusAgent::UnsubscribeAgentEvent(const DeviceStatusDataUtils::DeviceStatusType& type)
{
    if (type > DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID
        && type <= DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN) {
        UnRegisterServiceEvent(type);
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}

void DeviceStatusAgent::RegisterServiceEvent(const DeviceStatusDataUtils::DeviceStatusType& type)
{
    DEV_HILOGI(INNERKIT, "Enter");
    callback_ = new (std::nothrow) DeviceStatusAgentCallback(shared_from_this());
    if (callback_ == nullptr) {
        DEV_HILOGE(INNERKIT, "callback_ is nullptr");
        return;
    }
    DeviceStatusClient::GetInstance().SubscribeCallback(type, callback_);
}

void DeviceStatusAgent::UnRegisterServiceEvent(const DeviceStatusDataUtils::DeviceStatusType& type)
{
    DEV_HILOGI(INNERKIT, "Enter");
    DeviceStatusClient::GetInstance().UnsubscribeCallback(type, callback_);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS