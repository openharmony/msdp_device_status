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

#include "cooperate_free.h"

#include "devicestatus_define.h"
#include "dsoftbus_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateFree" };
constexpr int32_t DEFAULT_TIMEOUT { 3000 };
constexpr int32_t REPEAT_ONCE { 1 };
constexpr uint32_t P2P_SESSION_CLOSED { 1 };
} // namespace

CooperateFree::CooperateFree(IContext *env)
    : env_(env)
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
    CALL_DEBUG_ENTER;
    current_->OnEvent(context, event);
}

void CooperateFree::OnEnterState(Context &context)
{}

void CooperateFree::OnLeaveState(Context &context)
{}

void CooperateFree::RegisterDInputSessionCb(Context &context)
{
    CALL_DEBUG_ENTER;
    env_->GetDInput().RegisterSessionStateCb(
        [sender = context.Sender()](uint32_t status) mutable {
            if (status == P2P_SESSION_CLOSED) {
                sender.Send(CooperateEvent(CooperateEventType::DINPUT_SESSION_CLOSED));
            }
        });
}

CooperateFree::Initial::Initial(CooperateFree &parent)
    : ICooperateStep(parent, nullptr)
{}

void CooperateFree::Initial::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    switch (event.type) {
        case CooperateEventType::DISABLE: {
            break;
        }
        case CooperateEventType::START: {
            OnStart(context, event);
            break;
        }
        case CooperateEventType::DSOFTBUS_START_COOPERATE: {
            OnRemoteStart(context, event);
            break;
        }
        default: {
            FI_HILOGW("cooperate event type is: %{public}d", event.type);
            break;
        }
    }
}

void CooperateFree::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::BuildChains(std::shared_ptr<Initial> initial, CooperateFree &parent)
{
    CALL_DEBUG_ENTER;
    CHKPV(initial);
    auto s11 = std::make_shared<ContactRemote>(parent, initial);
    initial->start_ = s11;
    auto s12 = std::make_shared<PrepareRemoteInput>(parent, s11);
    s11->SetNext(s12);
    auto s13 = std::make_shared<StartRemoteInput>(parent, s12);
    s12->SetNext(s13);
    s13->SetNext(initial);

    auto s21 = std::make_shared<RemoteStart>(parent, initial);
    initial->remoteStart_ = s21;
    s21->SetNext(initial);
}

void CooperateFree::Initial::RemoveChains(std::shared_ptr<Initial> initial)
{
    CHKPV(initial);
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
    CALL_DEBUG_ENTER;
    StartCooperateEvent ev = std::get<StartCooperateEvent>(event.event);
    context.StartCooperate(ev);
    context.eventMgr_.StartCooperate(ev);

    if (start_ != nullptr) {
        Switch(start_);
        start_->OnProgress(context, event);
    }
}

void CooperateFree::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    if (remoteStart_ != nullptr) {
        Switch(remoteStart_);
        remoteStart_->OnProgress(context, event);
    }
}

CooperateFree::ContactRemote::ContactRemote(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{}

void CooperateFree::ContactRemote::OnEvent(Context &context, const CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::DSOFTBUS_START_COOPERATE_RESPONSE: {
            OnResponse(context, event);
            break;
        }
        default:
            break;
    }
}

void CooperateFree::ContactRemote::OnResponse(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    DSoftbusStartCooperateResponse resp = std::get<DSoftbusStartCooperateResponse>(event.event);
    if (!context.IsPeer(resp.networkId)) {
        return;
    }
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (resp.normal) {
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateFree::ContactRemote::OnProgress(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(context.dsoftbus_);
    CHKPV(parent_.env_);
    const std::string remoteNetworkId = context.Peer();
    int32_t ret = context.dsoftbus_->OpenSession(remoteNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to connect to \'%{public}s\'", GetAnonyString(remoteNetworkId).c_str());
        OnReset(context, event);
        return;
    }
    DSoftbusStartCooperate request {
        .networkId = context.Origin(),
    };
    ret = context.dsoftbus_->StartCooperate(remoteNetworkId, request);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to contact with \'%{public}s\'", GetAnonyString(remoteNetworkId).c_str());
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
    DSoftbusStartCooperateFinished ev {
        .networkId = context.Origin(),
        .success = false,
    };
    context.eventMgr_.StartCooperateFinish(ev);
    Reset(context, event);
}

CooperateFree::PrepareRemoteInput::PrepareRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{}

void CooperateFree::PrepareRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    switch (event.type) {
        case CooperateEventType::DINPUT_PREPARE_RESULT: {
            parent_.env_->GetTimerManager().RemoveTimer(timerId_);
            DInputPrepareResult result = std::get<DInputPrepareResult>(event.event);
            if (result.success) {
                Proceed(context, event);
            } else {
                OnReset(context, event);
            }
            break;
        }
        default: {
            FI_HILOGW("cooperate event type is: %{public}d", event.type);
            break;
        }
    }
}

void CooperateFree::PrepareRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    int32_t ret = parent_.env_->GetDInput().PrepareRemoteInput(context.Peer(), context.Origin(),
        [sender = context.Sender(), remoteNetworkId = context.Peer(),
         originNetworkId = context.Origin(),
         startDeviceId = context.StartDeviceId()](bool isSuccess) mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DINPUT_PREPARE_RESULT,
                DInputPrepareResult {
                    .remoteNetworkId = remoteNetworkId,
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = isSuccess,
                }));
        });
    if (ret != RET_OK) {
        FI_HILOGE("Failed to prepare remote input");
        OnReset(context, event);
        return;
    }
    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId = context.Peer(),
         originNetworkId = context.Origin(),
         startDeviceId = context.StartDeviceId()]() mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DINPUT_PREPARE_RESULT,
                DInputPrepareResult {
                    .remoteNetworkId = remoteNetworkId,
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = false,
                }));
        });
}

