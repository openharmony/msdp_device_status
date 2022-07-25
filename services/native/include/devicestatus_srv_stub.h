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

#ifndef DEVICESTATUS_SRV_STUB_H
#define DEVICESTATUS_SRV_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>

#include "idevicestatus.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace Msdp {
class DevicestatusSrvStub : public IRemoteStub<Idevicestatus> {
public:
    DevicestatusSrvStub() = default;
    virtual ~DevicestatusSrvStub() = default;
    DISALLOW_COPY_AND_MOVE(DevicestatusSrvStub);

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;
private:
    int32_t SubscribeStub(MessageParcel& data);
    int32_t UnSubscribeStub(MessageParcel& data);
    int32_t GetLatestDevicestatusDataStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_STUB_H
