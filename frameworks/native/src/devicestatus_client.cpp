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

#include "devicestatus_common.h"

#include "iremote_broker.h"
#include "iremote_object.h"

#include "coordination_manager_impl.h"
#include "fi_log.h"

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

int32_t DevicestatusClient::RegisterCoordinationListener(
    std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(listener, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return InputDevCoordinationImpl.RegisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}

int32_t DevicestatusClient::UnregisterCoordinationListener(
    std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(listener, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    return InputDevCoordinationImpl.UnregisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}

int32_t DevicestatusClient::EnableInputDeviceCoordination(bool enabled,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(callback, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    auto tempUserData = InputDevCoordinationImpl.AddCoordinationUserData(callback);
    if (!tempUserData) {
        FI_HILOGE("Failed to add user data");
        return RET_ERR;
    }
    int32_t userData = tempUserData.value();
    return devicestatusProxy_>EnableInputDeviceCoordination(userData, enabled);
#else
    FI_HILOGW("Coordination does not support");
    (void)(enabled);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}

int32_t DevicestatusClient::StartInputDeviceCoordination(const std::string &sinkDeviceId,
    int32_t srcInputDeviceId, std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(callback, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    auto tempUserData = InputDevCoordinationImpl.AddCoordinationUserData(callback);
    if (!tempUserData) {
        FI_HILOGE("Failed to add user data");
        return RET_ERR;
    }
    int32_t userData = tempUserData.value();
    return InputDevCoordinationImpl.StartInputDeviceCoordination(userData, sinkDeviceId, srcInputDeviceId);
#else
    FI_HILOGW("Coordination does not support");
    (void)(sinkDeviceId);
    (void)(srcInputDeviceId);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}

int32_t DevicestatusClient::StopDeviceCoordination(
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(callback, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    auto tempUserData = InputDevCoordinationImpl.AddCoordinationUserData(callback);
    if (!tempUserData) {
        FI_HILOGE("Failed to add user data");
        return RET_ERR;
    }
    int32_t userData = tempUserData.value();
    return InputDevCoordinationImpl.StopDeviceCoordination(userData);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}

int32_t DevicestatusClient::GetInputDeviceCoordinationState(
    const std::string &deviceId, std::function<void(bool)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    CHKPR(callback, ERROR_NULL_POINTER);
    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), RET_ERR);
    auto tempUserData = InputDevCoordinationImpl.AddCoordinationUserData(callback);
    if (!tempUserData) {
        FI_HILOGE("Failed to add user data");
        return RET_ERR;
    }
    int32_t userData = tempUserData.value();
    return InputDevCoordinationImpl.GetInputDeviceCoordinationState(userData, deviceId);
#else
    FI_HILOGW("Coordination does not support");
    (void)(deviceId);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COOPERATE
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
