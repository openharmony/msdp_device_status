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

#include "i_cooperate_state.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "ICooperateState" };
} // namespace

ICooperateState::ICooperateStep::ICooperateStep(ICooperateState &parent, std::shared_ptr<ICooperateStep> prev)
    : parent_(parent), prev_(prev)
{}

void ICooperateState::Switch(std::shared_ptr<ICooperateStep> step)
{
    CHKPV(step);
    current_ = step;
}

void ICooperateState::ICooperateStep::SetNext(std::shared_ptr<ICooperateStep> next)
{
    if (next != nullptr) {
        next_ = next;
    } else if (next_ != nullptr) {
        next_->SetNext(nullptr);
        next_ = nullptr;
    }
}

void ICooperateState::ICooperateStep::Switch(std::shared_ptr<ICooperateStep> step)
{
    CHKPV(step);
    parent_.Switch(step);
}

void ICooperateState::ICooperateStep::Proceed(Context &context, const CooperateEvent &event)
{
    CHKPV(next_);
    Switch(next_);
    next_->OnProgress(context, event);
}

void ICooperateState::ICooperateStep::Reset(Context &context, const CooperateEvent &event)
{
    CHKPV(prev_);
    Switch(prev_);
    prev_->OnReset(context, event);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
