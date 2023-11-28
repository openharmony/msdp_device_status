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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateIn" };
} // namespace
CooperateIn::CooperateIn()
{}

void CooperateIn::OnEvent(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
}

void CooperateIn::OnEnterState(Context & context)
{
    CALL_DEBUG_ENTER;
}

void CooperateIn::OnLeaveState(Context & context)
{
    CALL_DEBUG_ENTER;
}

void CooperateIn::Initial::OnEvent(Context &context, CooperateEvent &event)
{}

void CooperateIn::Initial::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateIn::Initial::OnReset(Context &context, CooperateEvent &event)
{
    CALL_DEBUG_ENTER;
    switch (event.type) {
        case CooperateEventType::POINTER_MOVE : {
            // 当前设备上连接的鼠标发生移动，重置穿越状态。
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateIn::PrepareRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{}

void CooperateIn::PrepareRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateIn::PrepareRemoteInput::OnReset(Context &context, CooperateEvent &event)
{
    if (context.isUnchain_) {
        // UnprepareRemoteInput(prepared_.first, prepared_.second);
    }
    Reset(context, event);
}

void CooperateIn::StartRemoteInput::OnEvent(Context &context, CooperateEvent &event)
{
    switch (event.type) {
        case CooperateEventType::POINTER_MOVE : {
            PointerMoveEvent e = std::get<PointerMoveEvent>(event.event);
            if (context.startDeviceId_ != context.devMgr_.GetDhid(e.deviceId)) {
                // 当前设备上连接的鼠标发生移动，需要重置穿越状态。
                if (prev_ != nullptr) {
                    prev_->OnReset(context, event);
                }
            }
            break;
        }
        default : {
            break;
        }
    }
}

void CooperateIn::StartRemoteInput::OnProgress(Context &context, CooperateEvent &event)
{}

void CooperateIn::StartRemoteInput::OnReset(Context &context, CooperateEvent &event)
{}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS