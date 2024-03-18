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

#include <unistd.h>
#include <tokenid_kit.h>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "pixel_map.h"

#include "devicestatus_callback_proxy.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "drag_data_packer.h"
#include "preview_style_packer.h"
#include "proto.h"
#include "stationary_callback.h"
#include "stationary_data.h"
#include "include/util.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "DeviceStatusSrvStub"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DeviceStatusSrvStub::DeviceStatusSrvStub()
{
    InitCoordination();
    InitDrag();
}

void DeviceStatusSrvStub::InitCoordination()
{
    CALL_DEBUG_ENTER;
    connFuncs_ = {
        { static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_SUBSCRIBE),
            &DeviceStatusSrvStub::SubscribeStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_UNSUBSCRIBE),
            &DeviceStatusSrvStub::UnsubscribeStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::DEVICESTATUS_GETCACHE),
            &DeviceStatusSrvStub::GetLatestDeviceStatusDataStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_COORDINATION_MONITOR),
            &DeviceStatusSrvStub::RegisterCoordinationMonitorStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_COORDINATION_MONITOR),
            &DeviceStatusSrvStub::UnregisterCoordinationMonitorStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::PREPARE_COORDINATION),
            &DeviceStatusSrvStub::PrepareCoordinationStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNPREPARE_COORDINATION),
            &DeviceStatusSrvStub::UnPrepareCoordinationStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::START_COORDINATION),
            &DeviceStatusSrvStub::ActivateCoordinationStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::STOP_COORDINATION),
            &DeviceStatusSrvStub::DeactivateCoordinationStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_COORDINATION_STATE),
            &DeviceStatusSrvStub::GetCoordinationStateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_COORDINATION_STATE_SYNC),
            &DeviceStatusSrvStub::GetCoordinationStateSyncStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::ADD_HOT_AREA_MONITOR),
            &DeviceStatusSrvStub::AddHotAreaListenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::REMOVE_HOT_AREA_MONITOR),
            &DeviceStatusSrvStub::RemoveHotAreaListenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_SUBSCRIPT_MONITOR),
            &DeviceStatusSrvStub::AddSubscriptListenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_SUBSCRIPT_MONITOR),
            &DeviceStatusSrvStub::RemoveSubscriptListenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_COOPERATE_MONITOR),
            &DeviceStatusSrvStub::RegisterCooperateMonitorStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_COOPERATE_MONITOR),
            &DeviceStatusSrvStub::UnregisterCooperateMonitorStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::PREPARE_COOPERATE),
            &DeviceStatusSrvStub::PrepareCooperateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNPREPARE_COOPERATE),
            &DeviceStatusSrvStub::UnPrepareCooperateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::START_COOPERATE),
            &DeviceStatusSrvStub::ActivateCooperateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::STOP_COOPERATE),
            &DeviceStatusSrvStub::DeactivateCooperateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_COOPERATE_STATE),
            &DeviceStatusSrvStub::GetCooperateStateStub }
    };
}