void CooperateFree::PrepareRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(context.dsoftbus_);
    DSoftbusStartCooperateFinished ev {
        .networkId = context.Origin(),
        .success = false,
    };
    context.dsoftbus_->StartCooperateFinish(context.Peer(), ev);
    Reset(context, event);
}

CooperateFree::StartRemoteInput::StartRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{}

void CooperateFree::StartRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    switch (event.type) {
        case CooperateEventType::DINPUT_START_RESULT: {
            OnStartFinished(context, event);
            break;
        }
        default: {
            FI_HILOGW("cooperate event type is: %{public}d", event.type);
            break;
        }
    }
}

void CooperateFree::StartRemoteInput::OnStartFinished(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    DInputStartResult ev = std::get<DInputStartResult>(event.event);
    if (!context.IsPeer(ev.remoteNetworkId)) {
        return;
    }
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (ev.success) {
        OnSuccess(context, ev);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateFree::StartRemoteInput::OnSuccess(Context &context, const DInputStartResult &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(context.dsoftbus_);
    parent_.RegisterDInputSessionCb(context);
    DSoftbusStartCooperateFinished ev {
        .networkId = context.Origin(),
        .startDeviceDhid = context.StartDeviceDhid(),
        .success = true,
        .cursorPos = context.CursorPosition(),
    };
    context.dsoftbus_->StartCooperateFinish(context.Peer(), ev);
    context.eventMgr_.StartCooperateFinish(ev);
    context.Sender().Send(CooperateEvent(
        CooperateEventType::UPDATE_STATE,
        UpdateStateEvent {
            .current = CooperateState::COOPERATE_STATE_OUT,
        }));
}

void CooperateFree::StartRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    std::vector<std::string> dhids = context.CooperateDhids();
    if (dhids.empty()) {
        FI_HILOGE("No input device for cooperation");
        OnReset(context, event);
        return;
    }
    int32_t ret = parent_.env_->GetDInput().StartRemoteInput(context.Peer(), context.Origin(), dhids,
        [sender = context.Sender(), remoteNetworkId = context.Peer(),
         originNetworkId = context.Origin(),
         startDeviceId = context.StartDeviceId()](bool isSuccess) mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DINPUT_START_RESULT,
                DInputStartResult {
                    .remoteNetworkId = remoteNetworkId,
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = isSuccess,
                }));
        });
    if (ret != RET_OK) {
        FI_HILOGE("Failed to start remote input");
        OnReset(context, event);
        return;
    }
    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId = context.Peer(),
         originNetworkId = context.Origin(),
         startDeviceId = context.StartDeviceId()]() mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DINPUT_START_RESULT,
                DInputStartResult {
                    .remoteNetworkId = remoteNetworkId,
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = false,
                }));
        });
}

void CooperateFree::StartRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{
    Reset(context, event);
}

CooperateFree::RemoteStart::RemoteStart(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{}

void CooperateFree::RemoteStart::OnEvent(Context &context, const CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::DSOFTBUS_START_COOPERATE_FINISHED: {
            OnRemoteStartFinished(context, event);
            break;
        }
        default: {
            FI_HILOGW("cooperate event type is: %{public}d", event.type);
            break;
        }
    }
}

void CooperateFree::RemoteStart::OnRemoteStartFinished(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(parent_.env_);
    DSoftbusStartCooperateFinished ev = std::get<DSoftbusStartCooperateFinished>(event.event);
    if (!context.IsPeer(ev.networkId)) {
        return;
    }
    parent_.env_->GetTimerManager().RemoveTimer(timerId_);
    if (ev.success) {
        OnSuccess(context, ev);
        Proceed(context, event);
    } else {
        OnReset(context, event);
    }
}

void CooperateFree::RemoteStart::OnSuccess(Context &context, const DSoftbusStartCooperateFinished &event)
{
    parent_.RegisterDInputSessionCb(context);
    context.RemoteStartSuccess(event);
    context.eventMgr_.RemoteStartFinish(event);
    context.Sender().Send(CooperateEvent(
        CooperateEventType::UPDATE_STATE,
        UpdateStateEvent {
            .current = CooperateState::COOPERATE_STATE_IN,
        }));
}

void CooperateFree::RemoteStart::OnProgress(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    CHKPV(context.dsoftbus_);
    CHKPV(parent_.env_);
    DSoftbusStartCooperate req = std::get<DSoftbusStartCooperate>(event.event);
    context.RemoteStart(req);

    DSoftbusStartCooperateResponse resp {
        .networkId = DSoftbusAdapter::GetLocalNetworkId(),
        .normal = true,
    };
    int32_t ret = context.dsoftbus_->StartCooperateResponse(req.networkId, resp);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to answer \'%{public}s\'", GetAnonyString(req.networkId).c_str());
        OnReset(context, event);
        return;
    }
    timerId_ = parent_.env_->GetTimerManager().AddTimer(DEFAULT_TIMEOUT, REPEAT_ONCE,
        [sender = context.Sender(), remoteNetworkId = req.networkId]() mutable {
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
    DSoftbusStartCooperateFinished ev {
        .networkId = context.Peer(),
        .success = false,
    };
    context.eventMgr_.RemoteStartFinish(ev);
    Reset(context, event);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
