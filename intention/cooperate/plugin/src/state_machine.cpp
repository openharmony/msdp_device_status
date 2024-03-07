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
#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "StateMachine"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

StateMachine::StateMachine(IContext *env)
    : env_(env)
{
    states_[COOPERATE_STATE_FREE] = std::make_shared<CooperateFree>(*this, env);
    states_[COOPERATE_STATE_OUT] = std::make_shared<CooperateOut>(*this, env);
    states_[COOPERATE_STATE_IN] = std::make_shared<CooperateIn>(*this, env);

    AddHandler(CooperateEventType::REGISTER_LISTENER, &StateMachine::RegisterListener);
    AddHandler(CooperateEventType::UNREGISTER_LISTENER, &StateMachine::UnregisterListener);
    AddHandler(CooperateEventType::REGISTER_HOTAREA_LISTENER, &StateMachine::RegisterHotAreaListener);
    AddHandler(CooperateEventType::UNREGISTER_HOTAREA_LISTENER, &StateMachine::UnregisterHotAreaListener);
    AddHandler(CooperateEventType::ENABLE, &StateMachine::EnableCooperate);
    AddHandler(CooperateEventType::DISABLE, &StateMachine::DisableCooperate);
    AddHandler(CooperateEventType::GET_COOPERATE_STATE, &StateMachine::GetCooperateState);
    AddHandler(CooperateEventType::DDM_BOARD_ONLINE, &StateMachine::OnBoardOnline);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &StateMachine::OnBoardOffline);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &StateMachine::OnProfileChanged);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &StateMachine::OnPointerEvent);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &StateMachine::OnSoftbusSessionClosed);
}

void StateMachine::OnEvent(Context &context, const CooperateEvent &event)
{
    if (auto iter = handlers_.find(event.type); iter != handlers_.end()) {
        iter->second(context, event);
    } else {
        Transfer(context, event);
    }
}

void StateMachine::TransiteTo(Context &context, CooperateState state)
{
    if ((state >= COOPERATE_STATE_FREE) &&
        (state < N_COOPERATE_STATES) &&
        (state != current_)) {
        states_[current_]->OnLeaveState(context);
        current_ = state;
        states_[current_]->OnEnterState(context);
    }
}

void StateMachine::AddHandler(CooperateEventType event,
    void (StateMachine::*handler)(Context&, const CooperateEvent&))
{
    handlers_.emplace(event, std::bind(handler, this, std::placeholders::_1, std::placeholders::_2));
}

void StateMachine::OnQuit(Context &context)
{
    CALL_DEBUG_ENTER;
    RemoveWatches(context);
    RemoveMonitor(context);
}

void StateMachine::RegisterListener(Context &context, const CooperateEvent &event)
{
    RegisterListenerEvent notice = std::get<RegisterListenerEvent>(event.event);
    context.eventMgr_.RegisterListener(notice);
}

void StateMachine::UnregisterListener(Context &context, const CooperateEvent &event)
{
    UnregisterListenerEvent notice = std::get<UnregisterListenerEvent>(event.event);
    context.eventMgr_.UnregisterListener(notice);
}

void StateMachine::RegisterHotAreaListener(Context &context, const CooperateEvent &event)
{
    RegisterHotareaListenerEvent notice = std::get<RegisterHotareaListenerEvent>(event.event);
    context.hotArea_.AddListener(notice);
}

void StateMachine::UnregisterHotAreaListener(Context &context, const CooperateEvent &event)
{
    UnregisterHotareaListenerEvent notice = std::get<UnregisterHotareaListenerEvent>(event.event);
    context.hotArea_.RemoveListener(notice);
}

void StateMachine::EnableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    EnableCooperateEvent enableEvent = std::get<EnableCooperateEvent>(event.event);
    context.EnableCooperate(enableEvent);
    context.eventMgr_.EnableCooperate(enableEvent);
    context.hotArea_.EnableCooperate(enableEvent);
    AddSessionObserver(context, enableEvent);
    AddMonitor(context);
    Transfer(context, event);
}

