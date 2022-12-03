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

#include "devicestatus_define.h"
#include "devicestatus_common.h"
#include "devicestatus_data_utils.h"
#include "hitrace_meter.h"
#include "idevicestatus_callback.h"
#include "iremote_object.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusSrvProxy" };
} // namespace

void DeviceStatusSrvProxy::Subscribe(const DeviceStatusDataUtils::DeviceStatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    StartTrace(HITRACE_TAG_MSDP, "clientSubscribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    WRITEINT32(data, type);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_SUBSCRIBE), data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    FinishTrace(HITRACE_TAG_MSDP);
    DEV_HILOGD(INNERKIT, "Exit");
}

void DeviceStatusSrvProxy::Unsubscribe(const DeviceStatusDataUtils::DeviceStatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    StartTrace(HITRACE_TAG_MSDP, "clientUnsubscribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed!");
        return;
    }

    WRITEINT32(data, type);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_UNSUBSCRIBE),
        data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    FinishTrace(HITRACE_TAG_MSDP);
    DEV_HILOGD(INNERKIT, "Exit");
}

DeviceStatusDataUtils::DeviceStatusData DeviceStatusSrvProxy::GetCache(const \
    DeviceStatusDataUtils::DeviceStatusType& type)
{
    DEV_HILOGD(INNERKIT, "Enter");
    DeviceStatusDataUtils::DeviceStatusData devicestatusData;
    devicestatusData.type = DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID;
    devicestatusData.value = DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID;

    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF_WITH_RET((remote == nullptr), devicestatusData);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed!");
        return devicestatusData;
    }

    WRITEINT32(data, type, devicestatusData);

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_GETCACHE), data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return devicestatusData;
    }

    int32_t devicestatusType = -1;
    int32_t devicestatusValue = -1;
    READINT32(reply, devicestatusType, devicestatusData);
    READINT32(reply, devicestatusValue, devicestatusData);
    devicestatusData.type = DeviceStatusDataUtils::DeviceStatusType(devicestatusType);
    devicestatusData.value = DeviceStatusDataUtils::DeviceStatusValue(devicestatusValue);
    DEV_HILOGD(INNERKIT, "type: %{public}d, value: %{public}d", devicestatusData.type, devicestatusData.value);
    DEV_HILOGD(INNERKIT, "Exit");
    return devicestatusData;
}

int32_t DeviceStatusSrvProxy::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(REGISTER_COORDINATION_MONITOR, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(UNREGISTER_COORDINATION_MONITOR, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::EnableInputDeviceCoordination(int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITEBOOL(data, enabled, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(ENABLE_COORDINATION, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::StartInputDeviceCoordination(int32_t userData, const std::string &sinkDeviceId,
    int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITESTRING(data, sinkDeviceId, ERR_INVALID_VALUE);
    WRITEINT32(data, srcInputDeviceId, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(START_COORDINATION, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::StopDeviceCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(STOP_COORDINATION, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITESTRING(data, deviceId, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(GET_COORDINATION_STATE, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &socketFd, int32_t &tokenType)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, moduleType, ERR_INVALID_VALUE);
    WRITESTRING(data, programName, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(ALLOC_SOCKET_FD, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    socketFd = reply.ReadFileDescriptor();
    if (socketFd < RET_OK) {
        FI_HILOGE("Read file descriptor failed, fd: %{public}d", socketFd);
        return IPC_PROXY_DEAD_OBJECT_ERR;
    }
    READINT32(reply, tokenType, IPC_PROXY_DEAD_OBJECT_ERR);
    FI_HILOGD("socketFd:%{public}d tokenType:%{public}d", socketFd, tokenType);
    return RET_OK;
}
} // namespace DeviceStatus
} // Msdp
} // OHOS
