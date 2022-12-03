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

#include "devicestatus_callback_stub.h"

#include <message_parcel.h>

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
int32_t DeviceStatusCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, \
    MessageOption &option)
{
    DEV_HILOGD(SERVICE, "cmd = %{public}u, flags= %{public}d", code, option.GetFlags());
    std::u16string descriptor = DeviceStatusCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        DEV_HILOGE(SERVICE, "DeviceStatusCallbackStub::OnRemoteRequest failed, descriptor mismatch");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int32_t>(IdevicestatusCallback::DEVICESTATUS_CHANGE): {
            return OnDeviceStatusChangedStub(data);
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ERR_OK;
}

int32_t DeviceStatusCallbackStub::OnDeviceStatusChangedStub(MessageParcel& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type;
    int32_t value;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, value, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DeviceStatusDataUtils::DeviceStatusData devicestatusData = {
        static_cast<DeviceStatusDataUtils::DeviceStatusType>(type),
        static_cast<DeviceStatusDataUtils::DeviceStatusValue>(value)
    };
    OnDeviceStatusChanged(devicestatusData);
    return ERR_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
