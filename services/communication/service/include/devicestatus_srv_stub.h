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

#include <iremote_stub.h>
#include <nocopyable.h>

#include "idevicestatus.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusSrvStub : public IRemoteStub<Idevicestatus> {
public:
    DeviceStatusSrvStub() = default;
    virtual ~DeviceStatusSrvStub() = default;
    DISALLOW_COPY_AND_MOVE(DeviceStatusSrvStub);

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
    int32_t HandleAllocSocketFdStub(MessageParcel& data, MessageParcel& reply);
private:
    int32_t SubscribeStub(MessageParcel& data);
    int32_t UnsubscribeStub(MessageParcel& data);
    int32_t GetLatestDeviceStatusDataStub(MessageParcel& data, MessageParcel& reply);

    int32_t RegisterCoordinationMonitorStub(MessageParcel& data, MessageParcel& reply);
    int32_t UnregisterCoordinationMonitorStub(MessageParcel& data, MessageParcel& reply);
    int32_t EnableCoordinationStub(MessageParcel& data, MessageParcel& reply);
    int32_t StartCoordinationStub(MessageParcel& data, MessageParcel& reply);
    int32_t StopCoordinationStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetCoordinationStateStub(MessageParcel& data, MessageParcel& reply);

    int32_t StartDragStub(MessageParcel& data, MessageParcel& reply);
    int32_t StopDragStub(MessageParcel& data, MessageParcel& reply);
    int32_t UpdateDragStyleStub(MessageParcel& data, MessageParcel& reply);
    int32_t UpdateDragMessageStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetDragTargetPidStub(MessageParcel& data, MessageParcel& reply);
    int32_t RegisterThumbnailDrawStub(MessageParcel& data, MessageParcel& reply);
    int32_t UnregisterThumbnailDrawStub(MessageParcel& data, MessageParcel& reply);
    int32_t AddDraglistenerStub(MessageParcel& data, MessageParcel& reply);
    int32_t RemoveDraglistenerStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_STUB_H
