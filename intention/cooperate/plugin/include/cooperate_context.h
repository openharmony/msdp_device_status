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

#include <memory>

#include "nocopyable.h"

#include "cooperate_events.h"
#include "dsoftbus_adapter.h"
#include "event_manager.h"
#include "input_device_manager.h"
#include "i_context.h"
#include "i_ddm_adapter.h"
#include "i_ddp_adapter.h"

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
    std::string Origin() const;
    std::string Peer() const;
    std::string StartDeviceDhid() const;
    int32_t StartDeviceId() const;
    std::vector<std::string> CooperateDhids() const;
    Coordinate CursorPosition() const;

    bool IsPeer(const std::string &networkId) const;

    void StartCooperate(const StartCooperateEvent &event);
    void RemoteStart(const DSoftbusStartCooperate &event);
    void RemoteStartSuccess(const DSoftbusStartCooperateFinished &event);

    std::shared_ptr<InputDeviceManager> devMgr_;
    std::shared_ptr<IDDMAdapter> ddm_;
    std::shared_ptr<IDDPAdapter> ddp_;
    std::shared_ptr<DSoftbusAdapter> dsoftbus_;
    EventManager eventMgr_;

private:
    int32_t EnableDevMgr();
    void DisableDevMgr();
    int32_t EnableDSoftbus();
    void DisableDSoftbus();
    int32_t EnableDDM();
    void DisableDDM();
    int32_t EnableDDP();
    void DisableDDP();
    void SetCursorPosition(const DSoftbusStartCooperateFinished &event);

    Channel<CooperateEvent>::Sender sender_;
    std::string originNetworkId_;
    std::string remoteNetworkId_;
    std::string startDeviceDhid_;
    int32_t startDeviceId_ { -1 };
    Coordinate cursorPos_ {};
    std::shared_ptr<IBoardObserver> boardObserver_;
    std::shared_ptr<IDeviceProfileObserver> dpObserver_;
};

inline void Context::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    sender_ = sender;
}

inline Channel<CooperateEvent>::Sender Context::Sender() const
{
    return sender_;
}

inline std::string Context::Origin() const
{
    return originNetworkId_;
}

inline std::string Context::Peer() const
{
    return remoteNetworkId_;
}

inline std::string Context::StartDeviceDhid() const
{
    return startDeviceDhid_;
}

inline int32_t Context::StartDeviceId() const
{
    return startDeviceId_;
}

inline std::vector<std::string> Context::CooperateDhids() const
{
    return devMgr_->GetCooperateDhids(startDeviceId_);
}

inline Coordinate Context::CursorPosition() const
{
    return cursorPos_;
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
