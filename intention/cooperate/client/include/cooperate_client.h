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

#ifndef COOPERATE_CLIENT_H
#define COOPERATE_CLIENT_H

#include <functional>
#include <list>
#include <map>
#include <mutex>

#include "nocopyable.h"

#include "coordination_message.h"
#include "i_coordination_listener.h"
#include "i_hotarea_listener.h"
#include "i_tunnel_client.h"
#include "net_packet.h"
#include "stream_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CooperateClient final {
public:
    using CooperateMessageCallback = std::function<void(const std::string&, CoordinationMessage)>;
    using CooperateStateCallback = std::function<void(bool)>;
    using CooperateListenerPtr = std::shared_ptr<ICoordinationListener>;
    using HotAreaListenerPtr = std::shared_ptr<IHotAreaListener>;

    struct CooperateEvent {
        CooperateEvent(CooperateMessageCallback callback) : msgCb(callback) {}
        CooperateEvent(CooperateStateCallback callback) : stateCb(callback) {}

        CooperateMessageCallback msgCb;
        CooperateStateCallback stateCb;
    };

    CooperateClient() = default;
    ~CooperateClient() = default;
    DISALLOW_COPY_AND_MOVE(CooperateClient);

    int32_t RegisterListener(ITunnelClient &tunnel,
        CooperateListenerPtr listener, bool isCheckPermission = false);
    int32_t UnregisterListener(ITunnelClient &tunnel,
        CooperateListenerPtr listener, bool isCheckPermission = false);
    int32_t Enable(ITunnelClient &tunnel,
        CooperateMessageCallback callback, bool isCheckPermission = false);
    int32_t Disable(ITunnelClient &tunnel,
        CooperateMessageCallback callback, bool isCheckPermission = false);
    int32_t Start(ITunnelClient &tunnel,
        const std::string &remoteNetworkId, int32_t startDeviceId,
        CooperateMessageCallback callback, bool isCheckPermission = false);
    int32_t Stop(ITunnelClient &tunnel,
        bool isUnchained, CooperateMessageCallback callback,
        bool isCheckPermission = false);
    int32_t GetCooperateState(ITunnelClient &tunnel,
        const std::string &networkId, CooperateStateCallback callback,
        bool isCheckPermission = false);
    int32_t AddHotAreaListener(ITunnelClient &tunnel, HotAreaListenerPtr listener);
    int32_t RemoveHotAreaListener(ITunnelClient &tunnel, HotAreaListenerPtr listener = nullptr);

    int32_t OnCoordinationListener(const StreamClient &client, NetPacket &pkt);
    int32_t OnCoordinationMessage(const StreamClient &client, NetPacket &pkt);
    int32_t OnCoordinationState(const StreamClient &client, NetPacket &pkt);
    int32_t OnHotAreaListener(const StreamClient &client, NetPacket &pkt);

private:
    int32_t GenerateRequestID();

private:
    std::list<CooperateListenerPtr> devCooperateListener_;
    std::map<int32_t, CooperateEvent> devCooperateEvent_;
    mutable std::mutex mtx_;
    std::atomic_bool isListeningProcess_ { false };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_CLIENT_H
