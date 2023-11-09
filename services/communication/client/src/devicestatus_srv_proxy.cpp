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

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "stationary_callback.h"
#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusSrvProxy" };
} // namespace

void DeviceStatusSrvProxy::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    CALL_DEBUG_ENTER;
    FI_HILOGI("Enter event:%{public}d, latency:%{public}d", event, latency);
    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Write descriptor failed");
        return;
    }

    WRITEINT32(data, type);
    WRITEINT32(data, event);
    WRITEINT32(data, latency);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_SUBSCRIBE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, errCode:%{public}d", ret);
        return;
    }
}

void DeviceStatusSrvProxy::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("Enter, event:%{public}d", event);
    StartTrace(HITRACE_TAG_MSDP, "clientUnSubscribeStart");
    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Write descriptor failed");
        return;
    }

    WRITEINT32(data, type);
    WRITEINT32(data, event);
    WRITEREMOTEOBJECT(data, callback->AsObject());

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_UNSUBSCRIBE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, errCode:%{public}d", ret);
        return;
    }
}

Data DeviceStatusSrvProxy::GetCache(const Type& type)
{
    CALL_DEBUG_ENTER;
    Data devicestatusData;
    devicestatusData.type = Type::TYPE_INVALID;
    devicestatusData.value = OnChangedValue::VALUE_INVALID;

    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL_WITH_RET((remote == nullptr), devicestatusData);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Write descriptor failed");
        return devicestatusData;
    }

    WRITEINT32(data, type, devicestatusData);

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_GETCACHE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, errCode:%{public}d", ret);
        return devicestatusData;
    }

    int32_t devicestatusType = -1;
    int32_t devicestatusValue = -1;
    READINT32(reply, devicestatusType, devicestatusData);
    READINT32(reply, devicestatusValue, devicestatusData);
    devicestatusData.type = static_cast<Type>(devicestatusType);
    devicestatusData.value = static_cast<OnChangedValue>(devicestatusValue);
    FI_HILOGD("type:%{public}d, value:%{public}d", devicestatusData.type, devicestatusData.value);
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
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_COORDINATION_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
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
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    MessageOption option;
    MessageParcel reply;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_COORDINATION_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::PrepareCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    MessageOption option;
    MessageParcel reply;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::PREPARE_COORDINATION),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::UnprepareCoordination(int32_t userData)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UNPREPARE_COORDINATION),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::ActivateCoordination(int32_t userData, const std::string &remoteNetworkId,
    int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITESTRING(data, remoteNetworkId, ERR_INVALID_VALUE);
    WRITEINT32(data, startDeviceId, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::START_COORDINATION),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::DeactivateCoordination(int32_t userData, bool isUnchained)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITEBOOL(data, isUnchained, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::STOP_COORDINATION),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UPDATED_DRAG_STYLE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("WriteInterfaceToken failed");
        return ERR_INVALID_VALUE;
    }
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_TARGET_PID),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    int32_t pid = 0;
    READINT32(reply, pid, IPC_PROXY_DEAD_OBJECT_ERR);
    return pid;
}

int32_t DeviceStatusSrvProxy::GetUdKey(std::string &udKey)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("WriteInterfaceToken failed");
        return ERR_INVALID_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_TARGET_UDKEY),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    READSTRING(reply, udKey, IPC_PROXY_DEAD_OBJECT_ERR);
    return RET_OK;
}

int32_t DeviceStatusSrvProxy::GetDragData(DragData &dragData)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_DATA), data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return ret;
    }
    READINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);
    if (ret != RET_OK) {
        FI_HILOGE("Get DragData failed");
        return ret;
    }

    auto pixelMap = OHOS::Media::PixelMap::Unmarshalling(reply);
    CHKPR(pixelMap, RET_ERR);
    dragData.shadowInfo.pixelMap = std::shared_ptr<OHOS::Media::PixelMap> (pixelMap);
    READINT32(reply, dragData.shadowInfo.x, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.shadowInfo.y, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READUINT8VECTOR(reply, dragData.buffer, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(reply, dragData.udKey, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.sourceType, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.dragNum, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.pointerId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.displayX, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.displayY, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(reply, dragData.displayId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READBOOL(reply, dragData.hasCanceledAnimation, E_DEVICESTATUS_READ_PARCEL_ERROR);
    return ret;
}

int32_t DeviceStatusSrvProxy::GetDragState(DragState &dragState)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_STATE), data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return ret;
    }

    int32_t dragStateTmp = 0;
    READINT32(reply, dragStateTmp, E_DEVICESTATUS_READ_PARCEL_ERROR);
    dragState = static_cast<DragState>(dragStateTmp);
    return ret;
}

