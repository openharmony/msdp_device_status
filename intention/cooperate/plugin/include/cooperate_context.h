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

#ifndef COOPERATE_CONTEXT_H
#define COOPERATE_CONTEXT_H

#include "nocopyable.h"

#ifdef ENABLE_PERFORMANCE_CHECK
#include <chrono>
#include <mutex>
#endif // ENABLE_PERFORMANCE_CHECK

#include "cooperate_events.h"
#include "ddm_adapter.h"
#include "ddp_adapter.h"
#include "dsoftbus_handler.h"
#include "event_manager.h"
#include "hot_area.h"
#include "input_event_transmission/input_event_builder.h"
#include "input_event_transmission/input_event_interceptor.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class Context final {
public:
    Context(IContext *env);
    ~Context() = default;
    DISALLOW_COPY_AND_MOVE(Context);

    void AttachSender(Channel<CooperateEvent>::Sender sender);
    void Enable();
    void Disable();

    Channel<CooperateEvent>::Sender Sender() const;
    IDDPAdapter& GetDP() const;
    std::string Local() const;
    std::string Peer() const;
    int32_t StartDeviceId() const;
    Coordinate CursorPosition() const;
    NormalizedCoordinate NormalizedCursorPosition() const;

    bool IsLocal(const std::string &networkId) const;
    bool IsPeer(const std::string &networkId) const;

    void EnableCooperate(const EnableCooperateEvent &event);
    void DisableCooperate(const DisableCooperateEvent &event);
    void StartCooperate(const StartCooperateEvent &event);
    void RemoteStartSuccess(const DSoftbusStartCooperateFinished &event);
    void RelayCooperate(const DSoftbusRelayCooperate &event);
    void OnPointerEvent(const InputPointerEvent &event);

#ifdef ENABLE_PERFORMANCE_CHECK
    void StartTrace(const std::string &name);
    void FinishTrace(const std::string &name);
#endif // ENABLE_PERFORMANCE_CHECK

    DDMAdapter ddm_;
    DSoftbusHandler dsoftbus_;
    EventManager eventMgr_;
    HotArea hotArea_;
    InputEventBuilder inputEventBuilder_;
    InputEventInterceptor inputEventInterceptor_;

private:
    int32_t EnableDDM();
    void DisableDDM();
    int32_t EnableDDP();
    void DisableDDP();
    int32_t EnableDevMgr();
    void DisableDevMgr();
    void SetCursorPosition(const Coordinate &cursorPos);
    void ResetCursorPosition();

    IContext *env_ { nullptr };
    Channel<CooperateEvent>::Sender sender_;
    std::string remoteNetworkId_;
    int32_t startDeviceId_ { -1 };
    Coordinate cursorPos_ {};
    std::shared_ptr<IBoardObserver> boardObserver_;
    std::shared_ptr<IDeviceProfileObserver> dpObserver_;
    std::shared_ptr<IDeviceObserver> hotplugObserver_;

#ifdef ENABLE_PERFORMANCE_CHECK
    std::mutex lock_;
    std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>> traces_;
#endif // ENABLE_PERFORMANCE_CHECK
};

inline Channel<CooperateEvent>::Sender Context::Sender() const
{
    return sender_;
}

inline std::string Context::Local() const
{
    return DSoftbusHandler::GetLocalNetworkId();
}

inline std::string Context::Peer() const
{
    return remoteNetworkId_;
}

inline int32_t Context::StartDeviceId() const
{
    return startDeviceId_;
}

inline Coordinate Context::CursorPosition() const
{
    return cursorPos_;
}

inline bool Context::IsLocal(const std::string &networkId) const
{
    return (networkId == DSoftbusHandler::GetLocalNetworkId());
}

inline bool Context::IsPeer(const std::string &networkId) const
{
    return (networkId == remoteNetworkId_);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_CONTEXT_H
