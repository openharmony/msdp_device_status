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

#ifndef DEVICESTATUS_AGENT_H
#define DEVICESTATUS_AGENT_H

#include <memory>

#include "devicestatus_callback_stub.h"
#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusAgent : public std::enable_shared_from_this<DeviceStatusAgent> {
public:
    DeviceStatusAgent() {};
    ~DeviceStatusAgent() {};

    class DeviceStatusAgentEvent {
    public:
        virtual ~DeviceStatusAgentEvent() = default;
        virtual bool OnEventResult(const Data &devicestatusData) = 0;
    };

    class DeviceStatusAgentCallback : public DeviceStatusCallbackStub {
    public:
        explicit DeviceStatusAgentCallback(std::shared_ptr<DeviceStatusAgent> agent) : agent_(agent) {};
        virtual ~DeviceStatusAgentCallback() {};
        void OnDeviceStatusChanged(const Data &devicestatusData) override;

    private:
        std::weak_ptr<DeviceStatusAgent> agent_;
    };

    int32_t SubscribeAgentEvent(Type type, ActivityEvent event, ReportLatencyNs latency,
        std::shared_ptr<DeviceStatusAgent::DeviceStatusAgentEvent> agentEvent);
    int32_t UnsubscribeAgentEvent(Type type, ActivityEvent event);
    friend class DeviceStatusAgentCallback;

private:
    void RegisterServiceEvent(Type type, ActivityEvent event, ReportLatencyNs latency);
    void UnRegisterServiceEvent(Type type, ActivityEvent event);

    sptr<IRemoteDevStaCallback> callback_ { nullptr };
    std::shared_ptr<DeviceStatusAgentEvent> agentEvent_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_AGENT_H
