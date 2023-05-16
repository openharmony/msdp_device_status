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
#include "devicestatus_define.h"
#include "devicestatus_manager.h"
#include "bytrace_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::HiviewDFX;
void DeviceStatusManager::DeviceStatusCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DEV_HILOGE(SERVICE, "OnRemoteDied failed, remote is nullptr");
        return;
    }
    DEV_HILOGD(SERVICE, "Recv death notice");
}

bool DeviceStatusManager::Init()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (devicestatusCBDeathRecipient_ == nullptr) {
        devicestatusCBDeathRecipient_ = new (std::nothrow) DeviceStatusCallbackDeathRecipient();
        if (devicestatusCBDeathRecipient_ == nullptr) {
            DEV_HILOGE(SERVICE, "devicestatusCBDeathRecipient_ failed");
            return false;
        }
    }

    msdpImpl_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_ is nullptr");
        return false;
    }

    DEV_HILOGD(SERVICE, "Init success");
    return true;
}

Data DeviceStatusManager::GetLatestDeviceStatusData(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    Data data = {type, OnChangedValue::VALUE_EXIT};
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "GetObserverData func is nullptr, return default");
        data.value = OnChangedValue::VALUE_INVALID;
        return data;
    }
    msdpData_ = msdpImpl_->GetObserverData();
    for (auto iter = msdpData_.begin(); iter != msdpData_.end(); ++iter) {
        if (data.type == iter->first) {
            data.value = iter->second;
            return data;
        }
    }
    return {type, OnChangedValue::VALUE_INVALID};
}

bool DeviceStatusManager::Enable(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (type == Type::TYPE_INVALID) {
        DEV_HILOGE(SERVICE, "enable is failed");
        return false;
    }
    InitAlgoMngrInterface(type);
    InitDataCallback();
    return true;
}

bool DeviceStatusManager::Disable(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "disable failed, msdpImpl is nullptr");
        return false;
    }

    if (msdpImpl_->Disable(type) != RET_OK) {
        DEV_HILOGE(SERVICE, "disable msdp impl failed");
        return false;
    }

    return true;
}

bool DeviceStatusManager::InitAlgoMngrInterface(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_ is nullptr");
        return false;
    }

    if (msdpImpl_->InitMsdpImpl(type) != RET_OK) {
        DEV_HILOGE(SERVICE, "init msdp impl failed");
        return false;
    };
    return true;
}

int32_t DeviceStatusManager::InitDataCallback()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (msdpImpl_ == nullptr) {
        DEV_HILOGE(SERVICE, "msdpImpl_ is nullptr");
        return false;
    }
    DeviceStatusMsdpClientImpl::CallbackManager callback =
        std::bind(&DeviceStatusManager::MsdpDataCallback, this, std::placeholders::_1);
    if (msdpImpl_->RegisterImpl(callback) == RET_ERR) {
        DEV_HILOGE(SERVICE, "register impl failed");
    }
    return true;
}

int32_t DeviceStatusManager::MsdpDataCallback(const Data& data)
{
    NotifyDeviceStatusChange(data);
    return RET_OK;
}

int32_t DeviceStatusManager::NotifyDeviceStatusChange(const Data& devicestatusData)
{
    DEV_HILOGD(SERVICE, "Enter");
    DEV_HILOGI(SERVICE, "type:%{public}d, value:%{public}d", devicestatusData.type, devicestatusData.value);
    std::set<const sptr<IRemoteDevStaCallback>, classcomp> listeners;
    auto iter = listenerMap_.find(devicestatusData.type);
    if (iter == listenerMap_.end()) {
        DEV_HILOGI(SERVICE, "type:%{public}d", devicestatusData.type);
        return false;
    }
    listeners = (std::set<const sptr<IRemoteDevStaCallback>, classcomp>)(iter->second);
    for (const auto &listener : listeners) {
        if (listener == nullptr) {
            DEV_HILOGE(SERVICE, "Listener is nullptr");
            return false;
        }
        DEV_HILOGI(SERVICE, "type:%{public}d, arrs_:%{public}d", devicestatusData.type, arrs_[devicestatusData.type]);
        switch (arrs_[devicestatusData.type]) {
            case ENTER: {
                if (devicestatusData.value == VALUE_ENTER) {
                    listener->OnDeviceStatusChanged(devicestatusData);
                }
                break;
            }
            case EXIT: {
                if (devicestatusData.value == VALUE_EXIT) {
                    listener->OnDeviceStatusChanged(devicestatusData);
                }
                break;
            }
            case ENTER_EXIT: {
                listener->OnDeviceStatusChanged(devicestatusData);
                break;
            }
            default: {
                DEV_HILOGE(SERVICE, "Exit");
                break;
            }
        }
    }
    return RET_OK;
}

void DeviceStatusManager::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    DEV_RET_IF_NULL(callback == nullptr);
    event_ = event;
    type_ = type;
    arrs_ [type_] = event_;
    DEV_HILOGI(SERVICE, "arr save:%{public}d, event:%{public}d", type_, event);
    std::set<const sptr<IRemoteDevStaCallback>, classcomp> listeners;
    DEV_HILOGI(SERVICE, "listenerMap_.size:%{public}zu", listenerMap_.size());
    auto object = callback->AsObject();
    DEV_RET_IF_NULL(object == nullptr);
    std::lock_guard lock(mutex_);
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        if (listeners.insert(callback).second) {
            DEV_HILOGI(SERVICE, "no found set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
        listenerMap_.insert(std::make_pair(type, listeners));
    } else {
        DEV_HILOGI(SERVICE, "callbacklist.size:%{public}zu", listenerMap_[dtTypeIter->first].size());
        auto iter = listenerMap_[dtTypeIter->first].find(callback);
        if (iter != listenerMap_[dtTypeIter->first].end()) {
            return;
        }
        if (listenerMap_[dtTypeIter->first].insert(callback).second) {
            DEV_HILOGI(SERVICE, "find set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
    }
    if (!Enable(type)) {
        DEV_HILOGE(SERVICE, "Enable failed");
        return;
    }
    DEV_HILOGI(SERVICE, "Subscribe success, Exit");
}

void DeviceStatusManager::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGD(SERVICE, "Enter");
    DEV_RET_IF_NULL(callback == nullptr);
    auto object = callback->AsObject();
    DEV_RET_IF_NULL(object == nullptr);
    DEV_HILOGE(SERVICE, "listenerMap_.size:%{public}zu, arrs_:%{public}d", listenerMap_.size(), arrs_ [type_]);
    DEV_HILOGE(SERVICE, "UNevent:%{public}d", event);
    std::lock_guard lock(mutex_);
    auto dtTypeIter = listenerMap_.find(type);
    if (dtTypeIter == listenerMap_.end()) {
        DEV_HILOGE(SERVICE, "Failed to find listener for type");
        return;
    }
    DEV_HILOGI(SERVICE, "callbacklist.size:%{public}zu", listenerMap_[dtTypeIter->first].size());
    auto iter = listenerMap_[dtTypeIter->first].find(callback);
    if (iter != listenerMap_[dtTypeIter->first].end()) {
        if (listenerMap_[dtTypeIter->first].erase(callback) != 0) {
            object->RemoveDeathRecipient(devicestatusCBDeathRecipient_);
            if (listenerMap_[dtTypeIter->first].empty()) {
                listenerMap_.erase(dtTypeIter);
            }
        }
    }
    DEV_HILOGI(SERVICE, "listenerMap_.size:%{public}zu", listenerMap_.size());
    if (listenerMap_.empty()) {
        Disable(type);
    } else {
        DEV_HILOGI(SERVICE, "other subscribe exist");
    }
    DEV_HILOGI(SERVICE, "Unsubscribe success, Exit");
}

int32_t DeviceStatusManager::LoadAlgorithm()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->LoadAlgoLibrary();
    }

    return RET_OK;
}

int32_t DeviceStatusManager::UnloadAlgorithm()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (msdpImpl_ != nullptr) {
        msdpImpl_->UnloadAlgoLibrary();
    }

    return RET_OK;
}

int32_t DeviceStatusManager::GetPackageName(AccessTokenID tokenId, std::string &packageName)
{
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case ATokenTypeEnum::TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                DEV_HILOGE(SERVICE, "get hap token info fail");
                return RET_ERR;
            }
            packageName = hapInfo.bundleName;
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                DEV_HILOGE(SERVICE, "get native token info fail");
                return RET_ERR;
            }
            packageName = tokenInfo.processName;
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "token type not match");
            break;
        }
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
