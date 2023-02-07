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

#ifndef DEVICESTATUS_SRV_PROXY_H
#define DEVICESTATUS_SRV_PROXY_H

#include "iremote_proxy.h"
#include <nocopyable.h>

#include "drag_data.h"
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

    virtual void Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback) override;
    virtual void Unsubscribe(Type type, ActivityEvent event,
        sptr<IRemoteDevStaCallback> callback) override;
    virtual Data GetCache(const Type& type) override;

    virtual int32_t RegisterCoordinationListener() override;
    virtual int32_t UnregisterCoordinationListener() override;
    virtual int32_t EnableCoordination(int32_t userData, bool enabled) override;
    virtual int32_t StartCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcDeviceId) override;
    virtual int32_t StopCoordination(int32_t userData) override;
    virtual int32_t GetCoordinationState(int32_t userData, const std::string &deviceId) override;

    virtual int32_t StartDrag(const DragData &dragData) override;
    virtual int32_t StopDrag(int32_t result) override;
    virtual int32_t UpdateDragStyle(int32_t style) override;
    virtual int32_t UpdateDragMessage(const std::u16string &message) override;
    virtual int32_t GetDragTargetPid() override;
    int32_t AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &socketFd, int32_t &tokenType) override;

private:
    static inline BrokerDelegator<DeviceStatusSrvProxy> delegator_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_PROXY_H
