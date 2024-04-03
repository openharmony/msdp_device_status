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

#include "cooperate_free.h"

#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "CooperateFree"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

CooperateFree::CooperateFree(IStateMachine &parent, IContext *env)
    : ICooperateState(parent), env_(env)
{
    initial_ = std::make_shared<Initial>(*this);
    Initial::BuildChains(initial_, *this);
    current_ = initial_;
}

CooperateFree::~CooperateFree()
{
    Initial::RemoveChains(initial_);
}

void CooperateFree::OnEvent(Context &context, const CooperateEvent &event)
{
    current_->OnEvent(context, event);
}

void CooperateFree::OnEnterState(Context &context)
{
    CALL_INFO_TRACE;
    bool visible = HasLocalPointerDevice();
    env_->GetInput().SetPointerVisibility(visible);
}

void CooperateFree::OnLeaveState(Context &context)
{
    CALL_INFO_TRACE;
}

bool CooperateFree::IsRemoteInputDevice(std::shared_ptr<IDevice> dev) const
{
    return (dev->GetName().find("DistributedInput ") != std::string::npos);
}

bool CooperateFree::HasLocalPointerDevice() const
{
    return env_->GetDeviceManager().AnyOf([this](std::shared_ptr<IDevice> dev) {
        return ((dev != nullptr) && dev->IsPointerDevice() && !IsRemoteInputDevice(dev));
    });
}

void CooperateFree::UnchainConnections(Context &context, const StopCooperateEvent &event) const
{
    if (event.isUnchained) {
        FI_HILOGI("Unchain all connections");
        context.dsoftbus_.CloseAllSessions();
        context.eventMgr_.OnUnchain(event);
    }
}

CooperateFree::Initial::Initial(CooperateFree &parent)
    : ICooperateStep(parent, nullptr), parent_(parent)
{
    AddHandler(CooperateEventType::START, &CooperateFree::Initial::OnStart, this);
    AddHandler(CooperateEventType::STOP, &CooperateFree::Initial::OnStop, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateFree::Initial::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE, &CooperateFree::Initial::OnRemoteStart, this);
}

void CooperateFree::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::BuildChains(std::shared_ptr<Initial> initial, CooperateFree &parent)
{
    CALL_DEBUG_ENTER;
    auto s11 = std::make_shared<ContactRemote>(parent, initial);
    initial->start_ = s11;
    s11->SetNext(initial);

    auto s21 = std::make_shared<RemoteStart>(parent, initial);
    initial->remoteStart_ = s21;
    s21->SetNext(initial);
}

void CooperateFree::Initial::RemoveChains(std::shared_ptr<Initial> initial)
{
    if (initial->start_ != nullptr) {
        initial->start_->SetNext(nullptr);
        initial->start_ = nullptr;
    }
    if (initial->remoteStart_ != nullptr) {
        initial->remoteStart_->SetNext(nullptr);
        initial->remoteStart_ = nullptr;
    }
}

void CooperateFree::Initial::OnStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent notice = std::get<StartCooperateEvent>(event.event);
    context.StartCooperate(notice);
    context.eventMgr_.StartCooperate(notice);

    if (start_ != nullptr) {
        Switch(start_);
        start_->OnProgress(context, event);
    }
}

void CooperateFree::Initial::OnStop(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StopCooperateEvent notice = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, notice);
}

void CooperateFree::Initial::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
}

void CooperateFree::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusStartCooperate notice = std::get<DSoftbusStartCooperate>(event.event);
    parent_.process_.RemoteStart(context, notice);
    context.eventMgr_.RemoteStart(notice);

    if (remoteStart_ != nullptr) {
        Switch(remoteStart_);
        remoteStart_->OnProgress(context, event);
    }
}

CooperateFree::ContactRemote::ContactRemote(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateFree::ContactRemote::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateFree::ContactRemote::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateFree::ContactRemote::OnAppClosed, this);
    AddHandler(CooperateEventType::INPUT_HOTPLUG_EVENT, &CooperateFree::ContactRemote::OnHotplug, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateFree::ContactRemote::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        &CooperateFree::ContactRemote::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED,
        &CooperateFree::ContactRemote::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE_RESPONSE,
        &CooperateFree::ContactRemote::OnResponse, this);
}

