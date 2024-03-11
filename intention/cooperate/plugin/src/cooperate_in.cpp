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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateIn" };
} // namespace

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
    CALL_DEBUG_ENTER;
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

void CooperateIn::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateIn &parent)
{
    auto s11 = std::make_shared<RelayCooperate>(parent, self);
    self->relay_ = s11;
    s11->SetNext(self);

    auto s21 = std::make_shared<RemoteStart>(parent, self);
    self->remoteStart_ = s21;
    s21->SetNext(self);
}

void CooperateIn::Initial::RemoveChains(std::shared_ptr<Initial> self)
{
    if (self->relay_ != nullptr) {
        self->relay_->SetNext(nullptr);
        self->relay_ = nullptr;
    }
    if (self->remoteStart_ != nullptr) {
        self->remoteStart_->SetNext(nullptr);
        self->remoteStart_ = nullptr;
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
        Utility::Anonymize(startEvent.remoteNetworkId), Utility::Anonymize(startEvent.startDeviceId));
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
    FI_HILOGI("[come back] To \'%{public}s\'", Utility::Anonymize(context.Peer()));

    DSoftbusComeBack notice {
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.ComeBack(context.Peer(), notice);
    context.eventMgr_.StartCooperateFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
}

void CooperateIn::Initial::OnRelay(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent startEvent = std::get<StartCooperateEvent>(event.event);
    parent_.process_.StartCooperate(context, startEvent);
    FI_HILOGI("[relay cooperate] To \'%{public}s\'", Utility::Anonymize(parent_.process_.Peer()));

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
}

void CooperateIn::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusStartCooperate notice = std::get<DSoftbusStartCooperate>(event.event);

    if (context.IsPeer(notice.networkId) || context.IsLocal(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote start] Request from \'%{public}s\'", Utility::Anonymize(notice.networkId));
    parent_.process_.RemoteStart(context, notice);
    context.eventMgr_.RemoteStart(notice);

    if (remoteStart_ != nullptr) {
        Switch(remoteStart_);
        remoteStart_->OnProgress(context, event);
    }
}

void CooperateIn::Initial::OnRemoteStop(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusStopCooperate notice = std::get<DSoftbusStopCooperate>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote stop] Notification from \'%{public}s\'", Utility::Anonymize(notice.networkId));
    context.eventMgr_.RemoteStop(notice);
    context.inputEventBuilder_.Disable();
    context.eventMgr_.RemoteStopFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
}

void CooperateIn::Initial::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[app closed] Stop cooperation");
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        !InputEventBuilder::IsLocalEvent(notice)) {
        return;
    }
    FI_HILOGI("Stop cooperation on operation of local pointer");
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[board offline] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (!context.IsPeer(notice.networkId) || notice.normal) {
        return;
    }
    FI_HILOGI("[switch off] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[softbus session closed] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
}

void CooperateIn::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateIn::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateIn::RelayCooperate::RelayCooperate(CooperateIn &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateIn::RelayCooperate::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateIn::RelayCooperate::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateIn::RelayCooperate::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateIn::RelayCooperate::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateIn::RelayCooperate::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &CooperateIn::RelayCooperate::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateIn::RelayCooperate::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE_RESPONSE, &CooperateIn::RelayCooperate::OnResponse, this);
}

void CooperateIn::RelayCooperate::OnDisable(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Disable cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnStop(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Stop cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnResponse(Context &context, const CooperateEvent &event)
{
    DSoftbusStartCooperateResponse resp = std::get<DSoftbusStartCooperateResponse>(event.event);

    if (!parent_.process_.IsPeer(resp.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] \'%{public}s\' respond", Utility::Anonymize(resp.networkId));
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (resp.normal) {
        OnNormal(context, event);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateIn::RelayCooperate::OnNormal(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Cooperation with \'%{public}s\' established",
        Utility::Anonymize(parent_.process_.Peer()));
    context.inputEventBuilder_.Disable();
    DSoftbusStartCooperateFinished notice {
        .originNetworkId = context.Peer(),
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.StartCooperateFinish(parent_.process_.Peer(), notice);

    DSoftbusRelayCooperate notice1 {
        .targetNetworkId = parent_.process_.Peer(),
    };
    context.dsoftbus_.RelayCooperate(context.Peer(), notice1);

    context.eventMgr_.StartCooperateFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
}

void CooperateIn::RelayCooperate::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] Stop cooperation on app closed");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        !InputEventBuilder::IsLocalEvent(notice)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Stop cooperation on operation of local pointer");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId) && !parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
    }
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (notice.normal ||
        (!context.IsPeer(notice.networkId) &&
         !parent_.process_.IsPeer(notice.networkId))) {
        return;
    }
    FI_HILOGI("[relay cooperate] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId.c_str()));
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
    }
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (!context.IsPeer(notice.networkId) && !parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[relay cooperate] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId.c_str()));
    if (context.IsPeer(notice.networkId)) {
        parent_.StopCooperate(context, event);
    }
    OnReset(context, event);
}

