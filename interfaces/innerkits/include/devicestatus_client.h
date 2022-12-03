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

#ifndef DEVICESTATUS_CLIENT_H
#define DEVICESTATUS_CLIENT_H

#include <singleton.h>

#include "idevicestatus.h"
#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_common.h"
#include "i_coordination_listener.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusClient final : public DelayedRefSingleton<DeviceStatusClient> {
    DECLARE_DELAYED_REF_SINGLETON(DeviceStatusClient)

public:
    DISALLOW_COPY_AND_MOVE(DeviceStatusClient);

    void SubscribeCallback(const DeviceStatusDataUtils::DeviceStatusType& type, \
        const sptr<IdevicestatusCallback>& callback);
    void UnsubscribeCallback(const DeviceStatusDataUtils::DeviceStatusType& type, \
        const sptr<IdevicestatusCallback>& callback);
    DeviceStatusDataUtils::DeviceStatusData GetDeviceStatusData(const DeviceStatusDataUtils::DeviceStatusType& type);

    int32_t RegisterCoordinationListener();
    int32_t UnregisterCoordinationListener();
    int32_t EnableInputDeviceCoordination(int32_t userData, bool enabled);
    int32_t StartInputDeviceCoordination(int32_t userData, const std::string &sinkDeviceId, int32_t srcInputDeviceId);
    int32_t StopDeviceCoordination(int32_t userData);
    int32_t GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId);

    int32_t AllocSocketPair(const int32_t moduleType);
    int32_t GetClientSocketFdOfAllocedSocketPair() const;
private:
    class DeviceStatusDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DeviceStatusDeathRecipient() = default;
        ~DeviceStatusDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        DISALLOW_COPY_AND_MOVE(DeviceStatusDeathRecipient);
    };

    ErrCode Connect();
    sptr<Idevicestatus> devicestatusProxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
    void ResetProxy(const wptr<IRemoteObject>& remote);
    std::mutex mutex_;
    int32_t tokenType_ { -1 };
    int32_t socketFd_ { -1 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // IDEVICESTATUS_H
