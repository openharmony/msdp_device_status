/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef COOPERATE_SM_H
#define COOPERATE_SM_H

#include <functional>

#include "singleton.h"

#include "devicestatus_define.h"
#include "device_manager_callback.h"
#include "distributed_input_adapter.h"
#include "dm_device_info.h"
#include "input_manager.h"
#include "i_cooperate_state.h"
#include "i_input_event_consumer.h"
#include "i_input_event_filter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
static constexpr int32_t SUBSTR_NETWORKID_LEN = 6;
enum class CooperateState {
    STATE_FREE = 0,
    STATE_IN = 1,
    STATE_OUT = 2
};

enum class CooperateMsg {
    COOPERATE_ON_SUCCESS = 0,
    COOPERATE_ON_FAIL = 1,
    COOPERATE_OFF_SUCCESS = 2,
    COOPERATE_OFF_FAIL = 3,
    COOPERATE_START = 4,
    COOPERATE_START_SUCCESS = 5,
    COOPERATE_START_FAIL = 6,
    COOPERATE_STOP = 7,
    COOPERATE_STOP_SUCCESS = 8,
    COOPERATE_STOP_FAIL = 9,
    COOPERATE_NULL = 10
};

enum class CooStateChangeType {
    STATE_NONE = -1,
    STATE_FREE_TO_IN = 0,
    STATE_FREE_TO_OUT = 1,
    STATE_IN_TO_FREE = 2,
    STATE_OUT_TO_FREE = 3
};

struct PointerFilter : public MMI::IInputEventFilter {
    bool OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override
    {
        return false;
    }
    inline void UpdateCurrentFilterId(int32_t filterId)
    {
        filterId_ = filterId;
    }
    bool OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
private:
    mutable int32_t filterId_ { -1 };
};

class CooperateSM final {
    DECLARE_DELAYED_SINGLETON(CooperateSM);

    class DeviceInitCallBack : public DistributedHardware::DmInitCallback {
        void OnRemoteDied() override;
    };

    class DmDeviceStateCallback : public DistributedHardware::DeviceStateCallback {
        void OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
        void OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
        void OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
        void OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    };

    class InterceptorConsumer : public MMI::IInputEventConsumer {
    public:
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    };

