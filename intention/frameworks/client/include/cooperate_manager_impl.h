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

#ifndef COOPERATE_MANAGER_IMPL_H
#define COOPERATE_MANAGER_IMPL_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <optional>

#include "cooperate_message.h"
#include "i_cooperate_listener.h"
#include "cooperate_params.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CooperateManagerImpl final {
public:
    using FuncCooperateMessage = std::function<void(const std::string&, CooperateMessage)>;
    using FuncCooperateState = std::function<void(bool)>;
    using CooperateMsg = FuncCooperateMessage;
    using CooperateState = FuncCooperateState;
    using CooperateListenerPtr = std::shared_ptr<ICooperateListener>;
    struct CooperateEvent {
        CooperateMsg msg;
        CooperateState state;
    };

    CooperateManagerImpl() = default;
    ~CooperateManagerImpl() = default;

    int32_t RegisterCooperateListener(CooperateListenerPtr listener);
    int32_t UnregisterCooperateListener(CooperateListenerPtr listener = nullptr);
    int32_t PrepareCooperate(FuncCooperateMessage callback);
    int32_t UnprepareCooperate(FuncCooperateMessage callback);
    int32_t ActivateCooperate(const std::string &remoteNetworkId, int32_t startDeviceId,
        FuncCooperateMessage callback);
    int32_t DeactivateCooperate(bool isUnchained, FuncCooperateMessage callback);
    int32_t GetCooperateState(const std::string &deviceId, FuncCooperateState callback);

private:
    std::list<CooperateListenerPtr> devCooperateListener_;
    std::map<int32_t, CooperateEvent> devCooperateEvent_;
    mutable std::mutex mtx_;
    int32_t userData_ { 0 };
    std::atomic_bool isListeningProcess_ { false };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_MANAGER_IMPL_H
