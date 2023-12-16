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

#include "state_machine.h"

#include "cooperate_free.h"
#include "cooperate_in.h"
#include "cooperate_out.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StateMachine" };
} // namespace

StateMachine::StateMachine()
    : states_ {
        std::make_shared<CooperateFree>(),
        std::make_shared<CooperateOut>(),
        std::make_shared<CooperateIn>(),
    }
{
}

int32_t StateMachine::Init(Channel<CooperateEvent>::Sender sender)
{
    CALL_DEBUG_ENTER;
    context_.sender = sender;
    return RET_OK;
}

void StateMachine::OnEvent(CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::UPDATE_STATE : {
            UpdateState(std::get<UpdateStateEvent>(event.event));
            break;
        }
        case CooperateEventType::ENABLE : {
            break;
        }
        case CooperateEventType::DISABLE : {
            break;
        }
        default : {
            if (states_[current_] != nullptr) {
                states_[current_]->OnEvent(context_, event);
            }
            break;
        }
    }
}

void StateMachine::UpdateState(UpdateStateEvent &event)
{
    if ((event.current < NUM_COOPERATE_STATES) && (event.current != current_)) {
        if (states_[current_] != nullptr) {
            states_[current_]->OnLeave(context_);
        }
        current_ = event.current;
        if (states_[current_] != nullptr) {
            states_[current_]->OnEvent(context_);
        }
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS