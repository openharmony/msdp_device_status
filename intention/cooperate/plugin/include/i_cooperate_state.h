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

#ifndef I_COOPERATE_STATE_H
#define I_COOPERATE_STATE_H

#include <memory>

#include "cooperate_context.h"
#include "cooperate_events.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class ICooperateState {
public:
    ICooperateState() = default;
    virtual ~ICooperateState() = default;

    virtual void OnEvent(Context &context, const CooperateEvent &event) = 0;
    virtual void OnEnterState(Context &context) = 0;
    virtual void OnLeaveState(Context &context) = 0;

protected:
    class ICooperateStep {
    public:
        ICooperateStep(ICooperateState &parent, std::shared_ptr<ICooperateStep> prev);
        virtual ~ICooperateStep() = default;

        virtual void OnEvent(Context &context, const CooperateEvent &event) = 0;
        virtual void OnProgress(Context &context, const CooperateEvent &event) = 0;
        virtual void OnReset(Context &context, const CooperateEvent &event) = 0;

        void SetNext(std::shared_ptr<ICooperateStep> next);

    protected:
        void Switch(std::shared_ptr<ICooperateStep> step);
        void Proceed(Context &context, const CooperateEvent &event);
        void Reset(Context &context, const CooperateEvent &event);

        ICooperateState &parent_;
        std::shared_ptr<ICooperateStep> prev_ { nullptr };
        std::shared_ptr<ICooperateStep> next_ { nullptr };
    };

    void Switch(std::shared_ptr<ICooperateStep> step);

    std::shared_ptr<ICooperateStep> current_ { nullptr };
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_COOPERATE_STATE_H
