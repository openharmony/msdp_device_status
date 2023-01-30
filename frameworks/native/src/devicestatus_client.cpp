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

#include "devicestatus_client.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "fi_log.h"
#include "util.h"

#include "coordination_manager_impl.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "drag_manager_impl.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusClient" };
} // namespace

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
        DEV_HILOGD(INNERKIT, "devicestatusProxy_ is not nullptr");
        return RET_OK;
    }

    sptr<ISystemAbilityManager> sa = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sa == nullptr) {
        DEV_HILOGE(INNERKIT, "GetSystemAbilityManager failed");
        return E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> remoteObject = sa->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    if (remoteObject == nullptr) {
        DEV_HILOGE(INNERKIT, "CheckSystemAbility failed");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) DeviceStatusDeathRecipient());
    if (deathRecipient_ == nullptr) {
        DEV_HILOGE(INNERKIT, "Failed to create DeviceStatusDeathRecipient");
        return ERR_NO_MEMORY;
    }

    if (remoteObject->IsProxyObject()) {
        if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
            DEV_HILOGE(INNERKIT, "Add death recipient to DeviceStatus service failed");
            return E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED;
        }
    }

    devicestatusProxy_ = iface_cast<Idevicestatus>(remoteObject);
    DEV_HILOGD(INNERKIT, "Connecting DeviceStatusService success");
    return RET_OK;
}

void DeviceStatusClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    DEV_RET_IF_NULL(devicestatusProxy_ == nullptr);

    auto serviceRemote = devicestatusProxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        devicestatusProxy_ = nullptr;
    }
    if (deathListener_ != nullptr) {
        DEV_HILOGI(INNERKIT, "notify death listner");
        deathListener_();
    }
}

void DeviceStatusClient::DeviceStatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DEV_HILOGE(INNERKIT, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    DeviceStatusClient::GetInstance().ResetProxy(remote);
    DEV_HILOGD(INNERKIT, "Recv death notice");
}

void DeviceStatusClient::SubscribeCallback(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGI(INNERKIT, "Enter event:%{public}d,latency:%{public}d", event, latency);
    typeMap_.insert(std::make_pair(type, 1));
    DEV_HILOGD(INNERKIT, "typeMap_ %{public}d, type: %{public}d", typeMap_[type], type);
    DEV_RET_IF_NULL((callback == nullptr) || (Connect() != RET_OK));
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return;
    }
    if (type > Type::TYPE_INVALID && type <= Type::TYPE_LID_OPEN) {
        devicestatusProxy_->Subscribe(type, event, latency, callback);
    }
    return;
}

void DeviceStatusClient::UnsubscribeCallback(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGI(INNERKIT, "UNevent: %{public}d", event);
    typeMap_.erase(type);
    DEV_HILOGD(INNERKIT, "typeMap_ %{public}d", typeMap_[type]);
    DEV_RET_IF_NULL((callback == nullptr) || (Connect() != RET_OK));
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return;
    }
    if ((type < TYPE_INVALID) || (type > TYPE_MAX)) {
        DEV_HILOGE(INNERKIT, "type out of range");
        return;
    }
    if (event < ActivityEvent::EVENT_INVALID || event > ActivityEvent::ENTER_EXIT) {
        DEV_HILOGE(INNERKIT, "event out of range");
        return;
    }
    devicestatusProxy_->Unsubscribe(type, event, callback);
    DEV_HILOGD(INNERKIT, "Exit");
    return;
}

Data DeviceStatusClient::GetDeviceStatusData(Type type)
{
    DEV_HILOGD(INNERKIT, "Enter");
    Data devicestatusData;
    devicestatusData.type = Type::TYPE_INVALID;
    devicestatusData.value = OnChangedValue::VALUE_INVALID;

    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), devicestatusData);
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return devicestatusData;
    }
    if (type > Type::TYPE_INVALID
        && type <= Type::TYPE_LID_OPEN) {
        devicestatusData = devicestatusProxy_->GetCache(type);
    }
    DEV_HILOGD(INNERKIT, "Exit");
    return devicestatusData;
}

int32_t DeviceStatusClient::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->RegisterCoordinationListener();
}

int32_t DeviceStatusClient::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UnregisterCoordinationListener();
}

int32_t DeviceStatusClient::EnableCoordination(int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->EnableCoordination(userData, enabled);
}

int32_t DeviceStatusClient::StartCoordination(int32_t userData,
    const std::string &sinkDeviceId, int32_t srcDeviceId)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->StartCoordination(userData, sinkDeviceId, srcDeviceId);
}

int32_t DeviceStatusClient::StopCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->StopCoordination(userData);
}

int32_t DeviceStatusClient::GetCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetCoordinationState(userData, deviceId);
}

int32_t DeviceStatusClient::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdateDragStyle(style);
}

int32_t DeviceStatusClient::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->UpdateDragMessage(message);
}

int32_t DeviceStatusClient::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->GetDragTargetPid();
}

int32_t DeviceStatusClient::AllocSocketPair(const int32_t moduleType)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    std::lock_guard<std::mutex> guard(mutex_);
    if (devicestatusProxy_ == nullptr) {
        FI_HILOGE("Client has not connect server");
        return RET_ERR;
    }

    const std::string programName(GetProgramName());
    int32_t result = devicestatusProxy_->AllocSocketFd(programName, moduleType, socketFd_, tokenType_);
    if (result != RET_OK) {
        FI_HILOGE("AllocSocketFd has error:%{public}d", result);
        return RET_ERR;
    }

    FI_HILOGI("AllocSocketPair success. socketFd_:%{public}d tokenType_:%{public}d", socketFd_, tokenType_);
    return RET_OK;
}

int32_t DeviceStatusClient::GetClientSocketFdOfAllocedSocketPair() const
{
    CALL_DEBUG_ENTER;
    return socketFd_;
}

void DeviceStatusClient::RegisterDeathListener(std::function<void()> deathListener)
{
    deathListener_ = deathListener;
}

int32_t DeviceStatusClient::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->StartDrag(dragData);
}

int32_t DeviceStatusClient::StopDrag(int32_t dragResult)
{
    CALL_DEBUG_ENTER;
    DEV_RET_IF_NULL_WITH_RET((Connect() != RET_OK), RET_ERR);
    return devicestatusProxy_->StopDrag(dragResult);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
