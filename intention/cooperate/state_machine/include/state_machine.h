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

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <array>
#include <memory>

#include "i_cooperate_state.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class StateMachine {
public:
    enum CooperateState : size_t {
        COOPERATE_STATE_FREE = 0,
        COOPERATE_STATE_OUT,
        COOPERATE_STATE_IN,
        NUM_COOPERATE_STATES,
    };

    StateMachine();
    ~StateMachine() = default;

    int32_t Init(Channel<CooperateEvent>::Sender sender);
    void OnEvent(CooperateEvent &event);

private:
    void UpdateState(UpdateStateEvent &event);

    size_t current_ { 0 };
    std::array<std::shared_ptr<ICooperateState>, NUM_COOPERATE_STATES> states_;
    Context context_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // STATE_MACHINE_H