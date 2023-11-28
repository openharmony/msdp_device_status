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

#include "cooperate_out.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateOut" };
} // namespace
CooperateOut::CooperateOut()
{
    auto initial = std::make_shared<Initial>(*this);
    Initial::BuildChains(initial, *this);
    current_ = initial;
}

void CooperateOut::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    if (current_ != nullptr) {
        current_->OnEvent(context, event);
    } else {
        FI_HILOGE("Current step is null");
    }
}

void CooperateOut::OnEnterState(Context & context)
{
    CALL_DEBUG_ENTER;
}

void CooperateOut::OnLeaveState(Context & context)
{
    CALL_DEBUG_ENTER;
}

CooperateOut::Initial::Initial(CooperateOut &parent)
    : ICooperateStep(parent, nullptr)
{}

void CooperateOut::Initial::OnEvent(Context &context, CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::POINTER_MOVE : {
            PointerMoveEvent e = std::get<PointerMoveEvent>(event.event);
            if (context.startDeviceId_ != context.devMgr_.GetDhid(e.deviceId)) {
                // 当前谁被上连接的鼠标发生移动，需要重置穿越状态。
                FI_HILOGD("Pointer moved, reset cooperate");
                if (stop_ != nullptr) {
                    Switch (stop_);
                    stop_->OnProgress(context, event);
                }
            }
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateOut::Initial::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateOut::Initial::OnReset(Context &context, CooperateEvent &event)
{}

void CooperateOut::Initial::BuildChains(std::shared_ptr<Initial> self, CooperateOut &parent)
{
    auto s1 = std::make_shared<StopRemoteInput>(parent, self);
    self->stop_ = s1;
    auto s2 = std::make_shared<UnprepareRemoteInput>(parent, self);
    s1->SetNext(s2);
    s2->SetNext(self);
}

CooperateOut::StopRemoteInput::StopRemoteInput(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateOut::StopRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void CooperateOut::StopRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    Proceed(context, event);
}

void CooperateOut::StopRemoteInput::OnReset(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

CooperateOut::UnprepareRemoteInput::UnprepareRemoteInput(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev)
    : ICooperateStep(parent, prev)
{}

void CooperateOut::UnprepareRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void CooperateOut::UnprepareRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{
    FI_HILOGD("unprepare remote input (remote:%{public}s)", context.cooperated_.c_str());
    context.sender.Send(CooperateEvent(CooperateEventType::UPDATE_STATE, UpdateStateEvent {
        .current = 0,
    }));
    Proceed(context, event);
}

void CooperateOut::UnprepareRemoteInput::OnReset(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS