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

#include "devicestatus_manager.h"

namespace OHOS {
namespace Msdp {
using namespace OHOS::HiviewDFX;
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NG = -1;
}
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
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusCBDeathRecipient_ == nullptr) {
        devicestatusCBDeathRecipient_ = new DevicestatusCallbackDeathRecipient();
    }

    msdpImpl_ = std::make_unique<DevicestatusMsdpClientImpl>();
    LoadAlgorithm();

    DEV_HILOGI(SERVICE, "Init success");
    return true;
}

DevicestatusDataUtils::DevicestatusData DevicestatusManager::GetLatestDevicestatusData(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    DevicestatusDataUtils::DevicestatusData data = {type, DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT};
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "GetObserverData func is nullptr,return default!");
        data.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;
        return data;
    }
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

bool DevicestatusManager::EnableMock(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGE(SERVICE, "Enter");
    if (!InitInterface(type)) {
        DEV_HILOGE(SERVICE, "init interface fail");
        return false;
    }

    if (!InitDataCallback()) {
        DEV_HILOGE(SERVICE, "init msdp callback fail");
        return false;
    }
    return true;
}

bool DevicestatusManager::DisableMock(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "disable failed, msdpImpl is nullptr");
        return false;
    }

    if (msdpImpl_->DisableMsdpImpl(type) == ERR_NG) {
        DEV_HILOGE(SERVICE, "disable msdp impl failed");
        return false;
    }

    if (msdpImpl_->UnregisterImpl() == ERR_NG) {
        DEV_HILOGE(SERVICE, "unregister impl failed");
        return false;
    }

    return true;
}

bool DevicestatusManager::InitInterface(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_ is nullptr");
        return false;
    }
    if (msdpImpl_->InitMsdpImpl(type) == ERR_NG) {
        DEV_HILOGE(SERVICE, "init msdp impl failed");
        return false;
    };
    return true;
}

bool DevicestatusManager::InitDataCallback()
{
    DEV_HILOGE(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_ is nullptr");
        return false;
    }
    DevicestatusMsdpClientImpl::CallbackManager callback =
        std::bind(&DevicestatusManager::MsdpDataCallback, this, std::placeholders::_1);
    if (msdpImpl_->RegisterImpl(callback) == ERR_NG) {
        DEV_HILOGE(SERVICE, "register impl failed");
    }
    return true;
}

int32_t DevicestatusManager::MsdpDataCallback(const DevicestatusDataUtils::DevicestatusData& data)
{
    NotifyDevicestatusChange(data);
    return ERR_OK;
}

