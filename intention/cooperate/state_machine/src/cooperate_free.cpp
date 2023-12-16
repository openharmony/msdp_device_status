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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateFree" };
} // namespace
CooperateFree::CooperateFree()
{
    auto initial = std::make_shared<Initial>(*this);
    Initial::BuildChains(initial, *this);
    current_ = initial;
}

void CooperateFree::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    if (current_ != nullptr) {
        current_->OnEvent(context, event);
    } else {
        FI_HILOGE("In CooperateFree::OnEvent, current step is null");
    }
}

void CooperateFree::OnEnterState(Context &context)
{
    CALL_INFO_TRACE;
}

void CooperateFree::OnLeaveState(Context &context)
{
    CALL_INFO_TRACE;
}

CooperateFree::Initial::Initial(CooperateFree &parent) : ICooperateStep(parent, nullptr)
{
    CALL_INFO_TRACE;
}

void CooperateFree::Initial::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    switch (event.type) {
        case CooperateEventType::DISABLE : {
            break;
        }
        case CooperateEventType::START : {
            OnStart(context, event);
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateFree::Initial::OnProgress(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
}

void CooperateFree::Initial::OnReset(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
}

void CooperateFree::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateFree &parent)
{
    CALL_INFO_TRACE;
    auto s1 = std::make_shared<PrepareRemoteInput>(parent, self);
    self->start_ = s1;
    auto s2 = std::make_shared<StartRemoteInput>(parent, s1);
    s1->SetNext(s2);
    s2->SetNext(self);
}

void CooperateFree::Initial::OnStart(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    StartCooperateEvent ev = std::get<StartCooperateEvent>(event.event);
    context.cooperated_ = ev.remoteNetworkId;
    if (start_ != nullptr) {
        Switch(start_);
        start_->OnProgress(context, event);
    }
}

CooperateFree::PrepareRemoteInput::PrepareRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateFree::PrepareRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    switch (event.type) {
        case CooperateEventType::PREPARE_DINPUT_RESULT: {
            StartRemoteInputResult result = std::get<StartRemoteInputResult>(event.event);
            if (result.success) {
                Proceed(context, event);
            } else {
                Reset(context, event);
            }
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateFree::PrepareRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
}

void CooperateFree::PrepareRemoteInput::OnReset(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    Reset(context, event);
}

CooperateFree::StartRemoteInput::StartRemoteInput(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateFree::StartRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
    switch (event.type) {
        case CooperateEventType::START_DINPUT_RESULT: {
            StartRemoteInputResult result = std::get<StartRemoteInputResult>(event.event);
            if (result.success) {
                context.sender.Send(CooperateEvent(CooperateEventType::UPDATE_STATE,
                    UpdateStateEvent { .current = 1 }));
                Proceed(context, event);
            } else {
                Reset(context, event);
            }
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateFree::StartRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateFree::StartRemoteInput::OnReset(Context &context, CooperateEvent &event)
{}

CooperateFree::OpenSession::OpenSession(CooperateFree &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateFree::OpenSession::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_INFO_TRACE;
}

void CooperateFree::OpenSession::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateFree::OpenSession::OnReset(Context &context, CooperateEvent &event)
{}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS