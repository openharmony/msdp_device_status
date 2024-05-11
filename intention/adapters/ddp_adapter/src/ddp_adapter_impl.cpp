/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ddp_adapter_impl.h"

#include <unistd.h>

#include "device_manager.h"
#include "distributed_device_profile_client.h"

#include "devicestatus_define.h"
#include "dp_change_listener.h"
#include "json_parser.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "DDPAdapterImpl"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
using namespace OHOS::DistributedDeviceProfile;
const std::string SERVICE_ID { "deviceStatus" };
const std::string SERVICE_TYPE { "deviceStatus" };
const std::string CROSSING_SWITCH_STATE { "crossingSwitchState" };
const std::string CHARACTERISTIC_VALUE { "characteristicValue" };
constexpr int32_t DEVICE_STATUS_SA_ID { 2902 };
constexpr int32_t DP_ERROR_CODE_DATA_EXIST { 98566164 };
const std::string PKG_NAME_PREFIX { "DBinderBus_Dms_" };
#define DSTB_HARDWARE DistributedHardware::DeviceManager::GetInstance()
} // namespace

#define DDP_CLIENT  DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance()

void DDPAdapterImpl::OnProfileChanged(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard guard(mutex_);
    FI_HILOGI("Profile of \'%{public}s\' has changed", Utility::Anonymize(networkId).c_str());
    for (const auto &item : observers_) {
        std::shared_ptr<IDeviceProfileObserver> observer = item.Lock();
        if (observer != nullptr) {
            FI_HILOGD("Notify profile change: \'%{public}s\'", Utility::Anonymize(networkId).c_str());
            observer->OnProfileChanged(networkId);
        }
    }
}

void DDPAdapterImpl::AddObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_INFO_TRACE;
    std::lock_guard guard(mutex_);
    CHKPV(observer);
    observers_.erase(Observer());
    observers_.emplace(observer);
}

void DDPAdapterImpl::RemoveObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_INFO_TRACE;
    std::lock_guard guard(mutex_);
    if (auto iter = observers_.find(Observer(observer)); iter != observers_.end()) {
        observers_.erase(iter);
    }
    observers_.erase(Observer());
}

void DDPAdapterImpl::AddWatch(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard guard(mutex_);
    FI_HILOGD("Add watch \'%{public}s\'", Utility::Anonymize(networkId).c_str());
    RegisterProfileListener(networkId);
    auto udId = GetUdIdByNetworkId(networkId);
    FI_HILOGI("OnDeviceOnline, networkId:%{public}s, udId:%{public}s",
        Utility::Anonymize(networkId).c_str(), Utility::Anonymize(udId).c_str());
    udId2NetworkId_[udId] = networkId;
}

void DDPAdapterImpl::RemoveWatch(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard guard(mutex_);
    FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(networkId).c_str());
    UnregisterProfileListener(networkId);
    auto udId = GetUdIdByNetworkId(networkId);
    FI_HILOGI("OnDeviceOffline, networkId:%{public}s, udId:%{public}s",
        Utility::Anonymize(networkId).c_str(), Utility::Anonymize(udId).c_str());
    if (udId2NetworkId_.find(udId) != udId2NetworkId_.end()) {
        udId2NetworkId_.erase(udId);
    }
}

