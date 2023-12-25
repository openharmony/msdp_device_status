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

#ifndef DINPUT_ADAPTER_H
#define DINPUT_ADAPTER_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "distributed_input_kit.h"
#include "i_start_stop_d_inputs_call_back.h"
#include "nocopyable.h"
#include "prepare_d_input_call_back_stub.h"
#include "simulation_event_listener_stub.h"
#include "start_d_input_call_back_stub.h"
#include "start_stop_d_inputs_call_back_stub.h"
#include "start_stop_result_call_back_stub.h"
#include "stop_d_input_call_back_stub.h"
#include "unprepare_d_input_call_back_stub.h"
#include "register_session_state_callback_stub.h"

#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class DInputAdapter final : public std::enable_shared_from_this<DInputAdapter> {
public:
    using DInputCallback = std::function<void(bool)>;

    DInputAdapter(IContext *env);
    ~DInputAdapter() = default;
    DISALLOW_COPY_AND_MOVE(DInputAdapter);

    bool IsNeedFilterOut(const std::string &networkId,
        const DistributedHardware::DistributedInput::BusinessEvent &event);

    int32_t StartRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback);
    int32_t StopRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback);

    int32_t StopRemoteInput(const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback);

    int32_t PrepareRemoteInput(const std::string &remoteNetworkId,
        const std::string &originNetworkId, DInputCallback callback);
    int32_t UnPrepareRemoteInput(const std::string &remoteNetworkId,
        const std::string &originNetworkId, DInputCallback callback);

    int32_t PrepareRemoteInput(const std::string &networkId, DInputCallback callback);
    int32_t UnPrepareRemoteInput(const std::string &networkId, DInputCallback callback);
    int32_t RegisterSessionStateCb(std::function<void(uint32_t)> callback);
    int32_t UnregisterSessionStateCb();

private:
    enum class CallbackType {
        StartDInputCallbackSink,
        StopDInputCallbackSink,
        StopDInputCallbackDHIds,
        PrepareDInputCallback,
        UnprepareDInputCallback,
        PrepareStartDInputCallbackSink,
        UnPrepareStopDInputCallbackSink
    };

    struct TimerInfo {
        int32_t times { 0 };
        int32_t timerId { 0 };
    };

    class StopDInputCallbackDHIds final :
        public DistributedHardware::DistributedInput::StartStopDInputsCallbackStub {
    public:
        StopDInputCallbackDHIds(std::shared_ptr<DInputAdapter> dinput);
        ~StopDInputCallbackDHIds() = default;
        DISALLOW_COPY_AND_MOVE(StopDInputCallbackDHIds);

        void OnResultDhids(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class StartDInputCallbackSink final :
        public DistributedHardware::DistributedInput::StartStopDInputsCallbackStub {
    public:
        StartDInputCallbackSink(std::shared_ptr<DInputAdapter> dinput);
        ~StartDInputCallbackSink() = default;
        DISALLOW_COPY_AND_MOVE(StartDInputCallbackSink);

        void OnResultDhids(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class StopDInputCallbackSink final :
        public DistributedHardware::DistributedInput::StartStopDInputsCallbackStub {
    public:
        StopDInputCallbackSink(std::shared_ptr<DInputAdapter> dinput);
        ~StopDInputCallbackSink() = default;
        DISALLOW_COPY_AND_MOVE(StopDInputCallbackSink);

        void OnResultDhids(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class PrepareDInputCallback final :
        public DistributedHardware::DistributedInput::PrepareDInputCallbackStub {
    public:
        PrepareDInputCallback(std::shared_ptr<DInputAdapter> dinput);
        ~PrepareDInputCallback() = default;
        DISALLOW_COPY_AND_MOVE(PrepareDInputCallback);

        void OnResult(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class UnprepareDInputCallback final :
        public DistributedHardware::DistributedInput::UnprepareDInputCallbackStub {
    public:
        UnprepareDInputCallback(std::shared_ptr<DInputAdapter> dinput);
        ~UnprepareDInputCallback() = default;
        DISALLOW_COPY_AND_MOVE(UnprepareDInputCallback);

        void OnResult(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class PrepareStartDInputCallbackSink final :
        public DistributedHardware::DistributedInput::PrepareDInputCallbackStub {
    public:
        PrepareStartDInputCallbackSink(std::shared_ptr<DInputAdapter> dinput);
        ~PrepareStartDInputCallbackSink() = default;
        DISALLOW_COPY_AND_MOVE(PrepareStartDInputCallbackSink);

        void OnResult(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class UnPrepareStopDInputCallbackSink final :
        public DistributedHardware::DistributedInput::UnprepareDInputCallbackStub {
    public:
        UnPrepareStopDInputCallbackSink(std::shared_ptr<DInputAdapter> dinput);
        ~UnPrepareStopDInputCallbackSink() = default;
        DISALLOW_COPY_AND_MOVE(UnPrepareStopDInputCallbackSink);

        void OnResult(const std::string &devId, const int32_t &status) override;

    private:
        std::weak_ptr<DInputAdapter> dinput_;
    };

    class SessionStateCallback final :
        public DistributedHardware::DistributedInput::RegisterSessionStateCallbackStub {
    public:
        SessionStateCallback() = default;
        ~SessionStateCallback() = default;
        SessionStateCallback(std::function<void(uint32_t)> callback) : callback_(callback) {}
        void OnResult(const std::string &devId, const uint32_t status) override;

    private:
        std::function<void(uint32_t)> callback_;
    };

    void SaveCallback(CallbackType type, DInputCallback callback);
    void AddTimer(const CallbackType &type);
    void RemoveTimer(const CallbackType &type);
    void ProcessDInputCallback(CallbackType type, int32_t status);

    IContext *env_ { nullptr };
    std::map<CallbackType, TimerInfo> watchings_;
    std::map<CallbackType, DInputCallback> callbacks_;
    std::mutex adapterLock_;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DINPUT_ADAPTER_H
