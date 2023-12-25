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

#include "devicestatus_define.h"

#include "cooperate_free.h"
#include "cooperate_in.h"
#include "cooperate_out.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StateMachine" };
} // namespace

StateMachine::StateMachine(IContext *env)
    : env_(env)
{
    states_[COOPERATE_STATE_FREE] = std::make_shared<CooperateFree>(env);
    states_[COOPERATE_STATE_OUT] = std::make_shared<CooperateOut>(env);
    states_[COOPERATE_STATE_IN] = std::make_shared<CooperateIn>(env);
}

void StateMachine::OnEvent(Context &context, const CooperateEvent &event)
{
    CHKPV(env_);
    switch (event.type) {
        case CooperateEventType::UPDATE_STATE : {
            UpdateState(context, std::get<UpdateStateEvent>(event.event));
            break;
        }
        case CooperateEventType::ENABLE : {
            break;
        }
        case CooperateEventType::DISABLE : {
            break;
        }
        default : {
            states_[current_]->OnEvent(context, event);
            break;
        }
    }
}

void StateMachine::UpdateState(Context &context, const UpdateStateEvent &event)
{
    CALL_DEBUG_ENTER;
    if ((event.current >= COOPERATE_STATE_FREE) &&
        (event.current < NUM_COOPERATE_STATES) &&
        (event.current != current_)) {
        states_[current_]->OnLeaveState(context);
        current_ = event.current;
        states_[current_]->OnEnterState(context);
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
