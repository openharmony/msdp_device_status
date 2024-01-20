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

#ifndef COOPERATE_OUT_H
#define COOPERATE_OUT_H

#include "i_cooperate_state.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class CooperateOut : public ICooperateState {
public:
    CooperateOut(IContext *env);
    ~CooperateOut() = default;

    void OnEvent(Context &context, const CooperateEvent &event) override;
    void OnEnterState(Context &context) override;
    void OnLeaveState(Context &context) override;

private:
    class Initial final : public ICooperateStep {
    public:
        Initial(CooperateOut &parent);
        ~Initial() = default;

        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;

        static void BuildChains(std::shared_ptr<Initial> initial, CooperateOut &parent);
        static void RemoveChains(std::shared_ptr<Initial> initial);

    private:
        void OnStart(Context &context, const CooperateEvent &event);
        void OnRemoteStart(Context &context, const CooperateEvent &event);

        std::shared_ptr<ICooperateStep> stop_ { nullptr };
        std::shared_ptr<ICooperateStep> remoteStart_ { nullptr };
    };

    class StopRemoteInput : public ICooperateStep {
    public:
        StopRemoteInput(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev);
        ~StopRemoteInput() = default;

        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;
    };

    class UnprepareRemoteInput : public ICooperateStep {
    public:
        UnprepareRemoteInput(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev);
        ~UnprepareRemoteInput() = default;

        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;
    };

    class RemoteStart final : public ICooperateStep {
    public:
        RemoteStart(CooperateOut &parent, std::shared_ptr<ICooperateStep> prev);
        ~RemoteStart() = default;

        void OnEvent(Context &context, const CooperateEvent &event) override;
        void OnProgress(Context &context, const CooperateEvent &event) override;
        void OnReset(Context &context, const CooperateEvent &event) override;

    private:
        void OnRemoteStartFinished(Context &context, const CooperateEvent &event);
        void OnSuccess(Context &context, const DSoftbusStartCooperateFinished &event);

        CooperateOut &parent_;
        int32_t timerId_ { -1 };
    };

    void RegisterDInputSessionCb(Context &context);

    IContext *env_ { nullptr };
    std::shared_ptr<Initial> initial_ { nullptr };
    int32_t interceptorId_ { -1 };
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_OUT_H