void DevicestatusManager::ProcessDeathObserver(wptr<IRemoteObject> object)
{
    DEV_HILOGI(SERVICE, "Recv death notice");
    std::lock_guard<std::mutex> lock(mutex_);
    if (object == nullptr) {
        DEV_HILOGE(SERVICE, "object is nullptr");
        return;
    }
    sptr<IRemoteObject> client = object.promote();
    for (auto& typeItem : listenerMap_) {
        DisableMock(typeItem.first);
        for (auto iter = typeItem.second.begin(); iter != typeItem.second.end();) {
            if ((*iter)->AsObject() == client) {
                iter = typeItem.second.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

void DevicestatusManager::NotifyDevicestatusChange(const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    DEV_HILOGI(SERVICE, "Enter");
    // Call back for all listeners
    std::lock_guard lock(mutex_);
    auto iter = listenerMap_.find(devicestatusData.type);
    if (iter == listenerMap_.end()) {
        DEV_HILOGI(SERVICE, "type:%{public}d is not exist", devicestatusData.type);
        return;
    }
    for (const auto& item : iter->second) {
        if (item == nullptr) {
            DEV_HILOGW(SERVICE, "listener is nullptr");
            continue;
        }
        item->OnDevicestatusChanged(devicestatusData);
    }
}

void DevicestatusManager::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (!EnableMock(type)) {
        DEV_HILOGE(SERVICE, "Enable failed!");
        return;
    }
    std::lock_guard lock(mutex_);
    if (clientDeathObserver_ == nullptr) {
        clientDeathObserver_ = new (std::nothrow) DeathRecipient(*const_cast<DevicestatusManager *>(this));
        if (clientDeathObserver_ == nullptr) {
            DEV_HILOGE(SERVICE, "clientDeathObserver_ is nullptr");
            return;
        }
    }
    DEVICESTATUS_RETURN_IF(callback == nullptr);
    auto object = callback->AsObject();
    DEVICESTATUS_RETURN_IF(object == nullptr);
    object->AddDeathRecipient(clientDeathObserver_);
    std::set<const sptr<IdevicestatusCallback>, classcomp> listeners;
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        if (!listeners.insert(callback).second) {
            DEV_HILOGW(SERVICE, "callback is duplicated");
            return;
        }
        listenerMap_.insert(std::make_pair(type, listeners));
    } else {
        DEV_HILOGD(SERVICE, "callbacklist.size:%{public}zu", listenerMap_[dtTypeIter->first].size());
        auto iter = listenerMap_[dtTypeIter->first].find(callback);
        if (iter != listenerMap_[dtTypeIter->first].end()) {
            DEV_HILOGW(SERVICE, "Failed to find type");
            return;
        }
        if (!listenerMap_[dtTypeIter->first].insert(callback).second) {
            DEV_HILOGE(SERVICE, "callback is duplicated");
            return;
        }
    }
    DEV_HILOGI(SERVICE, "listenerMap_.size:%{public}zu", listenerMap_.size());
    DEV_HILOGI(SERVICE, "Subscribe success,Exit");
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
    if (clientDeathObserver_ != nullptr) {
        object->RemoveDeathRecipient(clientDeathObserver_);
    }
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        DEV_HILOGE(SERVICE, "Failed to find type");
        return;
    }
    DEV_HILOGI(SERVICE, "callbacklist.size=%{public}zu", listenerMap_[dtTypeIter->first].size());
    auto iter = listenerMap_[dtTypeIter->first].find(callback);
    if (iter != listenerMap_[dtTypeIter->first].end()) {
        if (listenerMap_[dtTypeIter->first].erase(callback) != 0) {
            if (listenerMap_[dtTypeIter->first].size() == 0) {
                listenerMap_.erase(dtTypeIter);
            }
        }
    }
    DEV_HILOGI(SERVICE, "listenerMap_.size = %{public}zu", listenerMap_.size());
    if (listenerMap_.empty()) {
        DisableMock(type);
    } else {
        DEV_HILOGI(SERVICE, "other subscribe exist");
    }
    DEV_HILOGI(SERVICE, "UnSubscribe success,Exit");
}

int32_t DevicestatusManager::LoadAlgorithm()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_  is nullptr");
        return RET_ERR;
    }
    if (msdpImpl_->LoadAlgoLib() != RET_OK) {
        msdpImpl_->LoadAlgorithmLibrary();
    }
    return ERR_OK;
}

int32_t DevicestatusManager::UnloadAlgorithm()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_  is nullptr");
        return RET_ERR;
    }
    if (msdpImpl_->UnloadAlgoLib() != RET_OK) {
        msdpImpl_->UnloadAlgorithmLibrary();
    }
    return ERR_OK;
}

void DevicestatusManager::GetPackageName(AccessTokenID tokenId, std::string &packageName)
{
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                DEV_HILOGE(SERVICE, "get hap token info fail");
                return;
            }
            packageName = hapInfo.bundleName;
            break;
        }
        // Native type and shell type get processname in the same way
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                DEV_HILOGE(SERVICE, "get native token info fail");
                return;
            }
            packageName = tokenInfo.processName;
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "token type not match");
            break;
        }
    }
}
} // namespace Msdp
} // namespace OHOS
