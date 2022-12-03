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

#ifndef DEVICESTATUS_SRV_PROXY_H
#define DEVICESTATUS_SRV_PROXY_H

#include <nocopyable.h>

#include "iremote_proxy.h"
#include "idevicestatus.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusSrvProxy : public IRemoteProxy<Idevicestatus> {
public:
    explicit DeviceStatusSrvProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<Idevicestatus>(impl) {}
    ~DeviceStatusSrvProxy() = default;
    DISALLOW_COPY_AND_MOVE(DeviceStatusSrvProxy);

    virtual void Subscribe(const DeviceStatusDataUtils::DeviceStatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    virtual void Unsubscribe(const DeviceStatusDataUtils::DeviceStatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    virtual DeviceStatusDataUtils::DeviceStatusData GetCache(const \
        DeviceStatusDataUtils::DeviceStatusType& type) override;

    virtual int32_t RegisterCoordinationListener() override;
    virtual int32_t UnregisterCoordinationListener() override;
    virtual int32_t EnableInputDeviceCoordination(int32_t userData, bool enabled) override;
    virtual int32_t StartInputDeviceCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId) override;
    virtual int32_t StopDeviceCoordination(int32_t userData) override;
    virtual int32_t GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId) override;

    int32_t AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &socketFd, int32_t &tokenType) override;

private:
    static inline BrokerDelegator<DeviceStatusSrvProxy> delegator_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_PROXY_H
