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

#include "cooperate_out.h"

#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "CooperateOut"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

CooperateOut::CooperateOut(IStateMachine &parent, IContext *env)
    : ICooperateState(parent), env_(env)
{
    initial_ = std::make_shared<Initial>(*this);
    Initial::BuildChains(initial_, *this);
    current_ = initial_;
}

CooperateOut::~CooperateOut()
{
    Initial::RemoveChains(initial_);
}

void CooperateOut::OnEvent(Context &context, const CooperateEvent &event)
{
    current_->OnEvent(context, event);
}

void CooperateOut::OnEnterState(Context &context)
{
    CALL_INFO_TRACE;
    env_->GetInput().SetPointerVisibility(false);
}

void CooperateOut::OnLeaveState(Context &context)
{
    CALL_INFO_TRACE;
}

void CooperateOut::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateOut &parent)
{
    auto s11 = std::make_shared<RemoteStart>(parent, self);
    self->remoteStart_ = s11;
    s11->SetNext(self);
}

void CooperateOut::Initial::RemoveChains(std::shared_ptr<Initial> self)
{
    if (self->remoteStart_ != nullptr) {
        self->remoteStart_->SetNext(nullptr);
        self->remoteStart_ = nullptr;
    }
}

CooperateOut::Initial::Initial(CooperateOut &parent)
    : ICooperateStep(parent, nullptr), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateOut::Initial::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateOut::Initial::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateOut::Initial::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_HOTPLUG_EVENT, &CooperateOut::Initial::OnHotplug, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateOut::Initial::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateOut::Initial::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &CooperateOut::Initial::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateOut::Initial::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_COME_BACK, &CooperateOut::Initial::OnComeBack, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE, &CooperateOut::Initial::OnRemoteStart, this);
    AddHandler(CooperateEventType::DSOFTBUS_STOP_COOPERATE, &CooperateOut::Initial::OnRemoteStop, this);
    AddHandler(CooperateEventType::DSOFTBUS_RELAY_COOPERATE, &CooperateOut::Initial::OnRelay, this);
}

void CooperateOut::Initial::OnDisable(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[disable cooperation] Stop cooperation");
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnStop(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StopCooperateEvent param = std::get<StopCooperateEvent>(event.event);

    context.eventMgr_.StopCooperate(param);
    FI_HILOGI("[stop] Stop cooperation with \'%{public}s\', unchain:%{public}d",
        Utility::Anonymize(context.Peer()), param.isUnchained);
    parent_.StopCooperate(context, event);

    DSoftbusStopCooperateFinished notice {
        .normal = true,
    };
    context.eventMgr_.StopCooperateFinish(notice);

    parent_.UnchainConnections(context, param);
}

void CooperateOut::Initial::OnComeBack(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusComeBack notice = std::get<DSoftbusComeBack>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[come back] From \'%{public}s\'", Utility::Anonymize(notice.networkId));
    DSoftbusStartCooperate startEvent {
        .networkId = notice.networkId,
    };
    context.eventMgr_.RemoteStart(startEvent);
    context.inputEventInterceptor_.Disable();

    context.RemoteStartSuccess(notice);
    context.eventMgr_.RemoteStartFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnBack();
}

void CooperateOut::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
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

void CooperateOut::Initial::OnRemoteStop(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice = std::get<DSoftbusStopCooperate>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote stop] Notification from \'%{public}s\'", Utility::Anonymize(notice.networkId));
    context.eventMgr_.RemoteStop(notice);
    context.inputEventInterceptor_.Disable();
    context.eventMgr_.RemoteStopFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnResetCooperation();
}

void CooperateOut::Initial::OnRelay(Context &context, const CooperateEvent &event)
{
    DSoftbusRelayCooperate notice = std::get<DSoftbusRelayCooperate>(event.event);
    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    DSoftbusRelayCooperateFinished resp {
        .targetNetworkId = notice.targetNetworkId,
    };

    int32_t ret = context.dsoftbus_.OpenSession(notice.targetNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("[relay cooperate] Failed to connect to \'%{public}s\'", Utility::Anonymize(notice.targetNetworkId));
        resp.normal = false;
        context.dsoftbus_.RelayCooperateFinish(notice.networkId, resp);
        return;
    }

    resp.normal = true;
    context.dsoftbus_.RelayCooperateFinish(notice.networkId, resp);

    context.RelayCooperate(notice);
    context.inputEventInterceptor_.Update(context);
    FI_HILOGI("[relay cooperate] Relay cooperation to \'%{public}s\'", Utility::Anonymize(context.Peer()));
}

