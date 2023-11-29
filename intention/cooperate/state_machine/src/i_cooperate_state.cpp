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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

ICooperateState::ICooperateStep::ICooperateStep(ICooperateState &parent, std::shared_ptr<ICooperateStep> prev)
    : parent_(parent), prev_(prev)
{}

void ICooperateState::Switch(std::shared_ptr<ICooperateStep> step)
{
    if (setp == nullptr) {
        FI_HILOGE("In ICooperateState::Switch, setp is null");
        return;
}
        current_ = step;
}

void ICooperateState::ICooperateStep::SetNext(std::shared_ptr<ICooperateStep> next)
{
    next_ = next;
}

void ICooperateState::ICooperateStep::Switch(std::shared_ptr<ICooperateStep> step)
{
    parent_.Switch(step);
}
void ICooperateState::ICooperateStep::Proceed(Context &context, CooperateEvent &event)
{
    if (next_ == nullptr) {
       FI_HILOGE("In ICooperateState::ICooperateStep::Proceed, next_  is null");
        return;
}
        Switch(next_);
        next_->OnProgress(context, event);
}

void ICooperateState::ICooperateStep::Reset(Context &context, CooperateEvent &event)
{
    if (prev_ == nullptr) {
         FI_HILOGE("In  ICooperateState::ICooperateStep::Reset, prev_ is null");
        return;
}
        Switch(prev_);
        prev_->OnReset(context, event);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
