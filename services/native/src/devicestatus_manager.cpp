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
        DEV_HILOGE(SERVICE, "OnRemoteDied failed, remote is nullptr");
        return;
    }
    DEV_HILOGD(SERVICE, "Recv death notice");
}

bool DevicestatusManager::Init()
{
    DEV_HILOGI(SERVICE, "DevicestatusManager: Init start");
    if (devicestatusCBDeathRecipient_ == nullptr) {
        devicestatusCBDeathRecipient_ = new DevicestatusCallbackDeathRecipient();
    }

    msdpImpl_ = std::make_unique<DevicestatusMsdpClientImpl>();
    if (msdpImpl_ == nullptr) {
        return false;
    }
    LoadAlgorithm(false);

    DEV_HILOGI(SERVICE, "DevicestatusManager: Init success");
    return true;
}

DevicestatusDataUtils::DevicestatusData DevicestatusManager::GetLatestDevicestatusData(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
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
    DEV_HILOGE(SERVICE, "Enter");
    if (!InitInterface()) {
        return false;
    }

    if (!InitDataCallback()) {
        DEV_HILOGE(SERVICE, "%{public}s init msdp callback fail.", __func__);
        return false;
    }
    return true;
}

bool DevicestatusManager::DisableRdb()
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DEV_HILOGE(SERVICE, "delete msdp client impl");
        msdpImpl_->DisableMsdpImpl();
        msdpImpl_->UnregisterImpl();
        DEV_HILOGE(SERVICE, "DisableRdb: after unregister impl");
    }
    return true;
}

bool DevicestatusManager::InitInterface()
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DEV_HILOGE(SERVICE, "Init msdp client impl");
        msdpImpl_->InitMsdpImpl();
    }
    return true;
}

bool DevicestatusManager::InitDataCallback()
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        DevicestatusMsdpClientImpl::CallbackManager callback =
            std::bind(&DevicestatusManager::MsdpDataCallback, this, std::placeholders::_1);
        msdpImpl_->RegisterImpl(callback);
        DEV_HILOGE(SERVICE, "InitDataCallback: after register impl");
    }
    return true;
}

int32_t DevicestatusManager::MsdpDataCallback(DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");

    NotifyDevicestatusChange(data);
    return ERR_OK;
}

int32_t DevicestatusManager::SensorDataCallback(const struct SensorEvents *event)
{
    DEV_HILOGI(SERVICE, "Enter");
    // TO-DO, handle sensor event properly when we get the data details of sensor HDI.
    DevicestatusDataUtils::DevicestatusData data = {DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL,
        DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER};
    NotifyDevicestatusChange(data);
    return ERR_OK;
}

void DevicestatusManager::NotifyDevicestatusChange(const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    DEV_HILOGI(SERVICE, "Enter");

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
        DEV_HILOGI(SERVICE, "No listener found for type: %{public}d", \
            devicestatusData.type);
        DEV_HILOGI(SERVICE, "Exit");
        return;
    }
    for (auto& listener : listeners) {
        listener->OnDevicestatusChanged(devicestatusData);
    }
}

void DevicestatusManager::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    DEVICESTATUS_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    DEVICESTATUS_RETURN_IF(object == nullptr);
    std::set<const sptr<IdevicestatusCallback>, classcomp> listeners;
    DEV_HILOGI(SERVICE, "listenerMap_.size=%{public}zu", listenerMap_.size());

    if (!EnableRdb()) {
        DEV_HILOGE(SERVICE, "Enable failed!");
        return;
    }

    std::lock_guard lock(mutex_);
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        if (listeners.insert(callback).second) {
            DEV_HILOGI(SERVICE, "no found set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
        listenerMap_.insert(std::make_pair(type, listeners));
    } else {
        DEV_HILOGI(SERVICE, "callbacklist.size=%{public}zu",
            listenerMap_[dtTypeIter->first].size());
        auto iter = listenerMap_[dtTypeIter->first].find(callback);
        if (iter != listenerMap_[dtTypeIter->first].end()) {
            return;
        } else {
            if (listenerMap_[dtTypeIter->first].insert(callback).second) {
                DEV_HILOGI(SERVICE, "found set list of type, insert success");
                object->AddDeathRecipient(devicestatusCBDeathRecipient_);
            }
        }
    }
    DEV_HILOGI(SERVICE, "object = %{public}p, callback = %{public}p",
        object.GetRefPtr(), callback.GetRefPtr());
    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusManager::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard lock(mutex_);
    DEVICESTATUS_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    DEVICESTATUS_RETURN_IF(object == nullptr);
    DEV_HILOGI(SERVICE, "listenerMap_.size=%{public}zu", listenerMap_.size());

    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        return;
    } else {
        DEV_HILOGI(SERVICE, "callbacklist.size=%{public}zu",
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
    DEV_HILOGI(SERVICE, "listenerMap_.size = %{public}d", listenerMap_.size());
    if (listenerMap_.size() == 0) {
        DisableRdb();
    } else {
        DEV_HILOGI(SERVICE, "other subscribe exist");
    }
    DEV_HILOGI(SERVICE, "object = %{public}p, callback = %{public}p",
        object.GetRefPtr(), callback.GetRefPtr());
    DEV_HILOGI(SERVICE, "Exit");
}

int32_t DevicestatusManager::LoadAlgorithm(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->LoadAlgorithmLibrary(bCreate);
        msdpImpl_->LoadSensorHdiLibrary(bCreate);
    }

    return ERR_OK;
}

int32_t DevicestatusManager::UnloadAlgorithm(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->UnloadAlgorithmLibrary(bCreate);
        msdpImpl_->UnloadSensorHdiLibrary(bCreate);
    }

    return ERR_OK;
}
} // namespace Msdp
} // namespace OHOS
