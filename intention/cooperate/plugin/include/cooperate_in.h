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

#ifndef COOPERATE_IN_H
#define COOPERATE_IN_H

#include "i_cooperate_state.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class CooperateIn final : public ICooperateState {
public:
    CooperateIn(IContext *env);
    ~CooperateIn() = default;

    void OnEvent(Context &context, const CooperateEvent &event) override;
    void OnEnterState(Context &context) override;
    void OnLeaveState(Context &context) override;

private:
    class Initial final : public ICooperateStep {
    public:
        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;

        static void BuildChains(std::shared_ptr<Initial> self, CooperateIn &parent);
    private:
        std::shared_ptr<ICooperateStep> start_ { nullptr };
    };

    class PrepareRemoteInput final : public ICooperateStep {
    public:
        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;

    private:
        std::pair<std::string, std::string> prepared_;
    };

    class StartRemoteInput final : public ICooperateStep {
    public:
        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;
    };

    class StopRemoteInput : public ICooperateStep {
    public:
        StopRemoteInput(CooperateIn &parent, std::shared_ptr<ICooperateStep> prev);
        ~StopRemoteInput() = default;

        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;
    private:
        void ComeBack(Context &context, const CooperateEvent &event);
        void RelayComeBack(Context &context, const CooperateEvent &event);
        void OnStartFinished(Context &context, const CooperateEvent &event);
        void OnSuccess(Context &context, const DInputStartResult &event);
        void SetPointerVisible(Context &context);
    private:
        CooperateIn &parent_;
        int32_t timerId_ { -1 };
    };

    IContext *env_ { nullptr };
    int32_t interceptorId_ { -1 };
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_IN_H
