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

#include "devicestatus_define.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusManager" };
} // namespace

void DeviceStatusManager::DeviceStatusCallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    CHKPV(remote);
    FI_HILOGI("Recv death notice");
}

bool DeviceStatusManager::Init()
{
    CALL_DEBUG_ENTER;
    if (devicestatusCBDeathRecipient_ == nullptr) {
        devicestatusCBDeathRecipient_ = new (std::nothrow) DeviceStatusCallbackDeathRecipient();
        if (devicestatusCBDeathRecipient_ == nullptr) {
            FI_HILOGE("devicestatusCBDeathRecipient_ failed");
            return false;
        }
    }

    msdpImpl_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    CHKPF(msdpImpl_);

    FI_HILOGD("Init success");
    return true;
}

Data DeviceStatusManager::GetLatestDeviceStatusData(Type type)
{
    CALL_DEBUG_ENTER;
    Data data = {type, OnChangedValue::VALUE_EXIT};
    if ((type <= TYPE_INVALID) || (type >= TYPE_MAX)) {
        FI_HILOGE("GetLatestDeviceStatusData type_:%{public}d is error", type);
        return data;
    }
    if (msdpImpl_ == nullptr) {
        FI_HILOGE("msdpImpl_ is nullptr");
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
    CALL_DEBUG_ENTER;
    if (type == Type::TYPE_INVALID) {
        FI_HILOGE("Enable is failed");
        return false;
    }
    InitAlgoMngrInterface(type);
    InitDataCallback();
    return true;
}

bool DeviceStatusManager::Disable(Type type)
{
    CALL_DEBUG_ENTER;
    CHKPF(msdpImpl_);

    if (msdpImpl_->Disable(type) != RET_OK) {
        FI_HILOGE("Disable msdp impl failed");
        return false;
    }

    return true;
}

bool DeviceStatusManager::InitAlgoMngrInterface(Type type)
{
    CALL_DEBUG_ENTER;
    CHKPF(msdpImpl_);

    if (msdpImpl_->InitMsdpImpl(type) != RET_OK) {
        FI_HILOGE("Init msdp impl failed");
        return false;
    };
    return true;
}

int32_t DeviceStatusManager::InitDataCallback()
{
    CALL_DEBUG_ENTER;
    CHKPF(msdpImpl_);
    DeviceStatusMsdpClientImpl::CallbackManager callback =
        std::bind(&DeviceStatusManager::MsdpDataCallback, this, std::placeholders::_1);
    if (msdpImpl_->RegisterImpl(callback) == RET_ERR) {
        FI_HILOGE("Register impl failed");
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
    CALL_DEBUG_ENTER;
    FI_HILOGI("type:%{public}d, value:%{public}d", devicestatusData.type, devicestatusData.value);
    std::set<const sptr<IRemoteDevStaCallback>, classcomp> listeners;
    std::lock_guard lock(mutex_);
    auto iter = listeners_.find(devicestatusData.type);
    if (iter == listeners_.end()) {
        FI_HILOGE("type:%{public}d is not exits", devicestatusData.type);
        return false;
    }
    listeners = (std::set<const sptr<IRemoteDevStaCallback>, classcomp>)(iter->second);
    for (const auto &listener : listeners) {
        if (listener == nullptr) {
            FI_HILOGE("Listener is nullptr");
            return false;
        }
        FI_HILOGI("type:%{public}d, arrs_:%{public}d", devicestatusData.type, arrs_[devicestatusData.type]);
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
                FI_HILOGE("OnChangedValue is unknown");
                break;
            }
        }
    }
    return RET_OK;
}