int32_t DDPAdapterImpl::RegisterProfileListener(const std::string &networkId)
{
    CALL_INFO_TRACE;
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(DEVICE_STATUS_SA_ID);
    std::string udId = GetUdIdByNetworkId(networkId);
    subscribeInfo.SetSubscribeKey(udId, SERVICE_ID, CROSSING_SWITCH_STATE, CHARACTERISTIC_VALUE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    sptr<IProfileChangeListener> subscribeDPChangeListener =
        new (std::nothrow) DeviceProfileChangeListener(shared_from_this());
    CHKPR(subscribeDPChangeListener, RET_ERR);
    subscribeInfo.SetListener(subscribeDPChangeListener);
    if (int32_t ret = DDP_CLIENT.SubscribeDeviceProfile(subscribeInfo); ret != RET_OK) {
        FI_HILOGE("SubscribeDeviceProfile failed, ret:%{public}d, udId:%{public}s",
            ret, Utility::Anonymize(udId).c_str());
        return RET_ERR;
    }
    crossingSwitchSubscribeInfo_.emplace(networkId, subscribeInfo);
    return RET_OK;
}

int32_t DDPAdapterImpl::UnregisterProfileListener(const std::string &networkId)
{
    CALL_INFO_TRACE;
    FI_HILOGI("Unregister profile listener for \'%{public}s\'", Utility::Anonymize(networkId).c_str());
    if (crossingSwitchSubscribeInfo_.find(networkId) == crossingSwitchSubscribeInfo_.end()) {
        FI_HILOGE("NetworkId:%{public}s is not founded in crossingSwitchSubscribeInfo",
            Utility::Anonymize(networkId).c_str());
        return RET_ERR;
    }
    auto subscribeInfo = crossingSwitchSubscribeInfo_[networkId];
    if (int32_t ret = DDP_CLIENT.UnSubscribeDeviceProfile(subscribeInfo); ret != RET_OK) {
        FI_HILOGE("UnSubscribeDeviceProfile failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    crossingSwitchSubscribeInfo_.erase(networkId);
    return RET_OK;
}

int32_t DDPAdapterImpl::UpdateCrossingSwitchState(bool state)
{
    CALL_INFO_TRACE;
    if (PutServiceProfile() != RET_OK) {
        FI_HILOGE("PutServiceProfile failed");
        return RET_ERR;
    }
    std::string profileStr = (state ? "true" : "false");
    if (PutCharacteristicProfile(profileStr) != RET_OK) {
        FI_HILOGE("PutCharacteristicProfile failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DDPAdapterImpl::GetCrossingSwitchState(const std::string &udId, bool &state)
{
    CALL_INFO_TRACE;
    DistributedDeviceProfile::CharacteristicProfile profile;
    if (int32_t ret = DDP_CLIENT.GetCharacteristicProfile(udId, SERVICE_ID, CROSSING_SWITCH_STATE, profile);
        ret != RET_OK) {
        FI_HILOGE("GetCharacteristicProfile failed, ret: %{public}d, udId: %{public}s",
            ret, Utility::Anonymize(udId).c_str());
    }
    state = (profile.GetCharacteristicValue() == "true" ? true : false);
    FI_HILOGI("GetCrossingSwitchState for udId: %{public}s successfully,state: %{public}s",
        Utility::Anonymize(udId).c_str(), state ? "true" : "false");
    return RET_OK;
}

int32_t DDPAdapterImpl::GetProperty(const std::string &udId, const std::string &name, bool &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(udId, name, [&name, &value](cJSON *json) {
        FI_HILOGD("Get bool property: %{public}s", name.c_str());
        if (cJSON_IsBool(json)) {
            value = cJSON_IsTrue(json);
        } else if (cJSON_IsNumber(json)) {
            value = (static_cast<int32_t>(json->valuedouble) != 0);
        } else {
            FI_HILOGE("Unexpected data type");
            return RET_ERR;
        }
        return RET_OK;
    });
}

int32_t DDPAdapterImpl::GetProperty(const std::string &udId, const std::string &name, int32_t &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(udId, name, [&name, &value](cJSON *json) {
        FI_HILOGD("Get integer property: %{public}s", name.c_str());
        if (!cJSON_IsNumber(json)) {
            FI_HILOGE("Unexpected data type");
            return RET_ERR;
        }
        value = static_cast<int32_t>(json->valuedouble);
        return RET_OK;
    });
}

int32_t DDPAdapterImpl::GetProperty(const std::string &udId, const std::string &name, std::string &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(udId, name, [&name, &value](cJSON *json) {
        FI_HILOGD("Get string property: %{public}s", name.c_str());
        if (!cJSON_IsString(json) && !cJSON_IsRaw(json)) {
            FI_HILOGE("Unexpected data type");
            return RET_ERR;
        }
        CHKPR(json->valuestring, RET_ERR);
        value = json->valuestring;
        return RET_OK;
    });
}

int32_t DDPAdapterImpl::GetProperty(const std::string &udId, const std::string &name,
    std::function<int32_t(cJSON *json)> parse)
{
    CALL_DEBUG_ENTER;
    FI_HILOGI("GetProperty for udId:%{public}s", Utility::Anonymize(udId).c_str());
    DistributedDeviceProfile::CharacteristicProfile profile;
    if (int32_t ret = DDP_CLIENT.GetCharacteristicProfile(udId, SERVICE_ID, CROSSING_SWITCH_STATE, profile);
        ret != RET_OK) {
        FI_HILOGE("GetCharacteristicProfile failed, ret:%{public}d, udId:%{public}s",
            ret, Utility::Anonymize(udId).c_str());
        return RET_ERR;
    }
    std::string jsonData = profile.GetCharacteristicValue();
    JsonParser parser;
    parser.json = cJSON_Parse(jsonData.c_str());
    if (!cJSON_IsObject(parser.json)) {
        FI_HILOGE("Unexpected data format");
        return RET_ERR;
    }
    cJSON* jsonValue = cJSON_GetObjectItem(parser.json, name.c_str());
    if (jsonValue == nullptr) {
        FI_HILOGE("Item \'%{public}s\' not found", name.c_str());
        return RET_ERR;
    }
    return parse(jsonValue);
}

int32_t DDPAdapterImpl::SetProperty(const std::string &name, bool value)
{
    DPValue dpVal(std::in_place_type<bool>, value);
    return SetProperty(name, dpVal);
}

int32_t DDPAdapterImpl::SetProperty(const std::string &name, int32_t value)
{
    DPValue dpVal(std::in_place_type<int32_t>, value);
    return SetProperty(name, dpVal);
}

int32_t DDPAdapterImpl::SetProperty(const std::string &name, const std::string &value)
{
    DPValue dpVal(std::in_place_type<std::string>, value);
    return SetProperty(name, dpVal);
}

int32_t DDPAdapterImpl::SetProperty(const std::string &name, const DPValue &value)
{
    CALL_DEBUG_ENTER;
    if (auto iter = properties_.find(name); iter != properties_.end()) {
        if (iter->second == value) {
            return RET_OK;
        }
        iter->second = value;
    } else {
        properties_.emplace(name, value);
    }

    PutProfile();
    return RET_OK;
}

int32_t DDPAdapterImpl::GenerateProfileStr(std::string &profileStr)
{
    CALL_DEBUG_ENTER;
    JsonParser parser;
    parser.json = cJSON_CreateObject();
    CHKPR(parser.json, RET_ERR);
    for (const auto &[name, value] : properties_) {
        JsonParser parser1;
        std::visit(
            [&parser1](const auto &arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr(std::is_same_v<T, bool>) {
                    parser1.json = cJSON_CreateNumber(arg);
                } else if constexpr(std::is_same_v<T, int32_t>) {
                    parser1.json = cJSON_CreateNumber(arg);
                } else if constexpr(std::is_same_v<T, std::string>) {
                    parser1.json = cJSON_CreateString(arg.c_str());
                }
            }, value);
        CHKPR(parser1.json, RET_ERR);

        if (!cJSON_AddItemToObject(parser.json, name.c_str(), parser1.json)) {
            FI_HILOGE("Failed to add \'%{public}s\' to object", name.c_str());
            return RET_ERR;
        }
        parser1.json = nullptr;
    }
    char *cProfile = cJSON_Print(parser.json);
    CHKPR(cProfile, RET_ERR);
    profileStr = std::string(cProfile);
    cJSON_free(cProfile);
    return RET_OK;
}

int32_t DDPAdapterImpl::PutServiceProfile()
{
    CALL_INFO_TRACE;
    if (isServiceProfileExist_) {
        FI_HILOGW("ServiceProfile exist already");
        return RET_OK;
    }
    DistributedDeviceProfile::ServiceProfile serviceProfile;
    std::string localUdId = GetLocalUdId();
    serviceProfile.SetDeviceId(localUdId);
    serviceProfile.SetServiceName(SERVICE_ID);
    serviceProfile.SetServiceType(SERVICE_TYPE);
    int32_t ret = DDP_CLIENT.PutServiceProfile(serviceProfile);
    if (ret != RET_OK && ret != DP_ERROR_CODE_DATA_EXIST) {
        FI_HILOGE("PutServiceProfile failed, ret:%{public}d", ret);
        return ret;
    }
    FI_HILOGI("PutServiceProfile successfully, ret:%{public}d", ret);
    isServiceProfileExist_ = true;
    return RET_OK;
}

int32_t DDPAdapterImpl::PutCharacteristicProfile(const std::string &profileStr)
{
    CALL_INFO_TRACE;
    DistributedDeviceProfile::CharacteristicProfile characteristicProfile;
    std::string localUdId = GetLocalUdId();
    characteristicProfile.SetDeviceId(localUdId);
    characteristicProfile.SetServiceName(SERVICE_ID);
    characteristicProfile.SetCharacteristicKey(CROSSING_SWITCH_STATE);
    characteristicProfile.SetCharacteristicValue(profileStr);
    if (int32_t ret = DDP_CLIENT.PutCharacteristicProfile(characteristicProfile); ret != RET_OK) {
        FI_HILOGE("PutCharacteristicProfile failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DDPAdapterImpl::PutProfile()
{
    CALL_INFO_TRACE;
    if (PutServiceProfile() != RET_OK) {
        FI_HILOGE("PutServiceProfile failed");
        return RET_ERR;
    }
    std::string profileStr;
    if (GenerateProfileStr(profileStr) != RET_OK) {
        FI_HILOGE("GenerateProfileStr failed");
        return RET_ERR;
    }
    if (PutCharacteristicProfile(profileStr) != RET_OK) {
        FI_HILOGE("PutCharacteristicProfile failed");
        return RET_ERR;
    }
    return RET_OK;
}

std::string DDPAdapterImpl::GetCurrentPackageName()
{
    return PKG_NAME_PREFIX + std::to_string(getpid());
}

std::string DDPAdapterImpl::GetLocalNetworkId()
{
    CALL_DEBUG_ENTER;
    auto packageName = GetCurrentPackageName();
    OHOS::DistributedHardware::DmDeviceInfo dmDeviceInfo;
    if (int32_t errCode = DSTB_HARDWARE.GetLocalDeviceInfo(packageName, dmDeviceInfo); errCode != RET_OK) {
        FI_HILOGE("GetLocalBasicInfo failed, errCode:%{public}d", errCode);
        return {};
    }
    FI_HILOGD("LocalNetworkId:%{public}s", Utility::Anonymize(dmDeviceInfo.networkId).c_str());
    return dmDeviceInfo.networkId;
}

std::string DDPAdapterImpl::GetLocalUdId()
{
    CALL_DEBUG_ENTER;
    auto localNetworkId = GetLocalNetworkId();
    auto localUdId = GetUdIdByNetworkId(localNetworkId);
    FI_HILOGI("LocalNetworkId:%{public}s, localUdId:%{public}s",
        Utility::Anonymize(localNetworkId).c_str(), Utility::Anonymize(localUdId).c_str());
    return localUdId;
}

std::string DDPAdapterImpl::GetNetworkIdByUdId(const std::string &udId)
{
    CALL_DEBUG_ENTER;
    if (udId2NetworkId_.find(udId) == udId2NetworkId_.end()) {
        FI_HILOGE("UdId:%{public}s is not founded in udId2NetworkId", Utility::Anonymize(udId).c_str());
        return {};
    }
    FI_HILOGI("NetworkId:%{public}s for UdId:%{public}s",
        Utility::Anonymize(udId2NetworkId_[udId]).c_str(), Utility::Anonymize(udId).c_str());
    return udId2NetworkId_[udId];
}

std::string DDPAdapterImpl::GetUdIdByNetworkId(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::string udId { "Empty" };
    if (int32_t errCode = DSTB_HARDWARE.GetUdidByNetworkId(GetCurrentPackageName(), networkId, udId);
        errCode != RET_OK) {
        FI_HILOGE("GetUdIdByNetworkId failed, errCode:%{public}d, networkId:%{public}s, udId:%{public}s", errCode,
            Utility::Anonymize(networkId).c_str(), Utility::Anonymize(udId).c_str());
    }
    FI_HILOGI("UdId:%{public}s networkId:%{public}s", Utility::Anonymize(udId).c_str(),
        Utility::Anonymize(networkId).c_str());
    return udId;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
