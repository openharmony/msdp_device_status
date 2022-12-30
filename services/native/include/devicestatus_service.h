/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DEVICESTATUS_SERVICE_H
#define DEVICESTATUS_SERVICE_H

#include <memory>
#include <iremote_object.h>
#include <system_ability.h>

#include "delegate_tasks.h"
#include "device_manager.h"
#include "devicestatus_srv_stub.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_dumper.h"
#include "devicestatus_manager.h"
#include "devicestatus_delayed_sp_singleton.h"
#include "timer_manager.h"
#include "i_context.h"
#include "idevicestatus_callback.h"
#include "uds_server.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class ServiceRunningState {STATE_NOT_START, STATE_RUNNING, STATE_EXIT};
class DeviceStatusService final : public IContext,
                                  public UDSServer,
                                  public SystemAbility,
                                  public DeviceStatusSrvStub {
    DECLARE_SYSTEM_ABILITY(DeviceStatusService)
    DECLARE_DELAYED_SP_SINGLETON(DeviceStatusService);
public:
    virtual void OnDump() override;
    virtual void OnStart() override;
    virtual void OnStop() override;

    IDelegateTasks& GetDelegateTasks() override;
    IDeviceManager& GetDeviceManager() override;
    ITimerManager& GetTimerManager() override;

    int32_t FindInputDeviceId(struct libinput_device* inputDevice) override
    {
        return -1;
    }

    std::shared_ptr<MMI::PointerEvent> GetPointerEvent() override
    {
        return nullptr;
    }

    MouseLocation GetMouseInfo() override
    {
        return MouseLocation();
    }

    int32_t SetPointerVisible(int32_t pid, bool visible) override
    {
        return -1;
    }

    void SelectAutoRepeat(std::shared_ptr<MMI::KeyEvent>& keyEvent) override
    {}

    void SetJumpInterceptState(bool isJump) override
    {}

    bool IsRemote(struct libinput_device *inputDevice) override
    {
        return false;
    }

    const ::OHOS::MMI::DisplayGroupInfo GetDisplayGroupInfo() override
    {
        return ::OHOS::MMI::DisplayGroupInfo();
    }

    void SetAbsolutionLocation(double xPercent, double yPercent) override
    {}

    void Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback) override;
    void Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback) override;
    Data GetCache(const Type& type) override;
    bool IsServiceReady() const;
    std::shared_ptr<DeviceStatusManager> GetDeviceStatusManager() const;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void ReportSensorSysEvent(int32_t type, bool enable);

    int32_t RegisterCoordinationListener() override;
    int32_t UnregisterCoordinationListener() override;
    int32_t EnableInputDeviceCoordination(int32_t userData, bool enable) override;
    int32_t StartInputDeviceCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId) override;
    int32_t StopDeviceCoordination(int32_t userData) override;
    int32_t GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId) override;

    int32_t AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &toReturnClientFd, int32_t &tokenType) override;

    void OnConnected(SessionPtr s) override;
    void OnDisconnected(SessionPtr s) override;
    int32_t AddEpoll(EpollEventType type, int32_t fd) override;
    int32_t DelEpoll(EpollEventType type, int32_t fd);
    bool IsRunning() const override;
private:
    bool Init();
    int32_t InitDelegateTasks();
    int32_t InitTimerMgr();

    void OnThread();
    void OnSignalEvent(int32_t signalFd);
    void OnDelegateTask(const epoll_event &ev);
    void OnTimeout(const epoll_event &ev);
    void OnInputDevMgr(const epoll_event &ev);
    int32_t EnableDevMgr(int32_t nRetries);
    void DisableDevMgr();

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t OnRegisterCoordinationListener(int32_t pid);
    int32_t OnUnregisterCoordinationListener(int32_t pid);
    int32_t OnEnableInputDeviceCoordination(int32_t pid, int32_t userData, bool enabled);
    int32_t OnStartInputDeviceCoordination(int32_t pid, int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId);
    int32_t OnStopInputDeviceCoordination(int32_t pid, int32_t userData);
    int32_t OnGetInputDeviceCoordinationState(int32_t pid, int32_t userData, const std::string &deviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    std::atomic<ServiceRunningState> state_ { ServiceRunningState::STATE_NOT_START };
    std::thread t_;
    DelegateTasks delegateTasks_;
    DeviceManager devMgr_;
    TimerManager timerMgr_;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CoordinationEventHandler coordinationHandler;
#endif // OHOS_BUILD_ENABLE_COOPERATE
    std::atomic<bool> ready_ = false;
    std::shared_ptr<DeviceStatusManager> devicestatusManager_;
    std::shared_ptr<DeviceStatusMsdpClientImpl> msdpImpl_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SERVICE_H