void CooperateFree::ContactRemote::OnDisable(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[contact remote] Disable cooperation");
    OnReset(context, event);
}

void CooperateFree::ContactRemote::OnStop(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[contact remote] Stop cooperation");
    OnReset(context, event);

    StopCooperateEvent notice = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, notice);
}

void CooperateFree::ContactRemote::OnResponse(Context &context, const CooperateEvent &event)
{
    DSoftbusStartCooperateResponse resp = std::get<DSoftbusStartCooperateResponse>(event.event);
    if (!context.IsPeer(resp.networkId)) {
        return;
    }
    FI_HILOGI("[contact remote] \'%{public}s\' respond", Utility::Anonymize(resp.networkId));
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (resp.normal) {
        OnNormal(context);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateFree::ContactRemote::OnNormal(Context &context)
{
    FI_HILOGI("[contact remote] Cooperation with \'%{public}s\' established", Utility::Anonymize(context.Peer()));
    context.inputEventInterceptor_.Enable(context);

    DSoftbusStartCooperateFinished notice {
        .originNetworkId = context.Local(),
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.StartCooperateFinish(context.Peer(), notice);
    context.eventMgr_.StartCooperateFinish(notice);
    TransiteTo(context, CooperateState::COOPERATE_STATE_OUT);

#ifdef ENABLE_PERFORMANCE_CHECK
    std::ostringstream ss;
    ss << "start_cooperation_with " << Utility::Anonymize(context.Peer());
    context.FinishTrace(ss.str());
#endif // ENABLE_PERFORMANCE_CHECK
}

void CooperateFree::ContactRemote::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[contact remote] Stop cooperation on app closed");
    OnReset(context, event);
}

void CooperateFree::ContactRemote::OnHotplug(Context &context, const CooperateEvent &event)
{
    InputHotplugEvent notice = std::get<InputHotplugEvent>(event.event);

    if (notice.deviceId != context.StartDeviceId()) {
        return;
    }
    FI_HILOGI("[contact remote] The dedicated pointer unplugged");
    OnReset(context, event);
}

void CooperateFree::ContactRemote::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!context.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[contact remote] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
    OnReset(context, event);
}

void CooperateFree::ContactRemote::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (!context.IsPeer(notice.networkId) || notice.normal) {
        return;
    }
    FI_HILOGI("[contact remote] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId));
    OnReset(context, event);
}

void CooperateFree::ContactRemote::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (context.IsPeer(notice.networkId)) {
        FI_HILOGI("[contact remote] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
        OnReset(context, event);
    }
    context.eventMgr_.OnSoftbusSessionClosed(notice);
}

void CooperateFree::ContactRemote::OnProgress(Context &context, const CooperateEvent &event)
{
    const std::string remoteNetworkId = context.Peer();
    FI_HILOGI("[contact remote] Start cooperation with \'%{public}s\'", Utility::Anonymize(remoteNetworkId));

    int32_t ret = context.dsoftbus_.OpenSession(remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("[contact remote] Failed to connect to \'%{public}s\'", Utility::Anonymize(remoteNetworkId));
        OnReset(context, event);
        return;
    }
    DSoftbusStartCooperate request {};

    ret = context.dsoftbus_.StartCooperate(remoteNetworkId, request);
    if (ret != RET_OK) {
        FI_HILOGE("[contact remote] Failed to contact with \'%{public}s\'", Utility::Anonymize(remoteNetworkId));
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

void CooperateFree::ContactRemote::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[contact remote] Reset cooperation with \'%{public}s\'", Utility::Anonymize(context.Peer()));
    DSoftbusStartCooperateFinished notice {
        .success = false,
    };
    context.eventMgr_.StartCooperateFinish(notice);
    Reset(context, event);
}

CooperateFree::RemoteStart::RemoteStart(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{
    AddHandler(CooperateEventType::DISABLE, &CooperateFree::RemoteStart::OnDisable, this);
    AddHandler(CooperateEventType::STOP, &CooperateFree::RemoteStart::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateFree::RemoteStart::OnAppClosed, this);
    AddHandler(CooperateEventType::DDM_BOARD_OFFLINE, &CooperateFree::RemoteStart::OnBoardOffline, this);
    AddHandler(CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        &CooperateFree::RemoteStart::OnSwitchChanged, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED,
        &CooperateFree::RemoteStart::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE_FINISHED,
        &CooperateFree::RemoteStart::OnRemoteStartFinished, this);
}

void CooperateFree::RemoteStart::OnDisable(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Disable cooperation");
    OnReset(context, event);
}

void CooperateFree::RemoteStart::OnStop(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation");
    OnReset(context, event);

    StopCooperateEvent param = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, param);
}

void CooperateFree::RemoteStart::OnRemoteStartFinished(Context &context, const CooperateEvent &event)
{
    DSoftbusStartCooperateFinished notice = std::get<DSoftbusStartCooperateFinished>(event.event);
    if (!parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    FI_HILOGI("[remote start] Confirmation from \'%{public}s\',success:%{public}d",
        Utility::Anonymize(notice.originNetworkId), notice.success);
    if (notice.success) {
        OnSuccess(context, notice);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateFree::RemoteStart::OnSuccess(Context &context, const DSoftbusStartCooperateFinished &event)
{
    context.RemoteStartSuccess(event);
    FI_HILOGI("[remote start] Cooperation with \'%{public}s\' established",
        Utility::Anonymize(parent_.process_.Peer()));
    context.inputEventBuilder_.Enable(context);
    context.eventMgr_.RemoteStartFinish(event);
    TransiteTo(context, CooperateState::COOPERATE_STATE_IN);
}

void CooperateFree::RemoteStart::OnAppClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusStopCooperate notice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), notice);

    FI_HILOGI("[remote start] Stop cooperation on app closed");
    OnReset(context, event);
}

void CooperateFree::RemoteStart::OnBoardOffline(Context &context, const CooperateEvent &event)
{
    DDMBoardOfflineEvent notice = std::get<DDMBoardOfflineEvent>(event.event);

    if (!parent_.process_.IsPeer(notice.networkId)) {
        return;
    }
    FI_HILOGI("[remote start] Peer(\'%{public}s\') is offline", Utility::Anonymize(notice.networkId));
    OnReset(context, event);
}

void CooperateFree::RemoteStart::OnSwitchChanged(Context &context, const CooperateEvent &event)
{
    DDPCooperateSwitchChanged notice = std::get<DDPCooperateSwitchChanged>(event.event);

    if (!parent_.process_.IsPeer(notice.networkId) || notice.normal) {
        return;
    }
    DSoftbusStopCooperate stopNotice {};
    context.dsoftbus_.StopCooperate(parent_.process_.Peer(), stopNotice);

    FI_HILOGI("[remote start] Peer(\'%{public}s\') switch off", Utility::Anonymize(notice.networkId));
    OnReset(context, event);
}

void CooperateFree::RemoteStart::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);

    if (parent_.process_.IsPeer(notice.networkId)) {
        FI_HILOGI("[remote start] Disconnected with \'%{public}s\'", Utility::Anonymize(notice.networkId));
        OnReset(context, event);
    }
    context.eventMgr_.OnSoftbusSessionClosed(notice);
}

void CooperateFree::RemoteStart::OnProgress(Context &context, const CooperateEvent &event)
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

void CooperateFree::RemoteStart::OnReset(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[remote start] Reset cooperation with \'%{public}s\'", Utility::Anonymize(parent_.process_.Peer()));
    DSoftbusStartCooperateFinished notice {
        .networkId = parent_.process_.Peer(),
        .success = false,
    };
    context.eventMgr_.RemoteStartFinish(notice);
    Reset(context, event);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