void DeviceStatusSrvStub::InitDrag()
{
    CALL_DEBUG_ENTER;
    std::map<uint32_t, ConnFunc> dragFuncs = {
        { static_cast<uint32_t>(DeviceInterfaceCode::ALLOC_SOCKET_FD),
            &DeviceStatusSrvStub::HandleAllocSocketFdStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::START_DRAG),
            &DeviceStatusSrvStub::StartDragStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::STOP_DRAG),
            &DeviceStatusSrvStub::StopDragStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UPDATED_DRAG_STYLE),
            &DeviceStatusSrvStub::UpdateDragStyleStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_TARGET_PID),
            &DeviceStatusSrvStub::GetDragTargetPidStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_TARGET_UDKEY),
            &DeviceStatusSrvStub::GetUdKeyStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::REGISTER_DRAG_MONITOR),
            &DeviceStatusSrvStub::AddDraglistenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UNREGISTER_DRAG_MONITOR),
            &DeviceStatusSrvStub::RemoveDraglistenerStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::SET_DRAG_WINDOW_VISIBLE),
            &DeviceStatusSrvStub::SetDragWindowVisibleStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_SHADOW_OFFSET),
            &DeviceStatusSrvStub::GetShadowOffsetStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::UPDATE_SHADOW_PIC),
            &DeviceStatusSrvStub::UpdateShadowPicStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_DATA),
            &DeviceStatusSrvStub::GetDragDataStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_STATE),
            &DeviceStatusSrvStub::GetDragStateStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_SUMMARY),
            &DeviceStatusSrvStub::GetDragSummaryStub },
        { static_cast<uint32_t>(DeviceInterfaceCode::ENTER_TEXT_EDITOR_AREA),
            &DeviceStatusSrvStub::EnterTextEditorAreaStub },
        {static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_EXTRAINFO),
            &DeviceStatusSrvStub::GetExtraInfoStub },
        {static_cast<uint32_t>(DeviceInterfaceCode::GET_DRAG_ACTION),
            &DeviceStatusSrvStub::GetDragActionStub },
        {static_cast<uint32_t>(DeviceInterfaceCode::UPDATE_PREVIEW_STYLE),
            &DeviceStatusSrvStub::UpdatePreviewStyleStub },
        {static_cast<uint32_t>(DeviceInterfaceCode::UPDATE_PREVIEW_STYLE_WITH_ANIMATION),
            &DeviceStatusSrvStub::UpdatePreviewStyleWithAnimationStub },
        {static_cast<uint32_t>(DeviceInterfaceCode::ADD_PRIVILEGE),
            &DeviceStatusSrvStub::AddPrivilegeStub }
    };
    connFuncs_.insert(dragFuncs.begin(), dragFuncs.end());
}

bool DeviceStatusSrvStub::CheckCooperatePermission()
{
    CALL_DEBUG_ENTER;
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        COOPERATE_PERMISSION);
    return result == Security::AccessToken::PERMISSION_GRANTED;
}

bool DeviceStatusSrvStub::IsSystemServiceCalling()
{
    const auto tokenId = IPCSkeleton::GetCallingTokenID();
    const auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (flag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE ||
        flag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL) {
        FI_HILOGD("system service calling, tokenId:%{public}u, flag:%{public}u", tokenId, flag);
        return true;
    }
    return false;
}

bool DeviceStatusSrvStub::IsSystemCalling()
{
    if (IsSystemServiceCalling()) {
        return true;
    }
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetCallingFullTokenID());
}

