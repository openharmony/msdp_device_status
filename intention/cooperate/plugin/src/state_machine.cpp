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
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StateMachine" };
} // namespace

StateMachine::StateMachine(IContext *env)
{
    states_[COOPERATE_STATE_FREE] = std::make_shared<CooperateFree>(env);
    states_[COOPERATE_STATE_OUT] = std::make_shared<CooperateOut>(env);
    states_[COOPERATE_STATE_IN] = std::make_shared<CooperateIn>(env);
}

void StateMachine::OnEvent(Context &context, const CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::UPDATE_STATE: {
            UpdateState(context, event);
            break;
        }
        case CooperateEventType::ENABLE: {
            EnableCooperate(context, event);
            break;
        }
        case CooperateEventType::DISABLE: {
            DisableCooperate(context, event);
            break;
        }
        case CooperateEventType::DDM_BOARD_ONLINE: {
            OnBoardOnline(context, event);
            break;
        }
        case CooperateEventType::DDM_BOARD_OFFLINE: {
            OnBoardOffline(context, event);
            break;
        }
        default: {
            states_[current_]->OnEvent(context, event);
            break;
        }
    }
}

void StateMachine::UpdateState(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    UpdateStateEvent updateEvent = std::get<UpdateStateEvent>(event.event);

    if ((updateEvent.current >= COOPERATE_STATE_FREE) &&
        (updateEvent.current < N_COOPERATE_STATES) &&
        (updateEvent.current != current_)) {
        states_[current_]->OnLeaveState(context);
        current_ = updateEvent.current;
        states_[current_]->OnEnterState(context);
    }
}

void StateMachine::EnableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void StateMachine::DisableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void StateMachine::OnBoardOnline(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DDMBoardOnlineEvent onlineEvent = std::get<DDMBoardOnlineEvent>(event.event);

    auto ret = onlineBoards_.insert(onlineEvent.networkId);
    if (ret.second) {
        context.ddp_->AddWatch(onlineEvent.networkId);
    }
}

void StateMachine::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DDMBoardOfflineEvent offlineEvent = std::get<DDMBoardOfflineEvent>(event.event);

    if (auto iter = onlineBoards_.find(offlineEvent.networkId); iter != onlineBoards_.end()) {
        onlineBoards_.erase(iter);
        context.ddp_->RemoveWatch(offlineEvent.networkId);
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
