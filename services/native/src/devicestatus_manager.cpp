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

#include "devicestatus_manager.h"
#include "devicestatus_msdp_client_impl.h"

namespace OHOS {
namespace Msdp {
void DevicestatusManager::DevicestatusCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "OnRemoteDied failed, remote is nullptr");
        return;
    }
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_SERVICE, "Recv death notice");
}

bool DevicestatusManager::Init()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusManager: Init start");
    if (devicestatusCBDeathRecipient_ == nullptr) {
        devicestatusCBDeathRecipient_ = new DevicestatusCallbackDeathRecipient();
    }

    msdpImpl_ = std::make_unique<DevicestatusMsdpClientImpl>();
    if (msdpImpl_ == nullptr) {
        return false;
    }
    LoadAlgorithm(false);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusManager: Init success");
    return true;
}

DevicestatusDataUtils::DevicestatusData DevicestatusManager::GetLatestDevicestatusData(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    DevicestatusDataUtils::DevicestatusData data = {type, DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT};
    msdpData_ = msdpImpl_->GetObserverData();
    for (auto iter = msdpData_.begin(); iter != msdpData_.end(); ++iter) {
        if (data.type == iter->first) {
            data.value = iter->second;
            return data;
        }
    }

    data.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;
    return data;
}

bool DevicestatusManager::EnableRdb()
{
    DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (!InitInterface()) {
        return false;
    }

    if (!InitDataCallback()) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "%{public}s init msdp callback fail.", __func__);
        return false;
    }
    return true;
}

bool DevicestatusManager::DisableRdb()
{
    DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "delete msdp client impl");
        msdpImpl_->DisableMsdpImpl();
        msdpImpl_->UnregisterImpl();
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "DisableRdb: after unregister impl");
    }
    return true;
}

bool DevicestatusManager::InitInterface()
{
    DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Init msdp client impl");
        msdpImpl_->InitMsdpImpl();
    }
    return true;
}

bool DevicestatusManager::InitDataCallback()
{
    DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DevicestatusMsdpClientImpl::CallbackManager callback =
            std::bind(&DevicestatusManager::MsdpDataCallback, this, std::placeholders::_1);
        msdpImpl_->RegisterImpl(callback);
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "InitDataCallback: after register impl");
    }
    return true;
}

int32_t DevicestatusManager::MsdpDataCallback(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    NotifyDevicestatusChange(data);
    return ERR_OK;
}

int32_t DevicestatusManager::SensorDataCallback(const struct SensorEvents *event)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    // TO-DO, handle sensor event properly when we get the data details of sensor HDI.
    DevicestatusDataUtils::DevicestatusData data = {DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL,
        DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER};
    NotifyDevicestatusChange(data);
    return ERR_OK;
}

void DevicestatusManager::NotifyDevicestatusChange(const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    // Call back for all listeners
    std::set<const sptr<IdevicestatusCallback>, classcomp> listeners;
    bool isExists = false;
    for (auto it = listenerMap_.begin(); it != listenerMap_.end(); ++it) {
        if (it->first == devicestatusData.type) {
            isExists = true;
            listeners = (std::set<const sptr<IdevicestatusCallback>, classcomp>)(it->second);
            break;
        }
    }
    if (!isExists) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "No listener found for type: %{public}d", \
            devicestatusData.type);
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
        return;
    }
    for (auto& listener : listeners) {
        listener->OnDevicestatusChanged(devicestatusData);
    }
}

void DevicestatusManager::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    DEVICESTATUS_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    DEVICESTATUS_RETURN_IF(object == nullptr);
    std::set<const sptr<IdevicestatusCallback>, classcomp> listeners;
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "listenerMap_.size=%{public}zu", listenerMap_.size());

    if (!EnableRdb()) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Enable failed!");
        return;
    }

    std::lock_guard lock(mutex_);
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        if (listeners.insert(callback).second) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "no found set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
        listenerMap_.insert(std::make_pair(type, listeners));
    } else {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "callbacklist.size=%{public}zu",
            listenerMap_[dtTypeIter->first].size());
        auto iter = listenerMap_[dtTypeIter->first].find(callback);
        if (iter != listenerMap_[dtTypeIter->first].end()) {
            return;
        } else {
            if (listenerMap_[dtTypeIter->first].insert(callback).second) {
                DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "found set list of type, insert success");
                object->AddDeathRecipient(devicestatusCBDeathRecipient_);
            }
        }
    }
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "object = %{public}p, callback = %{public}p",
        object.GetRefPtr(), callback.GetRefPtr());
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

void DevicestatusManager::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    std::lock_guard lock(mutex_);
    DEVICESTATUS_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    DEVICESTATUS_RETURN_IF(object == nullptr);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "listenerMap_.size=%{public}zu", listenerMap_.size());

    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        return;
    } else {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "callbacklist.size=%{public}zu",
            listenerMap_[dtTypeIter->first].size());
        auto iter = listenerMap_[dtTypeIter->first].find(callback);
        if (iter != listenerMap_[dtTypeIter->first].end()) {
            if (listenerMap_[dtTypeIter->first].erase(callback) != 0) {
                object->RemoveDeathRecipient(devicestatusCBDeathRecipient_);
                if (listenerMap_[dtTypeIter->first].size() == 0) {
                    listenerMap_.erase(dtTypeIter);
                }
            }
        }
    }
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "listenerMap_.size = %{public}d", listenerMap_.size());
    if (listenerMap_.size() == 0) {
        DisableRdb();
    } else {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "other subscribe exist");
    }
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "object = %{public}p, callback = %{public}p",
        object.GetRefPtr(), callback.GetRefPtr());
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

int32_t DevicestatusManager::LoadAlgorithm(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->LoadAlgorithmLibrary(bCreate);
        msdpImpl_->LoadSensorHdiLibrary(bCreate);
    }

    return ERR_OK;
}

int32_t DevicestatusManager::UnloadAlgorithm(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->UnloadAlgorithmLibrary(bCreate);
        msdpImpl_->UnloadSensorHdiLibrary(bCreate);
    }

    return ERR_OK;
}
} // namespace Msdp
} // namespace OHOS
