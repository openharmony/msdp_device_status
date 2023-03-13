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
#include "across_device_drag.h"
#include "drag_data.h"
#include "drag_manager.h"
#include "i_context.h"
#include "idevicestatus_callback.h"
#include "stream_server.h"
#include "timer_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class ServiceRunningState {STATE_NOT_START, STATE_RUNNING, STATE_EXIT};
class DeviceStatusService final : public IContext,
                                  public StreamServer,
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
    int32_t EnableCoordination(int32_t userData, bool enable) override;
    int32_t StartCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcDeviceId) override;
    int32_t StopCoordination(int32_t userData) override;
    int32_t GetCoordinationState(int32_t userData, const std::string &deviceId) override;

    int32_t StartDrag(const DragData &dragData) override;
    int32_t StopDrag(int32_t result) override;
    int32_t UpdateDragStyle(int32_t style) override;
    int32_t UpdateDragMessage(const std::u16string &message) override;
    int32_t GetDragTargetPid() override;
    int32_t AddDraglistener() override;
    int32_t RemoveDraglistener() override;
    int32_t SetDragWindowVisible(bool visible) override;

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
    void InitSessionDeathMonitor();

    void OnThread();
    void OnSignalEvent(int32_t signalFd);
    void OnDelegateTask(const epoll_event &ev);
    void OnTimeout(const epoll_event &ev);
    void OnDeviceMgr(const epoll_event &ev);
    int32_t EnableDevMgr(int32_t nRetries);
    void DisableDevMgr();

    int32_t OnStartDrag(const DragData &dragData, int32_t pid);
    int32_t OnStopDrag(int32_t result);

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t OnRegisterCoordinationListener(int32_t pid);
    int32_t OnUnregisterCoordinationListener(int32_t pid);
    int32_t OnEnableCoordination(int32_t pid, int32_t userData, bool enabled);
    int32_t OnStartCoordination(int32_t pid, int32_t userData, const std::string &sinkDeviceId,
        int32_t srcDeviceId);
    int32_t OnStopCoordination(int32_t pid, int32_t userData);
    int32_t OnGetCoordinationState(int32_t pid, int32_t userData, const std::string &deviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    std::atomic<ServiceRunningState> state_ { ServiceRunningState::STATE_NOT_START };
    std::thread t_;
    DelegateTasks delegateTasks_;
    DeviceManager devMgr_;
    TimerManager timerMgr_;
    std::atomic<bool> ready_ = false;
    std::shared_ptr<DeviceStatusManager> devicestatusManager_;
    std::shared_ptr<DeviceStatusMsdpClientImpl> msdpImpl_;
    DragManager dragMgr_;
    AcrossDeviceDrag acrossDeviceDrag_;
};

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SERVICE_H
