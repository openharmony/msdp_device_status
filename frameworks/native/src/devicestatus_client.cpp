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

#include "coordination_manager_impl.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "fi_log.h"
#include "util.h"


namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DevicestatusClient" };
} // namespace

DevicestatusClient::DevicestatusClient() {}
DevicestatusClient::~DevicestatusClient()
{
    if (devicestatusProxy_ != nullptr) {
        auto remoteObject = devicestatusProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

ErrCode DevicestatusClient::Connect()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (devicestatusProxy_ != nullptr) {
        DEV_HILOGE(INNERKIT, "devicestatusProxy_ is nut nullptr");
        return ERR_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DEV_HILOGE(INNERKIT, "GetSystemAbilityManager failed");
        return E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        DEV_HILOGE(INNERKIT, "CheckSystemAbility failed");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DevicestatusDeathRecipient());
    if (deathRecipient_ == nullptr) {
        DEV_HILOGE(INNERKIT, "Failed to create DevicestatusDeathRecipient");
        return ERR_NO_MEMORY;
    }

    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        DEV_HILOGE(INNERKIT, "Add death recipient to Devicestatus service failed");
        return E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED;
    }

    devicestatusProxy_ = iface_cast<Idevicestatus>(remoteObject_);
    DEV_HILOGD(INNERKIT, "Connecting DevicestatusService success");
    return ERR_OK;
}

void DevicestatusClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    std::lock_guard<std::mutex> lock(mutex_);
    DEVICESTATUS_RETURN_IF(devicestatusProxy_ == nullptr);

    auto serviceRemote = devicestatusProxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        devicestatusProxy_ = nullptr;
    }
}

void DevicestatusClient::DevicestatusDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DEV_HILOGE(INNERKIT, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    DevicestatusClient::GetInstance().ResetProxy(remote);
    DEV_HILOGD(INNERKIT, "Recv death notice");
}

void DevicestatusClient::SubscribeCallback(const DevicestatusDataUtils::DevicestatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    DEVICESTATUS_RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return;
    }
    if (type > DevicestatusDataUtils::DevicestatusType::TYPE_INVALID
        && type <= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        devicestatusProxy_->Subscribe(type, callback);
    }
    return;
    DEV_HILOGD(INNERKIT, "Exit");
}

void DevicestatusClient::UnSubscribeCallback(const DevicestatusDataUtils::DevicestatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGD(INNERKIT, "Enter");
    DEVICESTATUS_RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return;
    }
    if (type > DevicestatusDataUtils::DevicestatusType::TYPE_INVALID
        && type <= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        devicestatusProxy_->UnSubscribe(type, callback);
    }
    return;
    DEV_HILOGD(INNERKIT, "Exit");
}

DevicestatusDataUtils::DevicestatusData DevicestatusClient::GetDevicestatusData(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGD(INNERKIT, "Enter");
    DevicestatusDataUtils::DevicestatusData devicestatusData;
    devicestatusData.type = DevicestatusDataUtils::DevicestatusType::TYPE_INVALID;
    devicestatusData.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;

    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), devicestatusData);
    if (devicestatusProxy_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusProxy_ is nullptr");
        return devicestatusData;
    }
    if (type > DevicestatusDataUtils::DevicestatusType::TYPE_INVALID
        && type <= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        devicestatusData = devicestatusProxy_->GetCache(type);
    }
    DEV_HILOGD(INNERKIT, "Exit");
    return devicestatusData;
}

int32_t DevicestatusClient::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->RegisterCoordinationListener();
}

int32_t DevicestatusClient::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->UnregisterCoordinationListener();
}

int32_t DevicestatusClient::EnableInputDeviceCoordination(int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->EnableInputDeviceCoordination(userData, enabled);
}

int32_t DevicestatusClient::StartInputDeviceCoordination(int32_t userData,
    const std::string &sinkDeviceId, int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->StartInputDeviceCoordination(userData, sinkDeviceId, srcInputDeviceId);
}

int32_t DevicestatusClient::StopDeviceCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->StopDeviceCoordination(userData);
}

int32_t DevicestatusClient::GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return devicestatusProxy_->GetInputDeviceCoordinationState(userData, deviceId);
}


int32_t DevicestatusClient::AllocSocketPair(const int32_t moduleType)
{
    CALL_DEBUG_ENTER;
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
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

int32_t DevicestatusClient::GetClientSocketFdOfAllocedSocketPair() const
{
    CALL_DEBUG_ENTER;
    return socketFd_;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