int32_t DeviceStatusSrvStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    FI_HILOGD("cmd:%{public}d, flags:%{public}d", code, option.GetFlags());
    std::u16string descriptor = DeviceStatusSrvStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        FI_HILOGE("DeviceStatusSrvStub::OnRemoteRequest failed, descriptor is not matched");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }
    auto it = connFuncs_.find(code);
    if (it != connFuncs_.end()) {
        return (this->*it->second)(data, reply);
    }
    FI_HILOGE("Unknown code:%{public}u", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DeviceStatusSrvStub::SubscribeStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGD("Read type successfully");
    int32_t event = -1;
    READINT32(data, event, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGD("Read event successfully");
    FI_HILOGD("event:%{public}d", event);
    int32_t latency = -1;
    READINT32(data, latency, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGD("Read latency successfully");
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    CHKPR(obj, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGI("Read remote obj successfully");
    sptr<IRemoteDevStaCallback> callback = iface_cast<IRemoteDevStaCallback>(obj);
    CHKPR(callback, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGI("Read callback successfully");
    Subscribe(static_cast<Type>(type), static_cast<ActivityEvent>(event),
        static_cast<ReportLatencyNs>(latency), callback);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::UnsubscribeStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t event = -1;
    READINT32(data, event, E_DEVICESTATUS_READ_PARCEL_ERROR);
    FI_HILOGE("event:%{public}d", event);
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    CHKPR(obj, E_DEVICESTATUS_READ_PARCEL_ERROR);
    sptr<IRemoteDevStaCallback> callback = iface_cast<IRemoteDevStaCallback>(obj);
    CHKPR(callback, E_DEVICESTATUS_READ_PARCEL_ERROR);
    Unsubscribe(static_cast<Type>(type), static_cast<ActivityEvent>(event), callback);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::GetLatestDeviceStatusDataStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t type = -1;
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    Data devicestatusData = GetCache(static_cast<Type>(type));
    FI_HILOGD("devicestatusData.type:%{public}d", devicestatusData.type);
    FI_HILOGD("devicestatusData.value:%{public}d", devicestatusData.value);
    WRITEINT32(reply, devicestatusData.type, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    WRITEINT32(reply, devicestatusData.value, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::RegisterCoordinationMonitorStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = RegisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call registerCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnregisterCoordinationMonitorStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = UnregisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call unregisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::PrepareCoordinationStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = PrepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call prepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnPrepareCoordinationStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = UnprepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call unprepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::ActivateCoordinationStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string remoteNetworkId;
    READSTRING(data, remoteNetworkId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t startDeviceId = 0;
    READINT32(data, startDeviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = ActivateCoordination(userData, remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call ActivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::DeactivateCoordinationStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    bool isUnchained = false;
    READBOOL(data, isUnchained, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = DeactivateCoordination(userData, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Call DeactivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetCoordinationStateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string networkId;
    READSTRING(data, networkId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = GetCoordinationState(userData, networkId);
    if (ret != RET_OK) {
        FI_HILOGE("GetCoordinationStateStub failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetCoordinationStateSyncStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    std::string udId;
    READSTRING(data, udId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    bool state { false };
    int32_t ret = GetCoordinationState(udId, state);
    if (ret != RET_OK) {
        FI_HILOGE("GetCoordinationState failed, ret:%{public}d", ret);
    }
    WRITEBOOL(reply, state, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    return ret;
}

int32_t DeviceStatusSrvStub::RegisterCooperateMonitorStub(MessageParcel &data,
    MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t ret = RegisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call registerCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnregisterCooperateMonitorStub(MessageParcel &data,
    MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t ret = UnregisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call unregisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::PrepareCooperateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = PrepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call prepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UnPrepareCooperateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = UnprepareCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call unprepareCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::ActivateCooperateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string remoteNetworkId;
    READSTRING(data, remoteNetworkId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t startDeviceId = 0;
    READINT32(data, startDeviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = ActivateCoordination(userData, remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call ActivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::DeactivateCooperateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    bool isUnchained = false;
    READBOOL(data, isUnchained, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = DeactivateCoordination(userData, isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Call DeactivateCoordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetCooperateStateStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!IsSystemCalling()) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t userData = 0;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string networkId;
    READSTRING(data, networkId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = GetCoordinationState(userData, networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UpdateDragStyleStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t style = 0;
    READINT32(data, style, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = UpdateDragStyle(static_cast<DragCursorStyle>(style));
    if (ret != RET_OK) {
        FI_HILOGE("Call UpdateDragStyle failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetDragTargetPidStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = GetDragTargetPid();
    WRITEINT32(reply, pid, IPC_STUB_WRITE_PARCEL_ERR);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::GetUdKeyStub(MessageParcel &data, MessageParcel &reply)
{
    std::string udKey;
    int32_t ret = GetUdKey(udKey);
    if (ret != RET_OK) {
        FI_HILOGE("Get udKey failed, ret:%{public}d", ret);
    }
    WRITESTRING(reply, udKey, IPC_STUB_WRITE_PARCEL_ERR);
    FI_HILOGD("Target udKey:%{public}s", GetAnonyString(udKey).c_str());
    return RET_OK;
}

int32_t DeviceStatusSrvStub::HandleAllocSocketFdStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t pid = GetCallingPid();
    if (!IsRunning()) {
        FI_HILOGE("Service is not running, pid:%{public}d, go switch default", pid);
        return SERVICE_NOT_RUNNING;
    }
    int32_t moduleId = 0;
    READINT32(data, moduleId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string clientName;
    READSTRING(data, clientName, E_DEVICESTATUS_READ_PARCEL_ERROR);

    int32_t clientFd = -1;
    uint32_t tokenId = GetCallingTokenID();
    int32_t tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    int32_t ret = AllocSocketFd(clientName, moduleId, clientFd, tokenType);
    if (ret != RET_OK) {
        FI_HILOGE("AllocSocketFd failed, pid:%{public}d, go switch default", pid);
        if (clientFd >= 0) {
            if (close(clientFd) < 0) {
                FI_HILOGE("Close client fd failed, error:%{public}s, clientFd:%{public}d", strerror(errno), clientFd);
            }
        }
        return ret;
    }

    if (!reply.WriteFileDescriptor(clientFd)) {
        FI_HILOGE("Write file descriptor failed");
        if (close(clientFd) < 0) {
            FI_HILOGE("Close client fd failed, error:%{public}s, clientFd:%{public}d", strerror(errno), clientFd);
        }
        return IPC_STUB_WRITE_PARCEL_ERR;
    }

    WRITEINT32(reply, tokenType, IPC_STUB_WRITE_PARCEL_ERR);
    FI_HILOGD("Send clientFd to client, clientFd:%{public}d, tokenType:%{public}d", clientFd, tokenType);
    if (close(clientFd) < 0) {
        FI_HILOGE("Close client fd failed, error:%{public}s, clientFd:%{public}d", strerror(errno), clientFd);
    }
    return RET_OK;
}

int32_t DeviceStatusSrvStub::StartDragStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    DragData dragData;
    if (DragDataPacker::UnMarshalling(data, dragData) != RET_OK) {
        FI_HILOGE("UnMarshalling dragData failed");
        return E_DEVICESTATUS_READ_PARCEL_ERROR;
    }
    if (DragDataPacker::CheckDragData(dragData) != RET_OK) {
        FI_HILOGE("CheckDragData failed");
        return RET_ERR;
    }
    int32_t ret = StartDrag(dragData);
    if (ret != RET_OK) {
        FI_HILOGE("Call StartDrag failed, ret:%{public}d", ret);
    }
    WRITEINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);
    return ret;
}

int32_t DeviceStatusSrvStub::StopDragStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = 0;
    READINT32(data, result, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if ((result < static_cast<int32_t>(DragResult::DRAG_SUCCESS)) ||
        (result > static_cast<int32_t>(DragResult::DRAG_EXCEPTION))) {
        FI_HILOGE("Invalid result:%{public}d", result);
        return RET_ERR;
    }
    bool hasCustomAnimation = false;
    READBOOL(data, hasCustomAnimation, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t mainWindow = -1;
    READINT32(data, mainWindow, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t dragBehavior = -1;
    READINT32(data, dragBehavior, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if ((dragBehavior < static_cast<int32_t>(DragBehavior::UNKNOWN)) ||
        (dragBehavior > static_cast<int32_t>(DragBehavior::MOVE))) {
        FI_HILOGE("Invalid dragBehavior:%{public}d", dragBehavior);
        return RET_ERR;
    }
    DragDropResult dropResult;
    dropResult.result = static_cast<DragResult>(result);
    dropResult.hasCustomAnimation = hasCustomAnimation;
    dropResult.mainWindow = mainWindow;
    dropResult.dragBehavior = static_cast<DragBehavior>(dragBehavior);
    int32_t ret = StopDrag(dropResult);
    if (ret != RET_OK) {
        FI_HILOGE("Call StopDrag failed, ret:%{public}d", ret);
    }
    WRITEINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);
    return ret;
}

int32_t DeviceStatusSrvStub::AddDraglistenerStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = AddDraglistener();
    if (ret != RET_OK) {
        FI_HILOGE("Call AddDraglistener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::RemoveDraglistenerStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = RemoveDraglistener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RemoveDraglistener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::AddSubscriptListenerStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = AddSubscriptListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call AddSubscriptListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::RemoveSubscriptListenerStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = RemoveSubscriptListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RemoveSubscriptListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::SetDragWindowVisibleStub(MessageParcel &data, MessageParcel &reply)
{
    bool visible = false;
    READBOOL(data, visible, E_DEVICESTATUS_READ_PARCEL_ERROR);
    bool isForce = false;
    READBOOL(data, isForce, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = SetDragWindowVisible(visible, isForce);
    if (ret != RET_OK) {
        FI_HILOGE("Call SetDragWindowVisible failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetShadowOffsetStub(MessageParcel &data, MessageParcel &reply)
{
    ShadowOffset shadowOffset;
    if (GetShadowOffset(shadowOffset) != RET_OK) {
        FI_HILOGE("GetShadowOffset failed");
        return RET_ERR;
    }
    if (ShadowOffsetPacker::Marshalling(shadowOffset, reply) != RET_OK) {
        FI_HILOGE("Marshalling shadowOffset failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceStatusSrvStub::UpdateShadowPicStub(MessageParcel &data, MessageParcel &reply)
{
    ShadowInfo shadowInfo;
    if (ShadowPacker::UnPackShadowInfo(data, shadowInfo) != RET_OK) {
        FI_HILOGE("UnPackShadowInfo failed");
        return RET_ERR;
    }
    if (ShadowPacker::CheckShadowInfo(shadowInfo) != RET_OK) {
        FI_HILOGE("CheckShadowInfo failed");
        return RET_ERR;
    }
    int32_t ret = UpdateShadowPic(shadowInfo);
    if (ret != RET_OK) {
        FI_HILOGE("Call Update shadow picture failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetDragDataStub(MessageParcel &data, MessageParcel &reply)
{
    DragData dragData;
    int32_t ret = GetDragData(dragData);
    WRITEINT32(reply, ret, IPC_STUB_WRITE_PARCEL_ERR);

    if (ret != RET_OK) {
        FI_HILOGE("Get DragData failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    if (DragDataPacker::Marshalling(dragData, reply) != RET_OK) {
        FI_HILOGE("Marshalling dragData failed");
        return RET_ERR;
    }
    return ret;
}

int32_t DeviceStatusSrvStub::AddHotAreaListenerStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t ret = AddHotAreaListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call hot area listener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetDragStateStub(MessageParcel &data, MessageParcel &reply)
{
    DragState dragState;
    int32_t ret = GetDragState(dragState);
    if (ret != RET_OK) {
        FI_HILOGE("Get DragState failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    int32_t dragStateTmp = static_cast<int32_t>(dragState);
    WRITEINT32(reply, dragStateTmp, ERR_INVALID_VALUE);
    return ret;
}

int32_t DeviceStatusSrvStub::RemoveHotAreaListenerStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    if (!CheckCooperatePermission()) {
        FI_HILOGE("The caller has no COOPERATE_MANAGER permission");
        return COMMON_PERMISSION_CHECK_ERROR;
    }
    int32_t ret = RemoveHotAreaListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call remove hot area listener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UpdatePreviewStyleStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    PreviewStyle previewStyle;
    if (PreviewStylePacker::UnMarshalling(data, previewStyle) != RET_OK) {
        FI_HILOGE("UnMarshalling previewStyle failed");
        return RET_ERR;
    }
    int32_t ret = UpdatePreviewStyle(previewStyle);
    if (ret != RET_OK) {
        FI_HILOGE("UpdatePreviewStyle failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::UpdatePreviewStyleWithAnimationStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    PreviewStyle previewStyle;
    if (PreviewStylePacker::UnMarshalling(data, previewStyle) != RET_OK) {
        FI_HILOGE("UnMarshalling previewStyle failed");
        return RET_ERR;
    }
    PreviewAnimation animation;
    if (PreviewAnimationPacker::UnMarshalling(data, animation) != RET_OK) {
        FI_HILOGE("UnMarshalling animation failed");
        return RET_ERR;
    }
    int32_t ret = UpdatePreviewStyleWithAnimation(previewStyle, animation);
    if (ret != RET_OK) {
        FI_HILOGE("UpdatePreviewStyleWithAnimation failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetDragSummaryStub(MessageParcel &data, MessageParcel &reply)
{
    std::map<std::string, int64_t> summarys;
    if (GetDragSummary(summarys) != RET_OK) {
        FI_HILOGE("Get summarys failed");
        return RET_ERR;
    }
    if (SummaryPacker::Marshalling(summarys, reply) != RET_OK) {
        FI_HILOGE("Failed to summarys unmarshalling");
        return ERR_INVALID_VALUE;
    }
    return RET_OK;
}

int32_t DeviceStatusSrvStub::GetDragActionStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    DragAction dragAction = DragAction::INVALID;
    int32_t ret = GetDragAction(dragAction);
    if (ret != RET_OK) {
        return RET_ERR;
    }
    WRITEINT32(reply, static_cast<int32_t>(dragAction), IPC_STUB_WRITE_PARCEL_ERR);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::EnterTextEditorAreaStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    bool enable = false;
    READBOOL(data, enable, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = EnterTextEditorArea(enable);
    if (ret != RET_OK) {
        FI_HILOGE("Call EnterTextEditorArea failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusSrvStub::GetExtraInfoStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    std::string extraInfo;
    int32_t ret = GetExtraInfo(extraInfo);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to get extraInfo in dragData");
        return ret;
    }
    WRITESTRING(reply, extraInfo, IPC_STUB_WRITE_PARCEL_ERR);
    return RET_OK;
}

int32_t DeviceStatusSrvStub::AddPrivilegeStub(MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = AddPrivilege();
    if (ret != RET_OK) {
        FI_HILOGE("Failed to get extraInfo in dragData");
        return ret;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS