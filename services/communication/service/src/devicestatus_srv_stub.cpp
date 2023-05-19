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

#include "devicestatus_srv_stub.h"

#include "message_parcel.h"
#include "pixel_map.h"

#include "devicestatus_callback_proxy.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_service.h"
#include "devicestatus_srv_proxy.h"
#include "fi_log.h"
#include "stationary_callback.h"
#include "stationary_data.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusSrvStub" };
using ConnFunc = int32_t (DeviceStatusSrvStub::*)(MessageParcel& data, MessageParcel& reply);
} // namespace

int32_t DeviceStatusSrvStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DEV_HILOGD(SERVICE, "cmd = %{public}d, flags = %{public}d", code, option.GetFlags());
    std::u16string descriptor = DeviceStatusSrvStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        DEV_HILOGE(SERVICE, "DeviceStatusSrvStub::OnRemoteRequest failed, descriptor is not matched");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }
    const std::map<int32_t, ConnFunc> mapConnFunc = {
        {Idevicestatus::DEVICESTATUS_SUBSCRIBE, &DeviceStatusSrvStub::SubscribeStub},
        {Idevicestatus::DEVICESTATUS_UNSUBSCRIBE, &DeviceStatusSrvStub::UnsubscribeStub},
        {Idevicestatus::DEVICESTATUS_GETCACHE, &DeviceStatusSrvStub::GetLatestDeviceStatusDataStub},
        {Idevicestatus::REGISTER_COORDINATION_MONITOR, &DeviceStatusSrvStub::RegisterCoordinationMonitorStub},
        {Idevicestatus::UNREGISTER_COORDINATION_MONITOR, &DeviceStatusSrvStub::UnregisterCoordinationMonitorStub},
        {Idevicestatus::PREPARE_COORDINATION, &DeviceStatusSrvStub::PrepareCoordinationStub},
        {Idevicestatus::UNPREPARE_COORDINATION, &DeviceStatusSrvStub::UnPrepareCoordinationStub},
        {Idevicestatus::START_COORDINATION, &DeviceStatusSrvStub::ActivateCoordinationStub},
        {Idevicestatus::STOP_COORDINATION, &DeviceStatusSrvStub::DeactivateCoordinationStub},
        {Idevicestatus::GET_COORDINATION_STATE, &DeviceStatusSrvStub::GetCoordinationStateStub},
        {Idevicestatus::ALLOC_SOCKET_FD, &DeviceStatusSrvStub::HandleAllocSocketFdStub},
        {Idevicestatus::START_DRAG, &DeviceStatusSrvStub::StartDragStub},
        {Idevicestatus::STOP_DRAG, &DeviceStatusSrvStub::StopDragStub},
        {Idevicestatus::UPDATED_DRAG_STYLE, &DeviceStatusSrvStub::UpdateDragStyleStub},
        {Idevicestatus::GET_DRAG_TARGET_PID, &DeviceStatusSrvStub::GetDragTargetPidStub},
        {Idevicestatus::GET_DRAG_TARGET_UDKEY, &DeviceStatusSrvStub::GetUdKeyStub},
        {Idevicestatus::REGISTER_DRAG_MONITOR, &DeviceStatusSrvStub::AddDraglistenerStub},
        {Idevicestatus::UNREGISTER_DRAG_MONITOR, &DeviceStatusSrvStub::RemoveDraglistenerStub},
        {Idevicestatus::SET_DRAG_WINDOW_VISIBLE, &DeviceStatusSrvStub::SetDragWindowVisibleStub},
        {Idevicestatus::GET_SHADOW_OFFSET, &DeviceStatusSrvStub::GetShadowOffsetStub},
    };
    auto it = mapConnFunc.find(code);
    if (it != mapConnFunc.end()) {
        return (this->*it->second)(data, reply);
    }
    DEV_HILOGE(SERVICE, "Unknown code:%{public}u", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DeviceStatusSrvStub::SubscribeStub(MessageParcel& data, MessageParcel& reply)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read type successfully");
    int32_t event = -1;
    READINT32(data, event, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read event successfully");
    DEV_HILOGD(SERVICE, "event:%{public}d", event);
    int32_t latency = -1;
    READINT32(data, latency, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Read latency successfully");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    DEV_RET_IF_NULL_WITH_RET((obj == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGI(SERVICE, "Read remote obj successfully");
    sptr<IRemoteDevStaCallback> callback = iface_cast<IRemoteDevStaCallback>(obj);
    DEV_RET_IF_NULL_WITH_RET((callback == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGI(SERVICE, "Read callback successfully");
    Subscribe(Type(type), ActivityEvent(event), ReportLatencyNs(latency), callback);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::UnsubscribeStub(MessageParcel& data, MessageParcel& reply)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t event = -1;
    READINT32(data, event, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DEV_HILOGE(SERVICE, "UNevent:%{public}d", event);
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    DEV_RET_IF_NULL_WITH_RET((obj == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    sptr<IRemoteDevStaCallback> callback = iface_cast<IRemoteDevStaCallback>(obj);
    DEV_RET_IF_NULL_WITH_RET((callback == nullptr), E_DEVICESTATUS_READ_PARCEL_ERROR);
    Unsubscribe(Type(type), ActivityEvent(event), callback);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::GetLatestDeviceStatusDataStub(MessageParcel& data, MessageParcel& reply)
{
    DEV_HILOGD(SERVICE, "Enter");
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    Data devicestatusData = GetCache(Type(type));
    DEV_HILOGD(SERVICE, "devicestatusData.type:%{public}d", devicestatusData.type);
    DEV_HILOGD(SERVICE, "devicestatusData.value:%{public}d", devicestatusData.value);
    WRITEINT32(reply, devicestatusData.type, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    WRITEINT32(reply, devicestatusData.value, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

int32_t DeviceStatusSrvStub::RegisterCoordinationMonitorStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = RegisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnregisterCoordinationMonitorStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = UnregisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::PrepareCoordinationStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = PrepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call PrepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnPrepareCoordinationStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = UnprepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call UnprepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::ActivateCoordinationStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string remoteNetworkId;
    READSTRING(data, remoteNetworkId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t startDeviceId;
    READINT32(data, startDeviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = ActivateCoordination(userData, remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call ActivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::DeactivateCoordinationStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = DeactivateCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call DeactivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetCoordinationStateStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string deviceId;
    READSTRING(data, deviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = GetCoordinationState(userData, deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UpdateDragStyleStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t style;
    READINT32(data, style, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = UpdateDragStyle(static_cast<DragCursorStyle>(style));
    if (ret != RET_OK) {
        FI_HILOGE("Call UpdateDragStyle failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetDragTargetPidStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t pid = GetDragTargetPid();
    WRITEINT32(reply, pid, IPC_STUB_WRITE_PARCEL_ERR);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::GetUdKeyStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    std::string udKey;
    int32_t ret = GetUdKey(udKey);
    if (ret != RET_OK) {
        FI_HILOGE("Get udKey failed, ret:%{public}d", ret);
    }
    WRITESTRING(reply, udKey, IPC_STUB_WRITE_PARCEL_ERR);
    FI_HILOGD("Target udKey:%{public}s", udKey.c_str());
    return RET_OK;
}

int32_t DeviceStatusSrvStub::HandleAllocSocketFdStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t pid = GetCallingPid();
    if (!IsRunning()) {
        FI_HILOGE("Service is not running, pid:%{public}d, go switch default", pid);
        return SERVICE_NOT_RUNNING;
    }
    int32_t moduleId;
    READINT32(data, moduleId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string clientName;
    READSTRING(data, clientName, E_DEVICESTATUS_READ_PARCEL_ERROR);

    int32_t clientFd = -1;
    uint32_t tokenId = GetCallingTokenID();
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    int32_t ret = AllocSocketFd(clientName, moduleId, clientFd, tokenType);
    if (ret != RET_OK) {
        FI_HILOGE("AllocSocketFd failed, pid:%{public}d, go switch default", pid);
        if (clientFd >= 0) {
            close(clientFd);
        }
        return ret;
    }

    if (!reply.WriteFileDescriptor(clientFd)) {
        FI_HILOGE("Write file descriptor failed");
        close(clientFd);
        return IPC_STUB_WRITE_PARCEL_ERR;
    }

    WRITEINT32(reply, tokenType, IPC_STUB_WRITE_PARCEL_ERR);
    FI_HILOGD("Send clientFd to client, clientFd:%{public}d, tokenType:%{public}d", clientFd, tokenType);
    close(clientFd);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::StartDragStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    auto pixelMap = OHOS::Media::PixelMap::Unmarshalling(data);
    CHKPR(pixelMap, RET_ERR);
    DragData dragData;
    dragData.shadowInfo.pixelMap = std::shared_ptr<OHOS::Media::PixelMap> (pixelMap);
    if (dragData.shadowInfo.pixelMap->GetWidth() > MAX_PIXEL_MAP_WIDTH ||
        dragData.shadowInfo.pixelMap->GetHeight() > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Too big pixelMap, width:%{public}d, height:%{public}d",
            dragData.shadowInfo.pixelMap->GetWidth(), dragData.shadowInfo.pixelMap->GetHeight());
        return RET_ERR;
    }
    READINT32(data, dragData.shadowInfo.x, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.shadowInfo.y, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READUINT8VECTOR(data, dragData.buffer, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(data, dragData.udKey, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.sourceType, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.dragNum, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.pointerId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayX, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayY, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READBOOL(data, dragData.hasCanceledAnimation, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (dragData.dragNum <= 0 || dragData.buffer.size() > MAX_BUFFER_SIZE ||
        dragData.displayX < 0 || dragData.displayY < 0 || dragData.displayId < 0) {
        FI_HILOGE("Invalid parameter, dragNum:%{public}d, bufferSize:%{public}zu, "
            "displayX:%{public}d, displayY:%{public}d, displayId:%{public}d",
            dragData.dragNum, dragData.buffer.size(), dragData.displayX, dragData.displayY, dragData.displayId);
        return RET_ERR;
    }
    int32_t ret = StartDrag(dragData);
    if (ret != RET_OK) {
        FI_HILOGE("Call StartDrag failed, ret:%{public}d", ret);
    }
    WRITEINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);
    return ret;
}

int32_t DeviceStatusSrvStub::StopDragStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t result;
    bool hasCustomAnimation;
    READINT32(data, result, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (result < static_cast<int32_t>(DragResult::DRAG_SUCCESS) ||
        result > static_cast<int32_t>(DragResult::DRAG_CANCEL)) {
        FI_HILOGE("Invalid result:%{public}d", result);
        return RET_ERR;
    }
    READBOOL(data, hasCustomAnimation, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = StopDrag(static_cast<DragResult>(result), hasCustomAnimation);
    if (ret != RET_OK) {
        FI_HILOGE("Call StopDrag failed, ret:%{public}d", ret);
    }
    WRITEINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);
    return ret;
}

int32_t DeviceStatusSrvStub::AddDraglistenerStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = AddDraglistener();
    if (ret != RET_OK) {
        FI_HILOGE("Call AddDraglistener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::RemoveDraglistenerStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = RemoveDraglistener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RemoveDraglistener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::SetDragWindowVisibleStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    bool visible;
    READBOOL(data, visible, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = SetDragWindowVisible(visible);
    if (ret != RET_OK) {
        FI_HILOGE("Call SetDragWindowVisible failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetShadowOffsetStub(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t ret = GetShadowOffset(offsetX, offsetY, width, height);
    if (ret != RET_OK) {
        FI_HILOGE("Call GetShadowOffsetStub failed, ret:%{public}d", ret);
    }
    WRITEINT32(reply, offsetX, IPC_STUB_WRITE_PARCEL_ERR);
    WRITEINT32(reply, offsetY, IPC_STUB_WRITE_PARCEL_ERR);
    WRITEINT32(reply, width, IPC_STUB_WRITE_PARCEL_ERR);
    WRITEINT32(reply, height, IPC_STUB_WRITE_PARCEL_ERR);
    return ret;
}
} // namespace DeviceStatus
} // Msdp
} // OHOS