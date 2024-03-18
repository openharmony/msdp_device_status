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

#ifndef DEVICESTATUS_SRV_STUB_H
#define DEVICESTATUS_SRV_STUB_H

#include <map>

#include <iremote_stub.h>
#include <nocopyable.h>

#include "i_devicestatus.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusSrvStub : public IRemoteStub<Idevicestatus> {
public:
    using ConnFunc = int32_t (DeviceStatusSrvStub::*)(MessageParcel &data, MessageParcel &reply);

    DeviceStatusSrvStub();
    DISALLOW_COPY_AND_MOVE(DeviceStatusSrvStub);
    virtual ~DeviceStatusSrvStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t SubscribeStub(MessageParcel &data, MessageParcel &reply);
    int32_t UnsubscribeStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetLatestDeviceStatusDataStub(MessageParcel &data, MessageParcel &reply);
    int32_t HandleAllocSocketFdStub(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterCoordinationMonitorStub(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterCoordinationMonitorStub(MessageParcel &data, MessageParcel &reply);
    int32_t PrepareCoordinationStub(MessageParcel &data, MessageParcel &reply);
    int32_t UnPrepareCoordinationStub(MessageParcel &data, MessageParcel &reply);
    int32_t ActivateCoordinationStub(MessageParcel &data, MessageParcel &reply);
    int32_t DeactivateCoordinationStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetCoordinationStateStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetCoordinationStateSyncStub(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterCooperateMonitorStub(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterCooperateMonitorStub(MessageParcel &data, MessageParcel &reply);
    int32_t PrepareCooperateStub(MessageParcel &data, MessageParcel &reply);
    int32_t UnPrepareCooperateStub(MessageParcel &data, MessageParcel &reply);
    int32_t ActivateCooperateStub(MessageParcel &data, MessageParcel &reply);
    int32_t DeactivateCooperateStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetCooperateStateStub(MessageParcel &data, MessageParcel &reply);
    int32_t StartDragStub(MessageParcel &data, MessageParcel &reply);
    int32_t StopDragStub(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateDragStyleStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetDragTargetPidStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetUdKeyStub(MessageParcel &data, MessageParcel &reply);
    int32_t AddDraglistenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveDraglistenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t AddSubscriptListenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveSubscriptListenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t SetDragWindowVisibleStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetShadowOffsetStub(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateShadowPicStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetDragDataStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetDragStateStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetDragActionStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetExtraInfoStub(MessageParcel &data, MessageParcel &reply);
    int32_t AddHotAreaListenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveHotAreaListenerStub(MessageParcel &data, MessageParcel &reply);
    int32_t UpdatePreviewStyleStub(MessageParcel &data, MessageParcel &reply);
    int32_t UpdatePreviewStyleWithAnimationStub(MessageParcel &data, MessageParcel &reply);
    int32_t GetDragSummaryStub(MessageParcel &data, MessageParcel &reply);
    bool CheckCooperatePermission();
    bool IsSystemServiceCalling();
    bool IsSystemCalling();
    void InitCoordination();
    void InitDrag();
    int32_t EnterTextEditorAreaStub(MessageParcel &data, MessageParcel &reply);
    int32_t AddPrivilegeStub(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, ConnFunc> connFuncs_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_STUB_H