    class MonitorConsumer : public MMI::IInputEventConsumer {
    public:
        explicit MonitorConsumer(std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : callback_(cb) {}
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_ { nullptr };
    };

public:
    void SetAbsolutionLocation(double xPercent, double yPercent);
    DISALLOW_COPY_AND_MOVE(CooperateSM);
    void Init();
    void OnSessionLost(SessionPtr session);
    void PrepareCooperate();
    void UnprepareCooperate();
    int32_t ActivateCooperate(const std::string &remoteNetworkId, int32_t startDeviceId);
    int32_t DeactivateCooperate(bool isUnchained);
    int32_t GetCooperateState(const std::string &deviceId);
    void StartRemoteCooperate(const std::string &remoteNetworkId, bool buttonIsPressed);
    void StartPointerEventFilter();
    void StartRemoteCooperateResult(bool isSuccess,
        const std::string &startDeviceDhid, int32_t xPercent, int32_t yPercent);
    void StopRemoteCooperate(bool isUnchained);
    void StopRemoteCooperateResult(bool isSuccess);
    void StartCooperateOtherResult(const std::string &remoteNetworkId);
    void UpdateState(CooperateState state);
    void UpdatePreparedDevices(const std::string &remoteNetworkId, const std::string &originNetworkId);
    std::pair<std::string, std::string> GetPreparedDevices() const;
    CooperateState GetCurrentCooperateState() const;
    void OnCooperateChanged(const std::string &networkId, bool isOpen);
    void OnKeyboardOnline(const std::string &dhid);
    void OnPointerOffline(const std::string &dhid, const std::vector<std::string> &keyboards);
    void OnKeyboardOffline(const std::string &dhid);
    bool InitDeviceManager();
    void OnDeviceOnline(const std::string &networkId);
    void OnDeviceOffline(const std::string &networkId);
    void OnStartFinish(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId);
    void OnStopFinish(bool isSuccess, const std::string &remoteNetworkId);
    bool IsStarting() const;
    bool IsStopping() const;
    void Reset(const std::string &networkId);
    void Dump(int32_t fd);
    std::string GetDeviceCooperateState(CooperateState value) const;
    void UpdateLastPointerEventCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    std::shared_ptr<MMI::PointerEvent> GetLastPointerEvent() const;
    void RemoveMonitor();
    void RemoveInterceptor();
    bool IsNeedFilterOut(const std::string &deviceId, const std::shared_ptr<MMI::KeyEvent> keyEvent);
    void RegisterStateChange(CooStateChangeType type,
        std::function<void(CooperateState, CooperateState)> callback);
    bool UnchainCooperate(const std::string &localNetworkId, const std::string &remoteNetworkId);
    void SetUnchainStatus(bool isUnchained);
    void NotifyChainRemoved();
    void NotifyUnchainedResult(const std::string &remoteNetworkId, bool isSuccess);
    void SetSinkNetworkId(const std::string &sinkNetworkId);
    void RegisterRemoteNetworkId(std::function<void(std::string)> callback);
    void RegisterMouseLocation(std::function<void(int32_t, int32_t)> callback);
    void RegisterNotifyDragCancel(std::function<void(void)> callback);
    void OnInterceptorInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
    void OnInterceptorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnMonitorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnSoftbusSessionClosed(const std::string &NetworkId);

private:
    void Reset(bool adjustAbsolutionLocation = false);
    void OnCloseCooperate(const std::string &networkId, bool isLocal);
    void NotifyRemoteStartFail(const std::string &remoteNetworkId);
    void NotifyRemoteStartSuccess(const std::string &remoteNetworkId, const std::string &startDeviceDhid);
    void NotifyRemoteStopFinish(bool isSuccess, const std::string &remoteNetworkId);
    bool UpdateMouseLocation();
    void StateChangedNotify(CooperateState oldState, CooperateState newState);
    void ChangeNotify(CooStateChangeType type, CooperateState oldState, CooperateState newState);
    void NotifyRemoteNetworkId(const std::string &remoteNetworkId);
    void NotifyMouseLocation(int32_t x, int32_t y);
    void SetPointerVisible();
    void OnPostInterceptorKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent);
    void OnPostInterceptorPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnPostMonitorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnReset(const std::string &NetworkId);
    std::shared_ptr<ICooperateState> GetCurrentState();
    void RegisterSessionCallback();

private:
    std::pair<std::string, std::string> preparedNetworkId_;
    std::string startDeviceDhid_;
    std::string remoteNetworkId_;
    std::string sinkNetworkId_;
    bool isUnchained_ { false };
    CooperateState currentState_ { CooperateState::STATE_FREE };
    std::shared_ptr<DistributedHardware::DmInitCallback> initCallback_ { nullptr };
    std::shared_ptr<DistributedHardware::DeviceStateCallback> stateCallback_ { nullptr };
    std::vector<std::string> onlineDevice_;
    mutable std::mutex mutex_;
    std::atomic<bool> isStarting_ { false };
    std::atomic<bool> isStopping_ { false };
    std::pair<int32_t, int32_t> mouseLocation_ { std::make_pair(0, 0) };
    std::shared_ptr<MMI::PointerEvent> lastPointerEvent_ { nullptr };
    int32_t displayX_ { -1 };
    int32_t displayY_ { -1 };
    int32_t interceptorId_ { -1 };
    int32_t monitorId_ { -1 };
    int32_t filterId_ { -1 };
    std::map<CooStateChangeType, std::function<void(CooperateState, CooperateState)>> stateChangedCallbacks_;
    std::function<void(std::string)> remoteNetworkIdCallback_;
    std::function<void(int32_t, int32_t)> mouseLocationCallback_;
    std::function<void(void)> notifyDragCancelCallback_;
    std::map<CooperateState, std::shared_ptr<ICooperateState>> cooperateStates_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_ { nullptr };
    std::shared_ptr<CooperateEventHandler> eventHandler_ { nullptr };
};

#define DIS_HARDWARE DistributedHardware::DeviceManager::GetInstance()
#define COOR_SM OHOS::DelayedSingleton<CooperateSM>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_SM_H
