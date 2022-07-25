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

#include "devicestatus_srv_proxy.h"

#include <message_option.h>
#include <message_parcel.h>

#include "devicestatus_common.h"
#include "devicestatus_data_utils.h"
#include "hitrace_meter.h"
#include "idevicestatus_callback.h"
#include "iremote_object.h"

namespace OHOS {
namespace Msdp {
void DevicestatusSrvProxy::Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    StartTrace(HITRACE_TAG_MSDP, "clientSubcribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DevicestatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, type);
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_SUBSCRIBE), data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    FinishTrace(HITRACE_TAG_MSDP);
    DEV_HILOGD(INNERKIT, "Exit");
}

void DevicestatusSrvProxy::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    StartTrace(HITRACE_TAG_MSDP, "clientUnSubcribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DevicestatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed!");
        return;
    }

    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, type);
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_UNSUBSCRIBE),
        data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    FinishTrace(HITRACE_TAG_MSDP);
    DEV_HILOGD(INNERKIT, "Exit");
}

DevicestatusDataUtils::DevicestatusData DevicestatusSrvProxy::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGD(INNERKIT, "Enter");
    DevicestatusDataUtils::DevicestatusData devicestatusData;
    devicestatusData.type = DevicestatusDataUtils::DevicestatusType::TYPE_INVALID;
    devicestatusData.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;

    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF_WITH_RET((remote == nullptr), devicestatusData);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DevicestatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed!");
        return devicestatusData;
    }

    DEVICESTATUS_WRITE_PARCEL_WITH_RET(data, Int32, type, devicestatusData);

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_GETCACHE), data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return devicestatusData;
    }

    int32_t devicestatusType = -1;
    int32_t devicestatusValue = -1;
    DEVICESTATUS_READ_PARCEL_WITH_RET(reply, Int32, devicestatusType, devicestatusData);
    DEVICESTATUS_READ_PARCEL_WITH_RET(reply, Int32, devicestatusValue, devicestatusData);
    devicestatusData.type = DevicestatusDataUtils::DevicestatusType(devicestatusType);
    devicestatusData.value = DevicestatusDataUtils::DevicestatusValue(devicestatusValue);
    DEV_HILOGD(INNERKIT, "type: %{public}d, value: %{public}d", devicestatusData.type, devicestatusData.value);
    DEV_HILOGD(INNERKIT, "Exit");
    return devicestatusData;
}
} // Msdp
} // OHOS
