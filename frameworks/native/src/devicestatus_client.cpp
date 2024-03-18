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

#include "devicestatus_client.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "include/util.h"

#include "coordination_manager_impl.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "drag_manager_impl.h"
#ifdef OHOS_BUILD_ENABLE_RUST_IMPL
#include "fusion_data_binding_internal.h"
#include "fusion_frameworks_binding.h"
#endif // OHOS_BUILD_ENABLE_RUST_IMPL

#undef LOG_TAG
#define LOG_TAG "DeviceStatusClient"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DeviceStatusClient::DeviceStatusClient() {}
DeviceStatusClient::~DeviceStatusClient()
{
    if (devicestatusProxy_ != nullptr) {
        auto remoteObject = devicestatusProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

ErrCode DeviceStatusClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (devicestatusProxy_ != nullptr) {
        return RET_OK;
    }

    sptr<ISystemAbilityManager> sa = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHKPR(sa, E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED);

    sptr<IRemoteObject> remoteObject = sa->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    CHKPR(remoteObject, E_DEVICESTATUS_GET_SERVICE_FAILED);

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) DeviceStatusDeathRecipient());
    CHKPR(deathRecipient_, ERR_NO_MEMORY);

    if (remoteObject->IsProxyObject()) {
        if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
            FI_HILOGE("Add death recipient to DeviceStatus service failed");
            return E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED;
        }
    }

    devicestatusProxy_ = iface_cast<Idevicestatus>(remoteObject);
    FI_HILOGD("Connecting DeviceStatusService success");
    return RET_OK;
}

void DeviceStatusClient::ResetProxy(const wptr<IRemoteObject> &remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHKPV(devicestatusProxy_);
    auto serviceRemote = devicestatusProxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        devicestatusProxy_ = nullptr;
    }
    if (deathListener_ != nullptr) {
        FI_HILOGI("Notify service death to listener");
        deathListener_();
    }
}

void DeviceStatusClient::DeviceStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    CHKPV(remote);
    DeviceStatusClient::GetInstance().ResetProxy(remote);
    FI_HILOGD("Recv death notice");
}

int32_t DeviceStatusClient::SubscribeCallback(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    FI_HILOGI("Enter event:%{public}d, latency:%{public}d", event, latency);
    auto [_, ret] = typeMap_.insert(std::make_pair(type, 1));
    if (!ret) {
        FI_HILOGW("Insert pair to typeMap_ failed");
    }
    FI_HILOGD("typeMap_:%{public}d, type:%{public}d", typeMap_[type], type);
    CHKPR(callback, RET_ERR);

    if (Connect() != RET_OK) {
        FI_HILOGE("Connect failed");
        return RET_ERR;
    }
    CHKPR(devicestatusProxy_, RET_ERR);

    if (type > Type::TYPE_INVALID && type <= Type::TYPE_LID_OPEN) {
        devicestatusProxy_->Subscribe(type, event, latency, callback);
    }
    return RET_OK;
}

int32_t DeviceStatusClient::UnsubscribeCallback(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    CALL_DEBUG_ENTER;
    FI_HILOGI("event:%{public}d", event);
    typeMap_.erase(type);
    FI_HILOGD("typeMap_ %{public}d", typeMap_[type]);
    CHKPR(callback, RET_ERR);

    if (Connect() != RET_OK) {
        FI_HILOGE("Connect failed");
        return RET_ERR;
    }
    CHKPR(devicestatusProxy_, RET_ERR);

    if ((type < TYPE_INVALID) || (type > TYPE_MAX)) {
        FI_HILOGE("type out of range");
        return RET_ERR;
    }
    if (event < ActivityEvent::EVENT_INVALID || event > ActivityEvent::ENTER_EXIT) {
        FI_HILOGE("event out of range");
        return RET_ERR;
    }
    devicestatusProxy_->Unsubscribe(type, event, callback);
    return RET_OK;
}

Data DeviceStatusClient::GetDeviceStatusData(Type type)
{
    CALL_DEBUG_ENTER;
    Data devicestatusData;
    devicestatusData.type = type;
    devicestatusData.value = OnChangedValue::VALUE_INVALID;
    if (Connect() != RET_OK) {
        FI_HILOGE("Connect failed");
        return devicestatusData;
    }
    if (devicestatusProxy_ == nullptr) {
        FI_HILOGE("devicestatusProxy_ is nullptr");
        return devicestatusData;
    }
    if (type > Type::TYPE_INVALID
        && type <= Type::TYPE_LID_OPEN) {
        devicestatusData = devicestatusProxy_->GetCache(type);
    }
    return devicestatusData;
}

#ifdef OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    return fusion_register_coordination_listener();
}

int32_t DeviceStatusClient::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    return fusion_unregister_coordination_listener();
}

int32_t DeviceStatusClient::PrepareCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    return fusion_enable_coordination(userData);
}

int32_t DeviceStatusClient::UnprepareCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    return fusion_disable_coordination(userData);
}

int32_t DeviceStatusClient::ActivateCoordination(int32_t userData,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    return fusion_start_coordination(userData, remoteNetworkId.c_str(), startDeviceId);
}

int32_t DeviceStatusClient::DeactivateCoordination(int32_t userData, bool isUnchained)
{
    CALL_DEBUG_ENTER;
    return fusion_stop_coordination(userData, isUnchained);
}

int32_t DeviceStatusClient::GetCoordinationState(int32_t userData, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    return fusion_get_coordination_state(userData, networkId.c_str());
}

