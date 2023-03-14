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

#ifndef IDEVICESTATUS_H
#define IDEVICESTATUS_H

#include <iremote_broker.h>
#include "iremote_object.h"

#include "devicestatus_data_utils.h"
#include "drag_data.h"
#include "idevicestatus_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Idevicestatus : public IRemoteBroker {
public:
    enum {
        DEVICESTATUS_SUBSCRIBE = 0,
        DEVICESTATUS_UNSUBSCRIBE,
        DEVICESTATUS_GETCACHE,
        REGISTER_COORDINATION_MONITOR = 10,
        UNREGISTER_COORDINATION_MONITOR,
        ENABLE_COORDINATION,
        START_COORDINATION,
        STOP_COORDINATION,
        GET_COORDINATION_STATE,
        UPDATED_DRAG_STYLE = 20,
        UPDATED_DRAG_MESSAGE,
        START_DRAG,
        STOP_DRAG,
        GET_DRAG_TARGET_PID,
        REGISTER_DRAG_MONITOR,
        UNREGISTER_DRAG_MONITOR,
        ALLOC_SOCKET_FD = 40
    };

    virtual void Subscribe(Type type,
        ActivityEvent event,
        ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback) = 0;
    virtual void Unsubscribe(Type type,
        ActivityEvent event,
        sptr<IRemoteDevStaCallback> callback) = 0;
    virtual Data GetCache(const Type& type) = 0;

    virtual int32_t RegisterCoordinationListener() = 0;
    virtual int32_t UnregisterCoordinationListener() = 0;
    virtual int32_t EnableCoordination(int32_t userData, bool enabled) = 0;
    virtual int32_t StartCoordination(int32_t userData, const std::string &sinkDeviceId,
        int32_t srcDeviceId) = 0;
    virtual int32_t StopCoordination(int32_t userData) = 0;
    virtual int32_t GetCoordinationState(int32_t userData, const std::string &deviceId) = 0;
    virtual int32_t StartDrag(const DragData &dragData) = 0;
    virtual int32_t StopDrag(DragResult result, bool hasCustomAnimation) = 0;
    virtual int32_t UpdateDragStyle(int32_t style) = 0;
    virtual int32_t UpdateDragMessage(const std::u16string &message) = 0;
    virtual int32_t GetDragTargetPid() = 0;
    virtual int32_t AddDraglistener() = 0;
    virtual int32_t RemoveDraglistener() = 0;
    virtual int32_t AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &socketFd, int32_t &tokenType) = 0;
    virtual bool IsRunning() const
    {
        return true;
    }
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.msdp.Idevicestatus");
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // IDEVICESTATUS_H