void CooperateIn::RelayCooperate::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string remoteNetworkId = parent_.process_.Peer();

    int32_t ret = context.dsoftbus_.OpenSession(remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("[relay cooperate] Failed to connect to \'%{public}s\'", Utility::Anonymize(remoteNetworkId));
        OnReset(context, event);
        return;
    }
    FI_HILOGI("[relay cooperate] remote:%{public}s", Utility::Anonymize(remoteNetworkId).c_str());
    DSoftbusStartCooperate notice {};

    ret = context.dsoftbus_.StartCooperate(remoteNetworkId, notice);
    if (ret != RET_OK) {
        FI_HILOGE("[relay cooperate] Failed to contact with \'%{public}s\'", Utility::Anonymize(remoteNetworkId));
        OnReset(context, event);
        return;
    }
    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId]() mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DSOFTBUS_START_COOPERATE_RESPONSE,
                DSoftbusStartCooperateResponse {
                    .networkId = remoteNetworkId,
                    .normal = false,
                }));
        });
}

void CooperateIn::RelayCooperate::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[relay cooperate] reset cooperation with \'%{public}s\'", Utility::Anonymize(parent_.process_.Peer()));
    DSoftbusStartCooperateFinished result {
        .success = false
    };
    context.eventMgr_.StartCooperateFinish(result);
    Reset(context, event);
}

CooperateIn::RemoteStart::RemoteStart(CooperateIn &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateIn::RemoteStart::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateIn::RemoteStart::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateIn::RemoteStart::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateIn::RemoteStart::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateIn::RemoteStart::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &CooperateIn::RemoteStart::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateIn::RemoteStart::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE_FINISHED,
        &CooperateIn::RemoteStart::OnRemoteStartFinished, this);
}

void CooperateIn::RemoteStart::OnDisable(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Disable cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RemoteStart::OnStop(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RemoteStart::OnRemoteStartFinished(Context &context, const CooperateEvent &event)
{
    DSoftbusStartCooperateFinished notice = std::get<DSoftbusStartCooperateFinished>(event.event);
    if (!parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote start] Confirmmation from \'%{public}s\'", Utility::Anonymize(notice.networkId));
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (notice.success) {
        OnSuccess(context, notice);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateIn::RemoteStart::OnSuccess(Context &context, const DSoftbusStartCooperateFinished &event)
{
    FI_HILOGI("[remote start] Cooperation with \'%{public}s\' established", Utility::Anonymize(event.networkId));
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(context.Peer(), notice);

    context.RemoteStartSuccess(event);
    context.inputEventBuilder_.Update(context);
    context.eventMgr_.RemoteStartFinish(event);
}

void CooperateIn::RemoteStart::OnAppClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation on app closed");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RemoteStart::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        !InputEventBuilder::IsLocalEvent(notice)) {
        return;
    }
    DSoftbusStopCooperate stopNotice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), stopNotice);

    FI_HILOGI("[remote start] Stop cooperation on operation of local pointer");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateIn::RemoteStart::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (context.IsPeer(notice.networkId)) {
        parent_.env_->GetTimerManager().AddTimer(DEFAULT_COOLING_TIME, REPEAT_ONCE,
            [sender = context.Sender(), event]() mutable {
                sender.Send(event);
            });
    } else if (parent_.process_.IsPeer(notice.networkId)) {
        FI_HILOGI("[remote start] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
        OnReset(context, event);
    }
}

void CooperateIn::RemoteStart::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (notice.normal) {
        return;
    }
    if (context.IsPeer(notice.networkId)) {
        parent_.env_->GetTimerManager().AddTimer(DEFAULT_COOLING_TIME, REPEAT_ONCE,
            [sender = context.Sender(), event]() mutable {
                sender.Send(event);
            });
    } else if (parent_.process_.IsPeer(notice.networkId)) {
        DSoftbusStopCooperate stopNotice {};
        context.dsoftbus_.StopCooperate(parent_.process_.Peer(), stopNotice);
        FI_HILOGI("[remote start] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId));
        OnReset(context, event);
    }
}

void CooperateIn::RemoteStart::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (context.IsPeer(notice.networkId)) {
        parent_.env_->GetTimerManager().AddTimer(DEFAULT_COOLING_TIME, REPEAT_ONCE,
            [sender = context.Sender(), event]() mutable {
                sender.Send(event);
            });
    } else if (parent_.process_.IsPeer(notice.networkId)) {
        FI_HILOGI("[remote start] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
        OnReset(context, event);
    }
}

void CooperateIn::RemoteStart::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string remoteNetworkId = parent_.process_.Peer();
    FI_HILOGI("[remote start] Request from \'%{public}s\'", Utility::Anonymize(remoteNetworkId));

    DSoftbusStartCooperateResponse resp {
        .normal = true,
    };
    int32_t ret = context.dsoftbus_.StartCooperateResponse(remoteNetworkId, resp);
    if (ret != RET_OK) {
        FI_HILOGE("[remote start] Failed to answer \'%{public}s\'", Utility::Anonymize(remoteNetworkId));
        OnReset(context, event);
        return;
    }
    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId]() mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DSOFTBUS_START_COOPERATE_FINISHED,
                DSoftbusStartCooperateFinished {
                    .networkId = remoteNetworkId,
                    .success = false,
                }));
        });
}

void CooperateIn::RemoteStart::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[remote start] reset cooperation with \'%{public}s\'",
        Utility::Anonymize(parent_.process_.Peer()));
    DSoftbusStartCooperateFinished notice {
        .networkId = parent_.process_.Peer(),
        .success = false,
    };
    context.eventMgr_.RemoteStartFinish(notice);
    Reset(context, event);
}

void CooperateIn::StopCooperate(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("Stop cooperation with \'%{public}s\'", Utility::Anonymize(context.Peer()));
    context.inputEventBuilder_.Disable();

    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(context.Peer(), notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