void StateMachine::DisableCooperate(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DisableCooperateEvent disableEvent = std::get<DisableCooperateEvent>(event.event);
    context.DisableCooperate(disableEvent);
    context.eventMgr_.DisableCooperate(disableEvent);
    RemoveSessionObserver(context, disableEvent);
    RemoveMonitor(context);
    Transfer(context, event);
}

void StateMachine::GetCooperateState(Context &context, const CooperateEvent &event)
{}

void StateMachine::OnBoardOnline(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DDMBoardOnlineEvent onlineEvent = std::get<DDMBoardOnlineEvent>(event.event);

    auto ret = onlineBoards_.insert(onlineEvent.networkId);
    if (ret.second) {
        FI_HILOGD("Watch \'%{public}s\'", Utility::Anonymize(onlineEvent.networkId));
        context.ddp_.AddWatch(onlineEvent.networkId);
        Transfer(context, event);
    }
}

void StateMachine::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DDMBoardOfflineEvent offlineEvent = std::get<DDMBoardOfflineEvent>(event.event);

    if (auto iter = onlineBoards_.find(offlineEvent.networkId); iter != onlineBoards_.end()) {
        onlineBoards_.erase(iter);
        FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(offlineEvent.networkId));
        context.ddp_.RemoveWatch(offlineEvent.networkId);
        Transfer(context, event);
    }
}

void StateMachine::OnProfileChanged(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);
    context.eventMgr_.OnProfileChanged(notice);
    Transfer(context, event);
}

void StateMachine::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    InputPointerEvent pointerEvent = std::get<InputPointerEvent>(event.event);
    context.OnPointerEvent(pointerEvent);
    Transfer(context, event);
}

void StateMachine::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
    Transfer(context, event);
}

void StateMachine::Transfer(Context &context, const CooperateEvent &event)
{
    states_[current_]->OnEvent(context, event);
}

void StateMachine::AddSessionObserver(Context &context, const EnableCooperateEvent &event)
{
    env_->GetSocketSessionManager().AddSessionDeletedCallback(
        event.pid,
        [sender = context.Sender()](SocketSessionPtr session) mutable {
            CHKPV(session);
            FI_HILOGI("Disconnected with AssociateAssistant(%{public}d)", session->GetPid());
            sender.Send(CooperateEvent(CooperateEventType::APP_CLOSED));
        });
}

void StateMachine::RemoveSessionObserver(Context &context, const DisableCooperateEvent &event)
{
    env_->GetSocketSessionManager().RemoveSessionDeletedCallback(event.pid);
}

void StateMachine::AddMonitor(Context &context)
{
    CALL_DEBUG_ENTER;
    if (monitorId_ >= 0) {
        return;
    }
    monitorId_ = env_->GetInput().AddMonitor(
        [sender = context.Sender(), &hotArea = context.hotArea_](
            std::shared_ptr<MMI::PointerEvent> pointerEvent) mutable {
            hotArea.ProcessData(pointerEvent);

            MMI::PointerEvent::PointerItem pointerItem;
            if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
                FI_HILOGE("Corrupted pointer event");
                return;
            }
            sender.Send(CooperateEvent(
                CooperateEventType::INPUT_POINTER_EVENT,
                InputPointerEvent {
                    .deviceId = pointerEvent->GetDeviceId(),
                    .pointerAction = pointerEvent->GetPointerAction(),
                    .sourceType = pointerEvent->GetSourceType(),
                    .position = Coordinate {
                        .x = pointerItem.GetDisplayX(),
                        .y = pointerItem.GetDisplayY(),
                    }
                }));
        });
    if (monitorId_ < 0) {
        FI_HILOGE("MMI::Add Monitor fail");
    }
}

void StateMachine::RemoveMonitor(Context &context)
{
    CALL_DEBUG_ENTER;
    if (monitorId_ < 0) {
        return;
    }
    env_->GetInput().RemoveMonitor(monitorId_);
    monitorId_ = -1;
}

void StateMachine::RemoveWatches(Context &context)
{
    CALL_DEBUG_ENTER;
    for (auto iter = onlineBoards_.begin();
         iter != onlineBoards_.end(); iter = onlineBoards_.begin()) {
        FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(*iter));
        context.ddp_.RemoveWatch(*iter);
        onlineBoards_.erase(iter);
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
