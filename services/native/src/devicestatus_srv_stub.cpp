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
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DevicestatusSrvStub" };
} // namespace

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
        case REGISTER_COORDINATION_MONITOR: {
            return StubRegisterCoordinationMonitor(data, reply);
        }
        case UNREGISTER_COORDINATION_MONITOR: {
            return StubUnregisterCoordinationMonitor(data, reply);
        }
        case ENABLE_COORDINATION: {
            return StubEnableInputDeviceCoordination(data, reply);
        }
        case START_COORDINATION: {
            return StubStartInputDeviceCoordination(data, reply);
        }
        case STOP_COORDINATION: {
            return StubStopDeviceCoordination(data, reply);
        }
        case GET_COORDINATION_STATE: {
            return StubGetInputDeviceCoordinationState(data, reply);
        }
        case ALLOC_SOCKET_FD : {
            return StubHandleAllocSocketFd(data, reply);
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
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
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
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
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
    READINT32(data, type, E_DEVICESTATUS_READ_PARCEL_ERROR);
    DevicestatusDataUtils::DevicestatusData devicestatusData = GetCache(DevicestatusDataUtils::DevicestatusType(type));
    DEV_HILOGD(SERVICE, "devicestatusData.type: %{public}d", devicestatusData.type);
    DEV_HILOGD(SERVICE, "devicestatusData.value: %{public}d", devicestatusData.value);
    WRITEINT32(reply, devicestatusData.type, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    WRITEINT32(reply, devicestatusData.value, E_DEVICESTATUS_WRITE_PARCEL_ERROR);
    DEV_HILOGD(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusSrvStub::StubRegisterCoordinationMonitor(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = RegisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubUnregisterCoordinationMonitor(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t ret = UnregisterCoordinationListener();
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubEnableInputDeviceCoordination(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    bool enabled;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READBOOL(data, enabled, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = EnableInputDeviceCoordination(userData, enabled);
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubStartInputDeviceCoordination(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string sinkDeviceId;
    READSTRING(data, sinkDeviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t srcInputDeviceId;
    READINT32(data, srcInputDeviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = StartInputDeviceCoordination(userData, sinkDeviceId, srcInputDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call StartInputDeviceCoordination failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubStopDeviceCoordination(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = StopDeviceCoordination(userData);
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubGetInputDeviceCoordinationState(MessageParcel& data, MessageParcel& reply)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    READINT32(data, userData, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string deviceId;
    READSTRING(data, deviceId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    int32_t ret = GetInputDeviceCoordinationState(userData, deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Call RegisterCoordinationEvent failed ret:%{public}d", ret);
    }
    return ret;
}

int32_t DevicestatusSrvStub::StubHandleAllocSocketFd(MessageParcel& data, MessageParcel& reply)
{
    int32_t pid = GetCallingPid();
    if (!IsRunning()) {
        FI_HILOGE("Service is not running. pid:%{public}d, go switch default", pid);
        return MMISERVICE_NOT_RUNNING;
    }
    int32_t moduleId;
    READINT32(data, moduleId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    std::string clientName;
    READSTRING(data, clientName, E_DEVICESTATUS_READ_PARCEL_ERROR);

    int32_t clientFd = -1;
    int32_t tokenId = GetCallingTokenID();
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    int32_t ret = AllocSocketFd(clientName, moduleId, clientFd, tokenType);
    if (ret != RET_OK) {
        FI_HILOGE("AllocSocketFd failed pid:%{public}d, go switch default", pid);
        if (clientFd >= 0) {
            close(clientFd);
        }
        return ret;
    }

    if (!reply.WriteFileDescriptor(clientFd)) {
        FI_HILOGE("Write file descriptor failed");
        return IPC_STUB_WRITE_PARCEL_ERR;
    }

    WRITEINT32(reply, tokenType, IPC_STUB_WRITE_PARCEL_ERR);
    FI_HILOGI("Send clientFd to client, clientFd:%{public}d, tokenType:%{public}d", clientFd, tokenType);
    close(clientFd);
    return RET_OK;
}

} // namespace DeviceStatus
} // Msdp
} // OHOS
