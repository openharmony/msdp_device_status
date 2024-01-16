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

#include "cooperate_in.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr int32_t DEFAULT_TIMEOUT { 3000 };
constexpr int32_t REPEAT_ONCE { 1 };
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateIn" };
} // namespace

CooperateIn::CooperateIn(IContext *env)
    : env_(env)
{}

void CooperateIn::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void CooperateIn::OnEnterState(Context &context)
{
    CALL_DEBUG_ENTER;
    MMI::InputManager::GetInstance()->SetPointerVisible(true);
    MMI::InputManager::GetInstance()->EnableInputDevice(true);
    interceptorId_ = env_->GetInput().AddInterceptor(
        [sender = context.Sender()](std::shared_ptr<MMI::KeyEvent> keyEvent) {});
}

void CooperateIn::OnLeaveState(Context & context)
{
    CALL_DEBUG_ENTER;
    env_->GetInput().RemoveInterceptor(interceptorId_);
    interceptorId_ = -1;
}

void CooperateIn::Initial::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    FI_HILOGE("CooperateIn::Initial");
    switch (event.type) {
        case CooperateEventType::START: {
            std::string localNetworkId = DSoftbusAdapter::GetLocalNetworkId();
            std::string remoteNetworkId = context.Peer();
            if (remoteNetworkId.empty() || localNetworkId.empty() || localNetworkId == remoteNetworkId) {
                FI_HILOGE("start failed");
                return;
            }
            start_->OnProgress(context, event);
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateIn::Initial::OnProgress(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("CooperateIn::Initial::StartRemoteCoordination");
    DSoftbusStartCooperate request {
        .networkId = DSoftbusAdapter::GetLocalNetworkId(),
    };
    int32_t ret = context.dsoftbus_->StartCooperate(context.Peer(), request);
    if (ret != RET_OK) {
        FI_HILOGE("Start cooperate failed");
        return;
    }
    Proceed(context, event);
}

void CooperateIn::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateIn::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateIn &parent)
{
    auto s1 = std::make_shared<StopRemoteInput>(parent, self);
    self->start_ = s1;
    s1->SetNext(self);
}

void CooperateIn::PrepareRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{}

void CooperateIn::PrepareRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateIn::PrepareRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateIn::StartRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{}

void CooperateIn::StartRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateIn::StartRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateIn::StopRemoteInput::StopRemoteInput(CooperateIn &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, nullptr), parent_(parent)
{
    CALL_DEBUG_ENTER;
}

void CooperateIn::StopRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{
    FI_HILOGD("CooperateIn::StopRemoteInput::OnEvent");
    switch (event.type) {
        case CooperateEventType::DINPUT_STOP_RESULT: {
            OnStartFinished(context, event);
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateIn::StopRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string localNetworkId = DSoftbusAdapter::GetLocalNetworkId();
    if (context.IsPeer(localNetworkId)) {
        ComeBack(context, event);
        return;
    }
    RelayComeBack(context, event);
    Proceed(context, event);
}

void CooperateIn::StopRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateIn::StopRemoteInput::ComeBack(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    int32_t startDeviceId = context.StartDeviceId();
    std::vector<std::string> inputDeviceDhids = context.devMgr_->GetCooperateDhids(startDeviceId);
    if (inputDeviceDhids.empty()) {
        OnStartFinished(context, event);
        return;
    }

    int32_t ret = parent_.env_->GetDInput().StopRemoteInput(context.Origin(), inputDeviceDhids,
        [sender = context.Sender(), originNetworkId = context.Origin(),
         startDeviceId = context.StartDeviceId()](bool isSuccess) mutable {
            sender.Send(CooperateEvent(
                CooperateEventType::DINPUT_STOP_RESULT,
                DInputStopResult {
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
                CooperateEventType::DINPUT_STOP_RESULT,
                DInputStopResult {
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = false,
                }));
        });
}

void CooperateIn::StopRemoteInput::OnStartFinished(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
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

void CooperateIn::StopRemoteInput::OnSuccess(Context &context, const DInputStartResult &event)
{
    CALL_INFO_TRACE;
    DSoftbusStartCooperateFinished ev {
        .networkId = context.Origin(),
        .startDeviceDhid = context.StartDeviceDhid(),
        .success = true,
        .cursorPos = context.CursorPosition(),
    };
    SetPointerVisible(context);
    context.dsoftbus_->StartCooperateFinish(context.Peer(), ev);
    context.eventMgr_.StartCooperateFinish(ev);
    context.Sender().Send(CooperateEvent(
        CooperateEventType::UPDATE_STATE,
        UpdateStateEvent {
            .current = CooperateState::COOPERATE_STATE_FREE,
        }));
}

void CooperateIn::StopRemoteInput::SetPointerVisible(Context &context)
{
    CALL_INFO_TRACE;
    bool hasPointer = context.devMgr_->HasLocalPointerDevice();
    FI_HILOGI("hasPointer:%{public}s", hasPointer ? "true" : "false");
    MMI::InputManager::GetInstance()->SetPointerVisible(hasPointer);
}

void CooperateIn::StopRemoteInput::RelayComeBack(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
