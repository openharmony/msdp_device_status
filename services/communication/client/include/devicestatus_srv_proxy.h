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
#include "i_devicestatus.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusSrvProxy : public IRemoteProxy<Idevicestatus> {
public:
    explicit DeviceStatusSrvProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<Idevicestatus>(impl) {}
    DISALLOW_COPY_AND_MOVE(DeviceStatusSrvProxy);
    ~DeviceStatusSrvProxy() = default;

    virtual void Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback) override;
    virtual void Unsubscribe(Type type, ActivityEvent event,
        sptr<IRemoteDevStaCallback> callback) override;
    virtual Data GetCache(const Type& type) override;
    int32_t AllocSocketFd(const std::string &programName, int32_t moduleType,
        int32_t &socketFd, int32_t &tokenType) override;
    virtual int32_t RegisterCoordinationListener() override;
    virtual int32_t UnregisterCoordinationListener() override;
    virtual int32_t PrepareCoordination(int32_t userData) override;
    virtual int32_t UnprepareCoordination(int32_t userData) override;
    virtual int32_t ActivateCoordination(int32_t userData, const std::string &remoteNetworkId,
        int32_t startDeviceId) override;
    virtual int32_t DeactivateCoordination(int32_t userData, bool isUnchained) override;
    virtual int32_t GetCoordinationState(int32_t userData, const std::string &networkId) override;
    virtual int32_t StartDrag(const DragData &dragData) override;
    virtual int32_t StopDrag(const DragDropResult &dropResult) override;
    virtual int32_t UpdateDragStyle(DragCursorStyle style) override;
    virtual int32_t UpdateShadowPic(const ShadowInfo &shadowInfo) override;
    virtual int32_t GetDragData(DragData &dragData) override;
    virtual int32_t GetDragTargetPid() override;
    virtual int32_t GetUdKey(std::string &udKey) override;
    virtual int32_t AddDraglistener() override;
    virtual int32_t RemoveDraglistener() override;
    virtual int32_t SetDragWindowVisible(bool visible) override;
    virtual int32_t GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height) override;
    virtual int32_t AddHotAreaListener() override;

private:
    static inline BrokerDelegator<DeviceStatusSrvProxy> delegator_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_PROXY_H
