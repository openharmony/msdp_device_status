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

#ifndef DEVICESTATUS_CALLBACK_STUB_H
#define DEVICESTATUS_CALLBACK_STUB_H

#include <iremote_stub.h>
#include <nocopyable.h>

#include "devicestatus_data_utils.h"
#include "idevicestatus_callback.h"
#include "message_option.h"
#include "message_parcel.h"
namespace OHOS {
namespace Msdp {
class DevicestatusCallbackStub : public IRemoteStub<IdevicestatusCallback> {
public:
    DISALLOW_COPY_AND_MOVE(DevicestatusCallbackStub);
    DevicestatusCallbackStub() = default;
    virtual ~DevicestatusCallbackStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
    void OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData& __attribute__((unused))value) override {}

private:
    int32_t OnDevicestatusChangedStub(MessageParcel& data);
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_CALLBACK_STUB_H
