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
{}

void CooperateIn::Initial::OnProgress(Context &context, const CooperateEvent &event)
{}

void CooperateIn::Initial::OnReset(Context &context, const CooperateEvent &event)
{}

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
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
