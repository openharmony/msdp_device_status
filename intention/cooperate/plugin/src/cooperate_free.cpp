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

namespace {
const std::string FINGER_PRINT { "hw_fingerprint_mouse" };
}

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
    env_->GetInput().SetPointerVisibility(visible, 1);
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
        if ((dev == nullptr) || (dev->GetName() == FINGER_PRINT)) {
            return false;
        }
        return (dev->IsPointerDevice() && !IsRemoteInputDevice(dev));
    });
}

void CooperateFree::UnchainConnections(Context &context, const StopCooperateEvent &event) const
{
    CALL_INFO_TRACE;
    if (event.isUnchained) {
        FI_HILOGI("Unchain all connections");
        context.dsoftbus_.CloseAllSessions();
        context.eventMgr_.OnUnchain(event);
        context.CloseDistributedFileConnection(std::string());
    }
}

CooperateFree::Initial::Initial(CooperateFree &parent)
    : ICooperateStep(parent, nullptr), parent_(parent)
{
    AddHandler(CooperateEventType::START, &CooperateFree::Initial::OnStart, this);
    AddHandler(CooperateEventType::STOP, &CooperateFree::Initial::OnStop, this);
    AddHandler(CooperateEventType::APP_CLOSED, &CooperateFree::Initial::OnAppClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_SESSION_CLOSED, &CooperateFree::Initial::OnSoftbusSessionClosed, this);
    AddHandler(CooperateEventType::DSOFTBUS_START_COOPERATE, &CooperateFree::Initial::OnRemoteStart, this);
}

void CooperateFree::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::BuildChains(std::shared_ptr<Initial> initial, CooperateFree &parent)
{}

void CooperateFree::Initial::RemoveChains(std::shared_ptr<Initial> initial)
{}

void CooperateFree::Initial::OnStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent notice = std::get<StartCooperateEvent>(event.event);
    FI_HILOGI("[start cooperation] With \'%{public}s\'", Utility::Anonymize(notice.remoteNetworkId).c_str());
    context.StartCooperate(notice);
    context.eventMgr_.StartCooperate(notice);

    int32_t ret = context.dsoftbus_.OpenSession(context.Peer());
    if (ret != RET_OK) {
        FI_HILOGE("[start cooperation] Failed to connect to \'%{public}s\'",
            Utility::Anonymize(context.Peer()).c_str());
        DSoftbusStartCooperateFinished failNotice {
            .success = false,
        };
        context.eventMgr_.StartCooperateFinish(failNotice);
        return;
    }
    DSoftbusStartCooperate startNotice {
        .originNetworkId = context.Local(),
        .success = true,
        .cursorPos = context.NormalizedCursorPosition(),
    };
    context.dsoftbus_.StartCooperate(context.Peer(), startNotice);
    context.inputEventInterceptor_.Enable(context);
    context.eventMgr_.StartCooperateFinish(startNotice);
    FI_HILOGI("[start cooperation] Cooperation with \'%{public}s\' established",
        Utility::Anonymize(context.Peer()).c_str());
    TransiteTo(context, CooperateState::COOPERATE_STATE_OUT);
    context.OnTransitionOut();
#ifdef ENABLE_PERFORMANCE_CHECK
    std::ostringstream ss;
    ss << "start_cooperation_with_ " << Utility::Anonymize(context.Peer()).c_str();
    context.FinishTrace(ss.str());
#endif // ENABLE_PERFORMANCE_CHECK
}

void CooperateFree::Initial::OnStop(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StopCooperateEvent notice = std::get<StopCooperateEvent>(event.event);
    parent_.UnchainConnections(context, notice);
}

void CooperateFree::Initial::OnAppClosed(Context &context, const CooperateEvent &event)
{
    FI_HILOGI("[app closed] Close all connections");
    context.dsoftbus_.CloseAllSessions();
}

void CooperateFree::Initial::OnSoftbusSessionClosed(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusSessionClosed notice = std::get<DSoftbusSessionClosed>(event.event);
    context.eventMgr_.OnSoftbusSessionClosed(notice);
    context.CloseDistributedFileConnection(std::string());
}

void CooperateFree::Initial::OnRemoteStart(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    DSoftbusStartCooperate notice = std::get<DSoftbusStartCooperate>(event.event);
    context.eventMgr_.RemoteStart(notice);
    context.RemoteStartSuccess(notice);
    context.inputEventBuilder_.Enable(context);
    context.eventMgr_.RemoteStartFinish(notice);
    FI_HILOGI("[remote start] Cooperation with \'%{public}s\' established", Utility::Anonymize(context.Peer()).c_str());
    TransiteTo(context, CooperateState::COOPERATE_STATE_IN);
    context.OnTransitionIn();
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
