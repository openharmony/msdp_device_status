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
#ifndef COORDINATION_MANAGER_IMPL_H
#define COORDINATION_MANAGER_IMPL_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <optional>

#include "nocopyable.h"

#include "i_coordination_listener.h"
#include "client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationManagerImpl final {
public:
    static CoordinationManagerImpl &GetInstance();
    DISALLOW_COPY_AND_MOVE(CoordinationManagerImpl);
    ~CoordinationManagerImpl() = default;

    using FuncCoordinationMessage = std::function<void(std::string, CoordinationMessage)>;
    using FuncCoordinationState = std::function<void(bool)>;

    using CoordinationMsg = FuncCoordinationMessage;
    using CoordinationState = FuncCoordinationState;

    using CoordinationListenerPtr = std::shared_ptr<ICoordinationListener>;

    struct CoordinationEvent {
        CoordinationMsg msg;
        CoordinationState state;
    };

    int32_t RegisterCoordinationListener(CoordinationListenerPtr listener);
    int32_t UnregisterCoordinationListener(CoordinationListenerPtr listener = nullptr);
    int32_t EnableCoordination(bool enabled, FuncCoordinationMessage callback);
    int32_t StartCoordination(const std::string &sinkDeviceId, int32_t srcDeviceId,
        FuncCoordinationMessage callback);
    int32_t StopCoordination(FuncCoordinationMessage callback);
    int32_t GetCoordinationState(const std::string &deviceId, FuncCoordinationState callback);
    void OnDevCoordinationListener(const std::string deviceId, CoordinationMessage msg);
    void OnCoordinationMessageEvent(int32_t userData, const std::string deviceId, CoordinationMessage msg);
    void OnCoordinationState(int32_t userData, bool state);
    int32_t GetUserData();
    bool InitClient();
private:
    const CoordinationMsg *GetCoordinationMessageEvent(int32_t userData) const;
    const CoordinationState *GetCoordinationStateEvent(int32_t userData) const;

private:
    CoordinationManagerImpl() = default;
    std::list<CoordinationListenerPtr> devCoordinationListener_;
    std::map<int32_t, CoordinationEvent> devCoordinationEvent_;
    std::mutex mtx_;
    int32_t userData_ { 0 };
    bool isListeningProcess_ { false };
    IClientPtr client_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#define CoordinationMgrImpl OHOS::Msdp::DeviceStatus::CoordinationManagerImpl::GetInstance()
#endif // COORDINATION_MANAGER_IMPL_H
