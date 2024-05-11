/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "cooperate_in.h"

#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "CooperateIn"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

CooperateIn::CooperateIn(IStateMachine &parent, IContext *env)
    : ICooperateState(parent), env_(env)
{
    initial_ = std::make_shared<Initial>(*this);
    Initial::BuildChains(initial_, *this);
    current_ = initial_;
}

CooperateIn::~CooperateIn()
{
    Initial::RemoveChains(initial_);
}

void CooperateIn::OnEvent(Context &context, const CooperateEvent &event)
{
    current_->OnEvent(context, event);
}

void CooperateIn::OnEnterState(Context &context)
{
    CALL_INFO_TRACE;
    env_->GetInput().SetPointerVisibility(true);
}

void CooperateIn::OnLeaveState(Context & context)
{
    CALL_INFO_TRACE;
}

std::set<int32_t> CooperateIn::Initial::filterPointerActions_ {
    MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW,
    MMI::PointerEvent::POINTER_ACTION_LEAVE_WINDOW,
    MMI::PointerEvent::POINTER_ACTION_PULL_IN_WINDOW,
    MMI::PointerEvent::POINTER_ACTION_PULL_OUT_WINDOW,
};

void CooperateIn::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateIn &parent)
{
    auto s11 = std::make_shared<RelayConfirmation>(parent, self);
    self->relay_ = s11;
    s11->SetNext(self);
}

void CooperateIn::Initial::RemoveChains(std::shared_ptr<Initial> self)
{
    if (self->relay_ != nullptr) {
        self->relay_->SetNext(nullptr);
        self->relay_ = nullptr;
    }
}

CooperateIn::Initial::Initial(CooperateIn &parent)
    : ICooperateStep(parent, nullptr), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateIn::Initial::OnDisable, this);
    AddHandler(CooperateEventType::START, &CooperateIn::Initial::OnStart, this);
    AddHandler(CooperateEventType::STOP, &CooperateIn::Initial::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateIn::Initial::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateIn::Initial::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateIn::Initial::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &CooperateIn::Initial::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateIn::Initial::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE, &CooperateIn::Initial::OnRemoteStart, this);
    AddHandler(CooperateEventType::DSOFTBUS_STOP_COOPERATE, &CooperateIn::Initial::OnRemoteStop, this);
}

void CooperateIn::Initial::OnDisable(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[disable cooperation] Stop cooperation");
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent startEvent = std::get<StartCooperateEvent>(event.event);

    if (context.IsLocal(startEvent.remoteNetworkId)) {
        DSoftbusStartCooperateFinished result {
            .success = false
        };
        context.eventMgr_.StartCooperateFinish(result);
        return;
    }
    FI_HILOGI("[start] start cooperation(%{public}s, %{public}d)",
        Utility::Anonymize(startEvent.remoteNetworkId).c_str(), startEvent.startDeviceId);
    context.eventMgr_.StartCooperate(startEvent);

    if (context.IsPeer(startEvent.remoteNetworkId)) {
        OnComeBack(context, event);
    } else {
        OnRelay(context, event);
    }
}

void CooperateIn::Initial::OnComeBack(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    context.inputEventBuilder_.Disable();
    FI_HILOGI("[come back] To \'%{public}s\'", Utility::Anonymize(context.Peer()).c_str());

    DSoftbusComeBack notice {
        .originNetworkId = context.Local(),
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.ComeBack(context.Peer(), notice);
    context.eventMgr_.StartCooperateFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnBack();
}

void CooperateIn::Initial::OnRelay(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent startEvent = std::get<StartCooperateEvent>(event.event);
    parent_.process_.StartCooperate(context, startEvent);
    FI_HILOGI("[relay cooperate] To \'%{public}s\'", Utility::Anonymize(parent_.process_.Peer()).c_str());

    if (relay_ != nullptr) {
        Switch(relay_);
        relay_->OnProgress(context, event);
    }
}

void CooperateIn::Initial::OnStop(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StopCooperateEvent param = std::get<StopCooperateEvent>(event.event);

    context.eventMgr_.StopCooperate(param);
    parent_.StopCooperate(context, event);

    DSoftbusStopCooperateFinished notice {
        .normal = true,
    };
    context.eventMgr_.StopCooperateFinish(notice);

    parent_.UnchainConnections(context, param);
}

void CooperateIn::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusStartCooperate notice = std::get<DSoftbusStartCooperate>(event.event);

    if (context.IsPeer(notice.networkId) || context.IsLocal(notice.networkId)) {
        return;
    }
    context.eventMgr_.RemoteStart(notice);

    DSoftbusStopCooperate stopNotice {};
    context.dsoftbus_.StopCooperate(context.Peer(), stopNotice);

    context.RemoteStartSuccess(notice);
    context.inputEventBuilder_.Update(context);
    context.eventMgr_.RemoteStartFinish(notice);
    FI_HILOGI("[remote start] Cooperation with \'%{public}s\' established", Utility::Anonymize(context.Peer()).c_str());
    context.OnResetCooperation();
}

void CooperateIn::Initial::OnRemoteStop(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice = std::get<DSoftbusStopCooperate>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote stop] Notification from \'%{public}s\'", Utility::Anonymize(notice.networkId).c_str());
    context.eventMgr_.RemoteStop(notice);
    context.inputEventBuilder_.Disable();
    context.eventMgr_.RemoteStopFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnResetCooperation();
}