int32_t DeviceStatusSrvProxy::GetCoordinationState(int32_t userData, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, userData, ERR_INVALID_VALUE);
    WRITESTRING(data, networkId, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_COORDINATION_STATE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
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
    WRITESTRING(data, dragData.udKey, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.filterInfo, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.extraInfo, ERR_INVALID_VALUE);
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::START_DRAG),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    READINT32(reply, ret, IPC_PROXY_DEAD_OBJECT_ERR);
    return ret;
}

int32_t DeviceStatusSrvProxy::StopDrag(const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    if (dropResult.result < DragResult::DRAG_SUCCESS || dropResult.result > DragResult::DRAG_EXCEPTION) {
        FI_HILOGE("Invalid result:%{public}d", static_cast<int32_t>(dropResult.result));
        return RET_ERR;
    }
    WRITEINT32(data, static_cast<int32_t>(dropResult.result), ERR_INVALID_VALUE);
    WRITEBOOL(data, dropResult.hasCustomAnimation, ERR_INVALID_VALUE);
    WRITEINT32(data, dropResult.windowId, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::STOP_DRAG),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    READINT32(reply, ret, IPC_PROXY_DEAD_OBJECT_ERR);
    return ret;
}

int32_t DeviceStatusSrvProxy::AllocSocketFd(const std::string &programName, int32_t moduleType,
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::ALLOC_SOCKET_FD),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    socketFd = reply.ReadFileDescriptor();
    if (socketFd < RET_OK) {
        FI_HILOGE("Read file descriptor failed, fd:%{public}d", socketFd);
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_DRAG_MONITOR),
        data, reply, option);
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_DRAG_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::AddSubscriptListener()
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_SUBSCRIPT_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::RemoveSubscriptListener()
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_SUBSCRIPT_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::SetDragWindowVisible(bool visible)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEBOOL(data, visible, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::SET_DRAG_WINDOW_VISIBLE),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height)
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::GET_SHADOW_OFFSET),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    READINT32(reply, offsetX, IPC_PROXY_DEAD_OBJECT_ERR);
    READINT32(reply, offsetY, IPC_PROXY_DEAD_OBJECT_ERR);
    READINT32(reply, width, IPC_PROXY_DEAD_OBJECT_ERR);
    READINT32(reply, height, IPC_PROXY_DEAD_OBJECT_ERR);
    FI_HILOGD("offsetX:%{public}d, offsetY:%{public}d, width:%{public}d, height:%{public}d",
        offsetX, offsetY, width, height);
    return ret;
}

int32_t DeviceStatusSrvProxy::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    CHKPR(shadowInfo.pixelMap, RET_ERR);
    if (!shadowInfo.pixelMap->Marshalling(data)) {
        FI_HILOGE("Failed to marshalling pixelMap");
        return ERR_INVALID_VALUE;
    }
    WRITEINT32(data, shadowInfo.x, ERR_INVALID_VALUE);
    WRITEINT32(data, shadowInfo.y, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::UPDATE_SHADOW_PIC),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::AddHotAreaListener()
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::ADD_HOT_AREA_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request fail, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::RemoveHotAreaListener()
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
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::REMOVE_HOT_AREA_MONITOR),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvProxy::EnterTextEditorArea(bool enable)
{
    CALL_DEBUG_ENTER;
    MessageParcel data;
    if (!data.WriteInterfaceToken(DeviceStatusSrvProxy::GetDescriptor())) {
        FI_HILOGE("Failed to write descriptor");
        return ERR_INVALID_VALUE;
    }
    WRITEBOOL(data, enable, ERR_INVALID_VALUE);
    MessageParcel reply;
    MessageOption option;
    sptr<IRemoteObject> remote = Remote();
    CHKPR(remote, RET_ERR);
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(DeviceInterfaceCode::ENTER_TEXT_EDITOR_AREA),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("Send request failed, ret:%{public}d", ret);
    }
    return ret;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
