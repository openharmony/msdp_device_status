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

#include "devicestatus_srv_stub.h"
#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_dumper.h"
#include "devicestatus_manager.h"
#include "devicestatus_delayed_sp_singleton.h"
#include "uds_server.h"
#include "delegate_tasks.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class ServiceRunningState {STATE_NOT_START, STATE_RUNNING, STATE_EXIT};   
class DevicestatusService final : public UDSServer, public SystemAbility, public DevicestatusSrvStub {
    DECLARE_SYSTEM_ABILITY(DevicestatusService)
    DECLARE_DELAYED_SP_SINGLETON(DevicestatusService);
public:
    virtual void OnDump() override;
    virtual void OnStart() override;
    virtual void OnStop() override;

    void Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    void UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    DevicestatusDataUtils::DevicestatusData GetCache(const DevicestatusDataUtils::DevicestatusType& type) override;

    int32_t RegisterCoordinationListener() override;
    int32_t UnregisterCoordinationListener() override;
    int32_t EnableInputDeviceCoordination(int32_t userData, bool enable) override;
    int32_t StartInputDeviceCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId) override;
    int32_t StopDeviceCoordination(int32_t userData) override;
    int32_t GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId) override;

    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void ReportMsdpSysEvent(const DevicestatusDataUtils::DevicestatusType& type, bool enable);

    int32_t AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &toReturnClientFd, int32_t &tokenType) override;

    void OnConnected(SessionPtr s) override;
    void OnDisconnected(SessionPtr s) override;
    int32_t AddEpoll(EpollEventType type, int32_t fd) override;
    int32_t DelEpoll(EpollEventType type, int32_t fd);
    bool IsRunning() const override;

    bool InitDelegateTasks();
    void OnThread();
    void OnSignalEvent(int32_t signalFd);
    void OnDelegateTask(epoll_event& ev);

private:
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t OnRegisterCoordinationListener(int32_t pid);
    int32_t OnUnregisterCoordinationListener(int32_t pid);
    int32_t OnEnableInputDeviceCoordination(int32_t pid, int32_t userData, bool enabled);
    int32_t OnStartInputDeviceCoordination(int32_t pid, int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId);
    int32_t OnStopInputDeviceCoordination(int32_t pid, int32_t userData);
    int32_t OnGetInputDeviceCoordinationState(int32_t pid, int32_t userData, const std::string &deviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    std::atomic<ServiceRunningState> state_ = ServiceRunningState::STATE_NOT_START;
    std::thread t_;
    bool Init();
    bool ready_ = false;
    DelegateTasks delegateTasks_;
    std::shared_ptr<DevicestatusManager> devicestatusManager_;
    std::shared_ptr<DevicestatusMsdpClientImpl> msdpImpl_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SERVICE_H