void DeviceStatusManager::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    event_ = event;
    type_ = type;
    if ((type_ <= TYPE_INVALID) || (type_ >= TYPE_MAX)) {
        FI_HILOGE("Subscribe type_:%{public}d is error", type_);
        return;
    }
    if ((event_ < ENTER) || (event_ > ENTER_EXIT)) {
        FI_HILOGE("Subscribe event_:%{public}d is error", event_);
        return;
    }
    arrs_ [type_] = event_;
    FI_HILOGI("type_:%{public}d, event:%{public}d", type_, event);
    std::set<const sptr<IRemoteDevStaCallback>, classcomp> listeners;
    FI_HILOGI("listeners_.size:%{public}zu", listeners_.size());
    auto object = callback->AsObject();
    CHKPV(object);
    std::lock_guard lock(mutex_);
    auto dtTypeIter = listeners_.find(type);
    if (dtTypeIter == listeners_.end()) {
        if (listeners.insert(callback).second) {
            FI_HILOGI("No found set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
        auto [_, ret] = listeners_.insert(std::make_pair(type, listeners));
        if (!ret) {
            FI_HILOGW("type is duplicated");
        }
    } else {
        FI_HILOGI("callbacklist.size:%{public}zu", listeners_[dtTypeIter->first].size());
        auto iter = listeners_[dtTypeIter->first].find(callback);
        if (iter != listeners_[dtTypeIter->first].end()) {
            return;
        }
        if (listeners_[dtTypeIter->first].insert(callback).second) {
            FI_HILOGI("Find set list of type, insert success");
            object->AddDeathRecipient(devicestatusCBDeathRecipient_);
        }
    }
    if (!Enable(type)) {
        FI_HILOGE("Enable failed");
        return;
    }
}

void DeviceStatusManager::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    if ((type <= TYPE_INVALID) || (type >= TYPE_MAX)) {
        FI_HILOGE("Unsubscribe type_:%{public}d is error", type);
        return;
    }
    if ((event < ENTER) || (event > ENTER_EXIT)) {
        FI_HILOGE("Unsubscribe event_:%{public}d is error", event);
        return;
    }
    auto object = callback->AsObject();
    CHKPV(object);
    FI_HILOGE("listeners_.size:%{public}zu, arrs_:%{public}d", listeners_.size(), arrs_ [type_]);
    FI_HILOGE("UNevent:%{public}d", event);
    std::lock_guard lock(mutex_);
    auto dtTypeIter = listeners_.find(type);
    if (dtTypeIter == listeners_.end()) {
        FI_HILOGE("Failed to find listener for type");
        return;
    }
    FI_HILOGI("callbacklist.size:%{public}zu", listeners_[dtTypeIter->first].size());
    auto iter = listeners_[dtTypeIter->first].find(callback);
    if (iter != listeners_[dtTypeIter->first].end()) {
        if (listeners_[dtTypeIter->first].erase(callback) != 0) {
            object->RemoveDeathRecipient(devicestatusCBDeathRecipient_);
            if (listeners_[dtTypeIter->first].empty()) {
                listeners_.erase(dtTypeIter);
            }
        }
    }
    FI_HILOGI("listeners_.size:%{public}zu", listeners_.size());
    if (listeners_.empty()) {
        Disable(type);
    } else {
        FI_HILOGI("Other subscribe exist");
    }
}

int32_t DeviceStatusManager::LoadAlgorithm()
{
    CALL_DEBUG_ENTER;
    if (msdpImpl_ != nullptr) {
        msdpImpl_->LoadAlgoLibrary();
    }
    return RET_OK;
}

int32_t DeviceStatusManager::UnloadAlgorithm()
{
    CALL_DEBUG_ENTER;
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
                FI_HILOGE("Get hap token info failed");
                return RET_ERR;
            }
            packageName = hapInfo.bundleName;
            break;
        }
        case ATokenTypeEnum::TOKEN_NATIVE:
        case ATokenTypeEnum::TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                FI_HILOGE("Get native token info failed");
                return RET_ERR;
            }
            packageName = tokenInfo.processName;
            break;
        }
        default: {
            FI_HILOGE("token type not match");
            break;
        }
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
