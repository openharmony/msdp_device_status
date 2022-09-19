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

#include <ipc_types.h>
#include <message_parcel.h>
#include <message_option.h>

#include "idevicestatus_callback.h"
#include "devicestatus_common.h"
#include "bytrace_adapter.h"

namespace OHOS {
namespace Msdp {
void DevicestatusSrvProxy::Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
    const DevicestatusDataUtils::DevicestatusActivityEvent& event,
    const DevicestatusDataUtils::DevicestatusReportLatencyNs& latency,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(INNERKIT, "Enter event: %{public}d", event);
    DEV_HILOGI(INNERKIT, "Enter event: %{public}d", latency);
    BytraceAdapter::StartBytrace(BytraceAdapter::TRACE_START, BytraceAdapter::SUBSCRIBE, BytraceAdapter::CLIENT);
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
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, event);
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, latency);
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_SUBSCRIBE), data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    BytraceAdapter::StartBytrace(BytraceAdapter::TRACE_STOP, BytraceAdapter::SUBSCRIBE, BytraceAdapter::CLIENT);
    DEV_HILOGI(INNERKIT, "Exit");
}

void DevicestatusSrvProxy::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const DevicestatusDataUtils::DevicestatusActivityEvent& event,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(INNERKIT, "Enter");
    DEV_HILOGI(INNERKIT, "UNevent: %{public}d",event);
    BytraceAdapter::StartBytrace(BytraceAdapter::TRACE_START, BytraceAdapter::UNSUBSCRIBE, BytraceAdapter::CLIENT);
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
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, event);
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_UNSUBSCRIBE),
        data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    BytraceAdapter::StartBytrace(BytraceAdapter::TRACE_STOP, BytraceAdapter::UNSUBSCRIBE, BytraceAdapter::CLIENT);
    DEV_HILOGI(INNERKIT, "Exit");
}

DevicestatusDataUtils::DevicestatusData DevicestatusSrvProxy::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(INNERKIT, "Enter");
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
    DEV_HILOGI(INNERKIT, "type: %{public}d, value: %{public}d", devicestatusData.type, devicestatusData.value);
    DEV_HILOGI(INNERKIT, "Exit");
    return devicestatusData;
}
} // Msdp
} // OHOS