#else // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::RegisterCoordinationListener(bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->RegisterCoordinationListener(isCompatible);
}

int32_t DeviceStatusClient::UnregisterCoordinationListener(bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UnregisterCoordinationListener(isCompatible);
}

int32_t DeviceStatusClient::PrepareCoordination(int32_t userData, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->PrepareCoordination(userData, isCompatible);
}

int32_t DeviceStatusClient::UnprepareCoordination(int32_t userData, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UnprepareCoordination(userData, isCompatible);
}

int32_t DeviceStatusClient::ActivateCoordination(int32_t userData,
    const std::string &remoteNetworkId, int32_t startDeviceId, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->ActivateCoordination(userData, remoteNetworkId, startDeviceId, isCompatible);
}

int32_t DeviceStatusClient::DeactivateCoordination(int32_t userData, bool isUnchained, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->DeactivateCoordination(userData, isUnchained, isCompatible);
}

int32_t DeviceStatusClient::GetCoordinationState(int32_t userData, const std::string &networkId, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetCoordinationState(userData, networkId, isCompatible);
}

int32_t DeviceStatusClient::GetCoordinationState(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetCoordinationState(udId, state);
}

#endif // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdateDragStyle(style);
}

int32_t DeviceStatusClient::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragTargetPid();
}

int32_t DeviceStatusClient::GetUdKey(std::string &udKey)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetUdKey(udKey);
}

#ifdef OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::AllocSocketPair(int32_t moduleType)
{
    const std::string programName(GetProgramName());
    int32_t ret = fusion_alloc_socket_fd(programName.c_str(), moduleType, &socketFd_, &tokenType_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to connect to server via socket:%{public}d", ret);
        return RET_ERR;
    }
    FI_HILOGI("Connected successfully to server via socket, "
        "socketFd_:%{public}d tokenType_:%{public}d",
        socketFd_, tokenType_);
    return RET_OK;
}

#else // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::AllocSocketPair(int32_t moduleType)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPR(devicestatusProxy_, RET_ERR);
    const std::string programName(GetProgramName());
    int32_t result = devicestatusProxy_->AllocSocketFd(programName, moduleType, socketFd_, tokenType_);
    if (result != RET_OK) {
        FI_HILOGE("AllocSocketFd has error:%{public}d", result);
        return RET_ERR;
    }

    FI_HILOGI("AllocSocketPair success, socketFd_:%{public}d, tokenType_:%{public}d", socketFd_, tokenType_);
    return RET_OK;
}

#endif // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::GetClientSocketFdOfAllocedSocketPair() const
{
    CALL_DEBUG_ENTER;
    return socketFd_;
}

void DeviceStatusClient::RegisterDeathListener(std::function<void()> deathListener)
{
    deathListener_ = deathListener;
}

#ifdef OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    CDragData cDragData;
    if (CDragDataFrom(&dragData, &cDragData) != RET_OK) {
        FI_HILOGE("Conversion of DragData to CDragData failed");
        return RET_ERR;
    }
    int32_t ret = fusion_start_drag(&cDragData);
    CDragDataFree(&cDragData);
    return ret;
}

#else // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    CHKPR(devicestatusProxy_, RET_ERR);
    return devicestatusProxy_->StartDrag(dragData);
}

#endif // OHOS_BUILD_ENABLE_RUST_IMPL

int32_t DeviceStatusClient::StopDrag(const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    CHKPR(devicestatusProxy_, RET_ERR);
    return devicestatusProxy_->StopDrag(dropResult);
}

int32_t DeviceStatusClient::AddDraglistener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->AddDraglistener();
}

int32_t DeviceStatusClient::RemoveDraglistener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->RemoveDraglistener();
}

int32_t DeviceStatusClient::AddSubscriptListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->AddSubscriptListener();
}

int32_t DeviceStatusClient::RemoveSubscriptListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->RemoveSubscriptListener();
}

int32_t DeviceStatusClient::SetDragWindowVisible(bool visible, bool isForce)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->SetDragWindowVisible(visible, isForce);
}

int32_t DeviceStatusClient::GetShadowOffset(ShadowOffset &shadowOffset)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetShadowOffset(shadowOffset);
}

int32_t DeviceStatusClient::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdateShadowPic(shadowInfo);
}

int32_t DeviceStatusClient::GetDragData(DragData &dragData)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragData(dragData);
}

int32_t DeviceStatusClient::GetDragState(DragState &dragState)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragState(dragState);
}

int32_t DeviceStatusClient::GetDragAction(DragAction& dragAction)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragAction(dragAction);
}

int32_t DeviceStatusClient::GetExtraInfo(std::string &extraInfo)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetExtraInfo(extraInfo);
}

int32_t DeviceStatusClient::AddHotAreaListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->AddHotAreaListener();
}

int32_t DeviceStatusClient::RemoveHotAreaListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->RemoveHotAreaListener();
}

int32_t DeviceStatusClient::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdatePreviewStyle(previewStyle);
}

int32_t DeviceStatusClient::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle,
    const PreviewAnimation &animation)
{
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdatePreviewStyleWithAnimation(previewStyle, animation);
}

int32_t DeviceStatusClient::GetDragSummary(std::map<std::string, int64_t> &summarys)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragSummary(summarys);
}

int32_t DeviceStatusClient::EnterTextEditorArea(bool enable)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->EnterTextEditorArea(enable);
}

int32_t DeviceStatusClient::AddPrivilege()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->AddPrivilege();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