void CooperateIn::Initial::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[app closed] Close all connections");
    context.dsoftbus_.CloseAllSessions();
    FI_HILOGI("[app closed] Stop cooperation");
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        (filterPointerActions_.find(notice.pointerAction) != filterPointerActions_.end()) ||
        !InputEventBuilder::IsLocalEvent(notice)) {
        return;
    }
    FI_HILOGI("Stop cooperation on operation of local pointer");
    context.OnPointerEvent(notice);
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[board offline] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId).c_str());
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (!context.IsPeer(notice.networkId) || notice.normal) {
        return;
    }
    FI_HILOGI("[switch off] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId).c_str());
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[softbus session closed] Disconnected with \'%{public}s\'",
        Utility::Anonymize(notice.networkId).c_str());
    parent_.StopCooperate(context, event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
    context.CloseDistributedFileConnection(std::string());
}

void CooperateIn::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateIn::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateIn::RelayConfirmation::RelayConfirmation(CooperateIn &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateIn::RelayConfirmation::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateIn::RelayConfirmation::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateIn::RelayConfirmation::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateIn::RelayConfirmation::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateIn::RelayConfirmation::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        &CooperateIn::RelayConfirmation::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED,
        &CooperateIn::RelayConfirmation::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_RELAY_COOPERATE_FINISHED,
        &CooperateIn::RelayConfirmation::OnResponse, this);
}

void CooperateIn::RelayConfirmation::OnDisable(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Disable cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnStop(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Stop cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);

    StopCooperateEvent param = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, param);
}

void CooperateIn::RelayConfirmation::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[app closed] Close all connections");
    context.dsoftbus_.CloseAllSessions();
    FI_HILOGI("[relay cooperate] Stop cooperation on app closed");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        !InputEventBuilder::IsLocalEvent(notice)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Stop cooperation on operation of local pointer");
    context.OnPointerEvent(notice);
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId) && !parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId).c_str());
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
    }
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (notice.normal ||
        (!context.IsPeer(notice.networkId) &&
         !parent_.process_.IsPeer(notice.networkId))) {
        return;
    }
    FI_HILOGI("[relay cooperate] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId).c_str());
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
    }
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (!context.IsPeer(notice.networkId) && !parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId).c_str());
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
        context.eventMgr_.OnSoftbusSessionClosed(notice);
        context.CloseDistributedFileConnection(std::string());
    }
    OnReset(context, event);
}

void CooperateIn::RelayConfirmation::OnResponse(Context &context, const CooperateEvent &event)
{
    DSoftbusRelayCooperateFinished notice = std::get<DSoftbusRelayCooperateFinished>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] \'%{public}s\' respond", Utility::Anonymize(notice.networkId).c_str());
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (notice.normal) {
        OnNormal(context, event);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateIn::RelayConfirmation::OnNormal(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Cooperation with \'%{public}s\' established",
        Utility::Anonymize(parent_.process_.Peer()).c_str());
    context.inputEventBuilder_.Disable();

    DSoftbusStartCooperate notice {
        .originNetworkId = context.Peer(),
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.StartCooperate(parent_.process_.Peer(), notice);

    context.eventMgr_.StartCooperateFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnRelay(parent_.process_.Peer());
}

void CooperateIn::RelayConfirmation::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string remoteNetworkId = parent_.process_.Peer();
    FI_HILOGI("[relay cooperate] Connect \'%{public}s\'", Utility::Anonymize(remoteNetworkId).c_str());
    int32_t ret = context.dsoftbus_.OpenSession(remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("[relay cooperate] Failed to connect to \'%{public}s\'", Utility::Anonymize(remoteNetworkId).c_str());
        OnReset(context, event);
        return;
    }

    FI_HILOGI("[relay cooperate] Notify origin(\'%{public}s\')", Utility::Anonymize(context.Peer()).c_str());
    DSoftbusRelayCooperate notice {
        .targetNetworkId = parent_.process_.Peer(),
    };
    context.dsoftbus_.RelayCooperate(context.Peer(), notice);

    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId = context.Peer()]() mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DSOFTBUS_RELAY_COOPERATE_FINISHED,
                DSoftbusRelayCooperateFinished {
                    .networkId = remoteNetworkId,
                    .normal = false,
                }));
        });
}

void CooperateIn::RelayConfirmation::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] reset cooperation with \'%{public}s\'",
        Utility::Anonymize(parent_.process_.Peer()).c_str());
    DSoftbusStartCooperateFinished result {
        .success = false
    };
    context.eventMgr_.StartCooperateFinish(result);
    Reset(context, event);
}

void CooperateIn::StopCooperate(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("Stop cooperation with \'%{public}s\'", Utility::Anonymize(context.Peer()).c_str());
    context.inputEventBuilder_.Disable();
    context.UpdateCursorPosition();

    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(context.Peer(), notice);

    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnResetCooperation();
}

void CooperateIn::UnchainConnections(Context &context, const StopCooperateEvent &event) const
{
    if (event.isUnchained) {
        FI_HILOGI("Unchain all connections");
        context.dsoftbus_.CloseAllSessions();
        context.eventMgr_.OnUnchain(event);
        context.CloseDistributedFileConnection(std::string());
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
