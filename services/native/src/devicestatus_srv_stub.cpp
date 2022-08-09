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

#include "devicestatus_srv_stub.h"

#include "message_parcel.h"
#include "devicestatus_srv_proxy.h"
#include "devicestatus_common.h"
#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_service.h"

namespace OHOS {
namespace Msdp {
int32_t DevicestatusSrvStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, \
    MessageOption &option)
{
    DEV_HILOGD(SERVICE, "cmd = %{public}d, flags = %{public}d", code, option.GetFlags());
    std::u16string descriptor = DevicestatusSrvStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        DEV_HILOGE(SERVICE, "DevicestatusSrvStub::OnRemoteRequest failed, descriptor is not matched");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int32_t>(Idevicestatus::DEVICESTATUS_SUBSCRIBE): {
            return SubscribeStub(data);
        }
        case static_cast<int32_t>(Idevicestatus::DEVICESTATUS_UNSUBSCRIBE): {
            return UnSubscribeStub(data);
        }
        case static_cast<int32_t>(Idevicestatus::DEVICESTATUS_GETCACHE): {
            return GetLatestDevicestatusDataStub(data, reply);
        }
        default: {
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
    return ERR_OK;
}

int32_t DevicestatusSrvStub::SubscribeStub(MessageParcel& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    DEVICESTATUS_READ_PARCEL_WITH_RET(data, Int32, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read type successfully");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    DEVICESTATUS_RETURN_IF_WITH_RET((obj == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read remote obj successfully");
    sptr<IdevicestatusCallback> callback = iface_cast<IdevicestatusCallback>(obj);
    DEVICESTATUS_RETURN_IF_WITH_RET((callback == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read callback successfully");
    Subscribe(DevicestatusDataUtils::DevicestatusType(type), callback);
    return ERR_OK;
}

int32_t DevicestatusSrvStub::UnSubscribeStub(MessageParcel& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    DEVICESTATUS_READ_PARCEL_WITH_RET(data, Int32, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    DEVICESTATUS_RETURN_IF_WITH_RET((obj == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    sptr<IdevicestatusCallback> callback = iface_cast<IdevicestatusCallback>(obj);
    DEVICESTATUS_RETURN_IF_WITH_RET((callback == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    UnSubscribe(DevicestatusDataUtils::DevicestatusType(type), callback);
    return ERR_OK;
}

int32_t DevicestatusSrvStub::GetLatestDevicestatusDataStub(MessageParcel& data, MessageParcel& reply)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    DEVICESTATUS_READ_PARCEL_WITH_RET(data, Int32, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DevicestatusDataUtils::DevicestatusData devicestatusData = GetCache(DevicestatusDataUtils::DevicestatusType(type));
    DEV_HILOGD(SERVICE, "devicestatusData.type: %{public}d", devicestatusData.type);
    DEV_HILOGD(SERVICE, "devicestatusData.value: %{public}d", devicestatusData.value);
    DEVICESTATUS_WRITE_PARCEL_WITH_RET(reply, Int32, devicestatusData.type, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    DEVICESTATUS_WRITE_PARCEL_WITH_RET(reply, Int32, devicestatusData.value, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Exit");
    return ERR_OK;
}
} // Msdp
} // OHOS
