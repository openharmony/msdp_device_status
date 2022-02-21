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

#include <iservice_registry.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <system_ability_definition.h>

namespace OHOS {
namespace Msdp {
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
        return ERR_OK;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_INNERKIT, "GetSystemAbilityManager failed");
        return E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED;
    }

    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_INNERKIT, "CheckSystemAbility failed");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new DevicestatusDeathRecipient());
    if (deathRecipient_ == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_INNERKIT, "Failed to create DevicestatusDeathRecipient");
        return ERR_NO_MEMORY;
    }

    if ((remoteObject_->IsProxyObject()) && (!remoteObject_->AddDeathRecipient(deathRecipient_))) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_INNERKIT, "Add death recipient to Devicestatus service failed");
        return E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED;
    }

    devicestatusProxy_ = iface_cast<Idevicestatus>(remoteObject_);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Connecting DevicestatusService success");
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
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_INNERKIT, "OnRemoteDied failed, remote is nullptr");
        return;
    }

    DevicestatusClient::GetInstance().ResetProxy(remote);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Recv death notice");
}

void DevicestatusClient::SubscribeCallback(const DevicestatusDataUtils::DevicestatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Enter");
    DEVICESTATUS_RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    devicestatusProxy_->Subscribe(type, callback);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Exit");
}

void DevicestatusClient::UnSubscribeCallback(const DevicestatusDataUtils::DevicestatusType& type, \
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Enter");
    DEVICESTATUS_RETURN_IF((callback == nullptr) || (Connect() != ERR_OK));
    devicestatusProxy_->UnSubscribe(type, callback);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Exit");
}

DevicestatusDataUtils::DevicestatusData DevicestatusClient::GetDevicestatusData(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Enter");
    DevicestatusDataUtils::DevicestatusData devicestatusData;
    devicestatusData.type = DevicestatusDataUtils::DevicestatusType::TYPE_INVALID;
    devicestatusData.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;

    DEVICESTATUS_RETURN_IF_WITH_RET((Connect() != ERR_OK), devicestatusData);
    devicestatusData = devicestatusProxy_->GetCache(type);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_INNERKIT, "Exit");
    return devicestatusData;
}
} // namespace Msdp
} // namespace OHOS
