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

#ifndef COORDINATION_MANAGER_IMPL_H
#define COORDINATION_MANAGER_IMPL_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <optional>

#include "client.h"
#include "i_coordination_listener.h"
#include "i_hotarea_listener.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationManagerImpl final {
public:
    using FuncCoordinationMessage = std::function<void(const std::string&, CoordinationMessage)>;
    using FuncCoordinationState = std::function<void(bool)>;
    using CoordinationMsg = FuncCoordinationMessage;
    using CoordinationState = FuncCoordinationState;
    using CoordinationListenerPtr = std::shared_ptr<ICoordinationListener>;
    using HotAreaListenerPtr = std::shared_ptr<IHotAreaListener>;
    struct CoordinationEvent {
        CoordinationMsg msg;
        CoordinationState state;
    };

    CoordinationManagerImpl() = default;
    ~CoordinationManagerImpl() = default;

    int32_t RegisterCoordinationListener(CoordinationListenerPtr listener);
    int32_t UnregisterCoordinationListener(CoordinationListenerPtr listener = nullptr);
    int32_t PrepareCoordination(FuncCoordinationMessage callback);
    int32_t UnprepareCoordination(FuncCoordinationMessage callback);
    int32_t ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId,
        FuncCoordinationMessage callback);
    int32_t DeactivateCoordination(bool isUnchained, FuncCoordinationMessage callback);
    int32_t GetCoordinationState(const std::string &deviceId, FuncCoordinationState callback);
    void OnDevCoordinationListener(const std::string deviceId, CoordinationMessage msg);
    void OnCoordinationMessageEvent(int32_t userData, const std::string deviceId, CoordinationMessage msg);
    void OnCoordinationStateEvent(int32_t userData, bool state);
    int32_t GetUserData() const;
    int32_t OnCoordinationListener(const StreamClient& client, NetPacket& pkt);
    int32_t OnCoordinationMessage(const StreamClient& client, NetPacket& pkt);
    int32_t OnCoordinationState(const StreamClient& client, NetPacket& pkt);

    int32_t AddHotAreaListener(HotAreaListenerPtr listener);
    void OnDevHotAreaListener(int32_t displayX, int32_t displayY, HotAreaType type, bool isEdge);
    int32_t OnHotAreaListener(const StreamClient& client, NetPacket& pkt);

private:
    const CoordinationMsg *GetCoordinationMessageEvent(int32_t userData) const;
    const CoordinationState *GetCoordinationStateEvent(int32_t userData) const;

private:
    std::list<CoordinationListenerPtr> devCoordinationListener_;
    std::map<int32_t, CoordinationEvent> devCoordinationEvent_;
    std::list<HotAreaListenerPtr> devHotAreaListener_;
    mutable std::mutex mtx_;
    int32_t userData_ { 0 };
    std::atomic_bool isListeningProcess_ { false };
    bool isHotAreaListener_ { false };
    IClientPtr client_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_MANAGER_IMPL_H