void CooperateOut::Initial::OnHotplug(Context &context, const CooperateEvent &event)
{
    InputHotplugEvent notice = std::get<InputHotplugEvent>(event.event);

    if (notice.deviceId != context.StartDeviceId()) {
        return;
    }
    FI_HILOGI("Stop cooperation on unplug of dedicated pointer");
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[app closed] Stop cooperation");
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        (notice.deviceId == context.StartDeviceId())) {
        return;
    }
    FI_HILOGI("Stop cooperation on operation of undedicated pointer");
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[board offline] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (!context.IsPeer(notice.networkId) || notice.normal) {
        return;
    }
    FI_HILOGI("[switch off] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
}

void CooperateOut::Initial::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[dsoftbus session closed] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
    parent_.StopCooperate(context, event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
    context.CloseDistributedFileConnection(std::string());
}

void CooperateOut::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateOut::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateOut::RemoteStart::RemoteStart(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateOut::RemoteStart::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateOut::RemoteStart::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateOut::RemoteStart::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_HOTPLUG_EVENT, &CooperateOut::RemoteStart::OnHotplug, this);
    AddHandler(CooperateEventType::INPUT_POINTER_EVENT, &CooperateOut::RemoteStart::OnPointerEvent, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateOut::RemoteStart::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED, &CooperateOut::RemoteStart::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateOut::RemoteStart::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE_FINISHED,
        &CooperateOut::RemoteStart::OnRemoteStartFinished, this);
}

void CooperateOut::RemoteStart::OnDisable(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Disable cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateOut::RemoteStart::OnStop(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation");
    parent_.StopCooperate(context, event);
    OnReset(context, event);

    StopCooperateEvent param = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, param);
}

void CooperateOut::RemoteStart::OnRemoteStartFinished(Context &context, const CooperateEvent &event)
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

void CooperateOut::RemoteStart::OnSuccess(Context &context, const DSoftbusStartCooperateFinished &event)
{
    FI_HILOGI("[remote start] Cooperation with \'%{public}s\' established", Utility::Anonymize(event.networkId));
    context.inputEventInterceptor_.Disable();

    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(context.Peer(), notice);

    context.RemoteStartSuccess(event);
    context.inputEventBuilder_.Enable(context);
    context.eventMgr_.RemoteStartFinish(event);
    TransiteTo(context, CooperateState::COOPERATE_STATE_IN);
    context.OnResetCooperation();
}

void CooperateOut::RemoteStart::OnAppClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation on app closed");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateOut::RemoteStart::OnHotplug(Context &context, const CooperateEvent &event)
{
    InputHotplugEvent notice = std::get<InputHotplugEvent>(event.event);

    if (notice.deviceId != context.StartDeviceId()) {
        return;
    }
    FI_HILOGI("[remote start] The dedicated pointer unplugged");
    parent_.env_->GetTimerManager().AddTimer(DEFAULT_COOLING_TIME, REPEAT_ONCE,
        [sender = context.Sender(), event]() mutable {
            sender.Send(event);
        });
}

void CooperateOut::RemoteStart::OnPointerEvent(Context &context, const CooperateEvent &event)
{
    InputPointerEvent notice = std::get<InputPointerEvent>(event.event);

    if ((notice.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        (notice.deviceId == context.StartDeviceId())) {
        return;
    }
    DSoftbusStopCooperate stopNotice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), stopNotice);

    FI_HILOGI("[remote start] Stop cooperation on operation of local pointer");
    parent_.StopCooperate(context, event);
    OnReset(context, event);
}

void CooperateOut::RemoteStart::OnBoardOffline(Context &context, const CooperateEvent &event)
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

void CooperateOut::RemoteStart::OnSwitchChanged(Context &context, const CooperateEvent &event)
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

void CooperateOut::RemoteStart::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (context.IsPeer(notice.networkId)) {
        parent_.env_->GetTimerManager().AddTimer(DEFAULT_COOLING_TIME, REPEAT_ONCE,
            [sender = context.Sender(), event]() mutable {
                sender.Send(event);
            });
    } else if (parent_.process_.IsPeer(notice.networkId)) {
        FI_HILOGI("[remote start] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
        context.CloseDistributedFileConnection(std::string());
        OnReset(context, event);
    }
}

void CooperateOut::RemoteStart::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string remoteNetworkId = parent_.process_.Peer();

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

void CooperateOut::RemoteStart::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[remote start] reset cooperation with \'%{public}s\'", Utility::Anonymize(parent_.process_.Peer()));
    DSoftbusStartCooperateFinished notice {
        .networkId = parent_.process_.Peer(),
        .success = false,
    };
    context.eventMgr_.RemoteStartFinish(notice);
    Reset(context, event);
}

void CooperateOut::StopCooperate(Context &context, const CooperateEvent &event)
{
    context.inputEventInterceptor_.Disable();

    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(context.Peer(), notice);

    TransiteTo(context, CooperateState::COOPERATE_STATE_FREE);
    context.OnResetCooperation();
}

void CooperateOut::UnchainConnections(Context &context, const StopCooperateEvent &event) const
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
