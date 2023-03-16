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

#include "devicestatus_srv_proxy.h"

#include "hitrace_meter.h"
#include "iremote_object.h"
#include <message_option.h>
#include <message_parcel.h>

#include "bytrace_adapter.h"
#include "devicestatus_common.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_define.h"
#include "idevicestatus_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusSrvProxy" };
} // namespace

void DeviceStatusSrvProxy::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGI(INNERKIT, "Enter event:%{public}d, latency:%{public}d", event, latency);
    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    WRITEINT32(data, type);
    WRITEINT32(data, event);
    WRITEINT32(data, latency);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_SUBSCRIBE), data, reply, option);
    if (ret != RET_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    DEV_HILOGD(INNERKIT, "Exit");
}

void DeviceStatusSrvProxy::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGD(INNERKIT, "Enter, event:%{public}d", event);
    StartTrace(HITRACE_TAG_MSDP, "clientUnSubscribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    WRITEINT32(data, type);
    WRITEINT32(data, event);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_UNSUBSCRIBE),
        data, reply, option);
    if (ret != RET_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return;
    }
    DEV_HILOGD(INNERKIT, "Exit");
}

Data DeviceStatusSrvProxy::GetCache(const Type& type)
{
    DEV_HILOGD(INNERKIT, "Enter");
    Data devicestatusData;
    devicestatusData.type = Type::TYPE_INVALID;
    devicestatusData.value = OnChangedValue::VALUE_INVALID;

    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL_WITH_RET((remote == nullptr), devicestatusData);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed!");
        return devicestatusData;
    }

    WRITEINT32(data, type, devicestatusData);

    int32_t ret = remote->SendRequest(static_cast<int32_t>(Idevicestatus::DEVICESTATUS_GETCACHE), data, reply, option);
    if (ret != RET_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
        return devicestatusData;
    }

    int32_t devicestatusType = -1;
    int32_t devicestatusValue = -1;
    READINT32(reply, devicestatusType, devicestatusData);
    READINT32(reply, devicestatusValue, devicestatusData);
    devicestatusData.type = Type(devicestatusType);
    devicestatusData.value = OnChangedValue(devicestatusValue);
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

int32_t DeviceStatusSrvProxy::EnableCoordination(int32_t userData, bool enabled)
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

int32_t DeviceStatusSrvProxy::StartCoordination(int32_t userData, const std::string &sinkDeviceId,
    int32_t srcDeviceId)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITESTRING(data, sinkDeviceId, ERR_INVALID_VALUE);
    WRITEINT32(data, srcDeviceId, ERR_INVALID_VALUE);
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

int32_t DeviceStatusSrvProxy::StopCoordination(int32_t userData)
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

int32_t DeviceStatusSrvProxy::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, static_cast<int32_t>(style), ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(UPDATED_DRAG_STYLE, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::GetDragTargetPid()
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
    int32_t ret = remote->SendRequest(GET_DRAG_TARGET_PID, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
        return RET_ERR;
    }
    int32_t pid;
    READINT32(reply, pid, IPC_PROXY_DEAD_OBJECT_ERR);
    return pid;
}

int32_t DeviceStatusSrvProxy::GetCoordinationState(int32_t userData, const std::string &deviceId)
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

int32_t DeviceStatusSrvProxy::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    CHKPR(dragData.shadowInfo.pixelMap, RET_ERR);
    if (!dragData.shadowInfo.pixelMap->Marshalling(data)) {
        FI_HILOGE("Failed to marshalling pixelMap");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, dragData.shadowInfo.x, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.shadowInfo.y, ERR_INVALID_VALUE);
    WRITEUINT8VECTOR(data, dragData.buffer, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.sourceType, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.dragNum, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.pointerId, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayX, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayY, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayId, ERR_INVALID_VALUE);
    WRITEBOOL(data, dragData.hasCanceledAnimation, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(START_DRAG, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    READINT32(reply, ret, IPC_PROXY_DEAD_OBJECT_ERR);
    return ret;
}

int32_t DeviceStatusSrvProxy::StopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    if (result < DragResult::DRAG_SUCCESS || result > DragResult::DRAG_CANCEL) {
        FI_HILOGE("Invalid result:%{public}d", static_cast<int32_t>(result));
        return RET_ERR;
    }
    WRITEINT32(data, static_cast<int32_t>(result), ERR_INVALID_VALUE);
    WRITEBOOL(data, hasCustomAnimation, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(STOP_DRAG, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    READINT32(reply, ret, IPC_PROXY_DEAD_OBJECT_ERR);
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
    FI_HILOGD("socketFd:%{public}d, tokenType:%{public}d", socketFd, tokenType);
    return RET_OK;
}

int32_t DeviceStatusSrvProxy::AddDraglistener()
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
    int32_t ret = remote->SendRequest(REGISTER_DRAG_MONITOR, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::RemoveDraglistener()
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
    int32_t ret = remote->SendRequest(UNREGISTER_DRAG_MONITOR, data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}
} // namespace DeviceStatus
} // Msdp
} // OHOS
