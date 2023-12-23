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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateFree" };
constexpr int32_t PREPARE_DINPUT_TIMEOUT { 3000 };
constexpr int32_t REPEAT_ONCE { 1 };
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
    initial_ = nullptr;
}

void CooperateFree::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_INFO_TRACE;
    if (current_ != nullptr) {
        current_->OnEvent(context, event);
    } else {
        FI_HILOGE("In CooperateFree::OnEvent, current step is null");
    }
}

void CooperateFree::OnEnterState(Context &context)
{}

void CooperateFree::OnLeaveState(Context &context)
{}

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
        default: {
            break;
        }
    }
}

void CooperateFree::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

void CooperateFree::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateFree &parent)
{
    CALL_DEBUG_ENTER;
    auto s1 = std::make_shared<PrepareRemoteInput>(parent, self);
    self->start_ = s1;
    auto s2 = std::make_shared<StartRemoteInput>(parent, s1);
    s1->SetNext(s2);
    s2->SetNext(self);
}

void CooperateFree::Initial::RemoveChains(std::shared_ptr<Initial> self)
{
    self->start_->SetNext(nullptr);
    self->start_ = nullptr;
}

void CooperateFree::Initial::OnStart(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    StartCooperateEvent ev = std::get<StartCooperateEvent>(event.event);
    context.remoteNetworkId_ = ev.remoteNetworkId;
    context.startDeviceId_ = ev.startDeviceId;

    if (start_ != nullptr) {
        Switch(start_);
        start_->OnProgress(context, event);
    }
}

CooperateFree::OpenSession::OpenSession(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateFree::OpenSession::OnEvent(Context &context, const CooperateEvent &event)
{}

void CooperateFree::OpenSession::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::OpenSession::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateFree::PrepareRemoteInput::PrepareRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev), parent_(parent)
{}

void CooperateFree::PrepareRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    switch (event.type) {
        case CooperateEventType::PREPARE_DINPUT_RESULT: {
            PrepareRemoteInputResult result = std::get<PrepareRemoteInputResult>(event.event);
            if (result.success) {
                Proceed(context, event);
            } else {
                OnReset(context, event);
            }
            break;
        }
        default: {
            break;
        }
    }
}

void CooperateFree::PrepareRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{
    std::string originNetworkId = context.devMgr_->GetOriginNetworkId(context.startDeviceId_);
    int32_t ret = context.dinput_->PrepareRemoteInput(context.remoteNetworkId_, originNetworkId,
        [sender = context.sender_, remoteNetworkId = context.remoteNetworkId_,
         originNetworkId, startDeviceId = context.startDeviceId_](bool isSuccess) mutable {
            sender.Send(CooperateEvent(CooperateEventType::PREPARE_DINPUT_RESULT, PrepareRemoteInputResult {
                .remoteNetworkId = remoteNetworkId,
                .originNetworkId = originNetworkId,
                .startDeviceId = startDeviceId,
                .success = isSuccess,
            }));
        });
    if (ret != RET_OK) {
        FI_HILOGE("Failed to prepare remote input");
        OnReset(context, event);
    } else {
        CHKPV(parent_.env_);
        timerId_ = parent_.env_->GetTimerManager().AddTimer(PREPARE_DINPUT_TIMEOUT, REPEAT_ONCE,
            [sender = context.sender_, remoteNetworkId = context.remoteNetworkId_,
             originNetworkId, startDeviceId = context.startDeviceId_]() mutable {
                sender.Send(CooperateEvent(CooperateEventType::PREPARE_DINPUT_RESULT, PrepareRemoteInputResult {
                    .remoteNetworkId = remoteNetworkId,
                    .originNetworkId = originNetworkId,
                    .startDeviceId = startDeviceId,
                    .success = false,
                }));
            }
        );
    }
}

void CooperateFree::PrepareRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{}

CooperateFree::StartRemoteInput::StartRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateFree::StartRemoteInput::OnEvent(Context &context, const CooperateEvent &event)
{}

void CooperateFree::StartRemoteInput::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateFree::StartRemoteInput::OnReset(Context &context, const CooperateEvent &event)
{}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
