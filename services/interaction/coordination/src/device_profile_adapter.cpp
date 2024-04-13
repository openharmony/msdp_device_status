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

#include "device_profile_adapter.h"

#include <algorithm>
#include <mutex>

#include "distributed_device_profile_client.h"

#include "coordination_util.h"
#include "devicestatus_define.h"
#include "json_parser.h"

#undef LOG_TAG
#define LOG_TAG "DeviceProfileAdapter"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::DistributedDeviceProfile;
namespace {
const std::string SERVICE_ID { "deviceStatus" };
const std::string SERVICE_TYPE { "deviceStatus" };
const std::string CROSSING_SWITCH_STATE { "crossingSwitchState" };
const std::string CHARACTERISTIC_VALUE { "characteristicValue" };
constexpr int32_t DEVICE_STATUS_SA_ID { 2902 };
constexpr int32_t DP_ERROR_CODE_DATA_EXIST { 98566164 };
} // namespace

DeviceProfileAdapter::DeviceProfileAdapter() {}

DeviceProfileAdapter::~DeviceProfileAdapter() {}

int32_t DeviceProfileAdapter::UpdateCrossingSwitchState(bool state)
{
    CALL_INFO_TRACE;
    if (!serviceProfileExist_) {
        DistributedDeviceProfile::ServiceProfile serviceProfile;
        serviceProfile.SetDeviceId(COORDINATION::GetLocalUdId());
        serviceProfile.SetServiceName(SERVICE_ID);
        serviceProfile.SetServiceType(SERVICE_TYPE);
        int32_t ret = DP_CLIENT.PutServiceProfile(serviceProfile);
        if (ret != RET_OK && ret != DP_ERROR_CODE_DATA_EXIST) {
            FI_HILOGE("PutServiceProfile failed, ret:%{public}d", ret);
            return RET_ERR;
        }
        FI_HILOGI("PutServiceProfile successfully, ret:%{public}d", ret);
        serviceProfileExist_ = true;
    }
    DistributedDeviceProfile::CharacteristicProfile characteristicProfile;
    characteristicProfile.SetDeviceId(COORDINATION::GetLocalUdId());
    characteristicProfile.SetServiceName(SERVICE_ID);
    characteristicProfile.SetCharacteristicKey(CROSSING_SWITCH_STATE);
    characteristicProfile.SetCharacteristicValue((state ? "true" : "false"));
    if (int32_t ret = DP_CLIENT.PutCharacteristicProfile(characteristicProfile); ret != RET_OK) {
        FI_HILOGE("PutCharacteristicProfile failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    FI_HILOGI("UpdateCrossingSwitchState successfully, state:%{public}s", state ? "true" : "false");
    return RET_OK;
}

int32_t DeviceProfileAdapter::GetCrossingSwitchState(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
    DistributedDeviceProfile::CharacteristicProfile profile;
    if (int32_t ret = DP_CLIENT.GetCharacteristicProfile(udId, SERVICE_ID, CROSSING_SWITCH_STATE, profile);
        ret != RET_OK) {
        FI_HILOGE("GetCharacteristicProfile failed, ret:%{public}d, udId:%{public}s",
            ret, GetAnonyString(udId).c_str());
        return RET_ERR;
    }
    state = (profile.GetCharacteristicValue() == "true" ? true : false);
    FI_HILOGD("GetCrossingSwitchState for udId:%{public}s successfully, state:%{public}s",
        GetAnonyString(udId).c_str(),  state ? "true" : "false");
    return RET_OK;
}

int32_t DeviceProfileAdapter::RegisterCrossingStateListener(const std::string &networkId, DPCallback callback)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(adapterLock_);
    OnDeviceOnline(networkId);
    if (RegisterProfileListener(networkId, callback) != RET_OK) {
        FI_HILOGE("RegisterProfileListener failed, networkId:%{public}s", GetAnonyString(networkId).c_str());
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceProfileAdapter::UnregisterCrossingStateListener(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(adapterLock_);
    OnDeviceOffline(networkId);
    if (UnregisterProfileListener(networkId) != RET_OK) {
        FI_HILOGE("UnregisterProfileListener failed, networkId:%{public}s", GetAnonyString(networkId).c_str());
        return RET_ERR;
    }
    return RET_OK;
}

void DeviceProfileAdapter::OnDeviceOnline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    auto udId = COORDINATION::GetUdIdByNetworkId(networkId);
    FI_HILOGI("OnDeviceOnline, networkId:%{public}s, udId:%{public}s",
        GetAnonyString(networkId).c_str(), GetAnonyString(udId).c_str());
    onlineDevUdId2NetworkId_.emplace(udId, networkId);
}

void DeviceProfileAdapter::OnDeviceOffline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    auto udId = COORDINATION::GetUdIdByNetworkId(networkId);
    FI_HILOGI("OnDeviceOffline, networkId:%{public}s, udId:%{public}s",
        GetAnonyString(networkId).c_str(), GetAnonyString(udId).c_str());
    if (onlineDevUdId2NetworkId_.find(udId) != onlineDevUdId2NetworkId_.end()) {
        onlineDevUdId2NetworkId_.erase(udId);
    }
}

int32_t DeviceProfileAdapter::RegisterProfileListener(const std::string &networkId, DPCallback callback)
{
    CALL_INFO_TRACE;
    SubscribeInfo subscribeInfo;
    subscribeInfo.SetSaId(DEVICE_STATUS_SA_ID);
    std::string udId = COORDINATION::GetUdIdByNetworkId(networkId);
    subscribeInfo.SetSubscribeKey(udId, SERVICE_ID, CROSSING_SWITCH_STATE, CHARACTERISTIC_VALUE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_ADD);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_UPDATE);
    subscribeInfo.AddProfileChangeType(ProfileChangeType::CHAR_PROFILE_DELETE);
    sptr<IProfileChangeListener> subscribeDPChangeListener = new (std::nothrow) SubscribeDPChangeListener;
    CHKPR(subscribeDPChangeListener, RET_ERR);
    subscribeInfo.SetListener(subscribeDPChangeListener);
    if (int32_t ret = DP_CLIENT.SubscribeDeviceProfile(subscribeInfo); ret != RET_OK) {
        FI_HILOGE("SubscribeDeviceProfile failed, ret:%{public}d, udId:%{public}s", ret, GetAnonyString(udId).c_str());
        return RET_ERR;
    }
    CrossingSwitchListener switchListener = {
        .subscribeInfo = subscribeInfo,
        .dpCallback = callback
    };
    crossingSwitchListener_.emplace(networkId, switchListener);
    return RET_OK;
}

int32_t DeviceProfileAdapter::UnregisterProfileListener(const std::string &networkId)
{
    CALL_INFO_TRACE;
    if (crossingSwitchListener_.find(networkId) == crossingSwitchListener_.end()) {
        FI_HILOGE("NetworkId:%{public}s is not founded in crossingSwitchListener", GetAnonyString(networkId).c_str());
        return RET_ERR;
    }
    auto switchListener = crossingSwitchListener_[networkId];
    if (int32_t ret = DP_CLIENT.UnSubscribeDeviceProfile(switchListener.subscribeInfo); ret != RET_OK) {
        FI_HILOGE("UnSubscribeDeviceProfile failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    crossingSwitchListener_.erase(networkId);
    return RET_OK;
}

std::string DeviceProfileAdapter::GetNetworkIdByUdId(const std::string &udId)
{
    if (onlineDevUdId2NetworkId_.find(udId) == onlineDevUdId2NetworkId_.end()) {
        FI_HILOGE("UdId:%{public}s is not founded in onlineDevUdId2NetworkId", GetAnonyString(udId).c_str());
        return {};
    }
    return onlineDevUdId2NetworkId_[udId];
}

int32_t DeviceProfileAdapter::OnProfileChanged(const CharacteristicProfile &profile)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(adapterLock_);
    std::string udId = profile.GetDeviceId();
    std::string networkId = GetNetworkIdByUdId(udId);
    if (networkId.empty()) {
        FI_HILOGE("Invalid networkId");
        return RET_ERR;
    }
    if (crossingSwitchListener_.find(networkId) == crossingSwitchListener_.end()) {
        FI_HILOGE("NetworkId:%{public}s is not founded in crossingSwitchListener", GetAnonyString(networkId).c_str());
        return RET_ERR;
    }
    auto switchListener = crossingSwitchListener_[networkId];
    CHKPR(switchListener.dpCallback, RET_ERR);
    bool state = (profile.GetCharacteristicValue() == "true" ? true : false);
    FI_HILOGI("UdId:%{public}s, networkId:%{public}s, state:%{public}s",
        GetAnonyString(udId).c_str(), GetAnonyString(networkId).c_str(), state ? "true" : "false");
    switchListener.dpCallback(networkId, state);
    return RET_OK;
}

DeviceProfileAdapter::SubscribeDPChangeListener::SubscribeDPChangeListener()
{
    FI_HILOGW("Constructor");
}

DeviceProfileAdapter::SubscribeDPChangeListener::~SubscribeDPChangeListener()
{
    FI_HILOGW("Destructor");
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnTrustDeviceProfileAdd(const TrustDeviceProfile &profile)
{
    FI_HILOGW("OnTrustDeviceProfileAdd");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnTrustDeviceProfileDelete(const TrustDeviceProfile &profile)
{
    FI_HILOGW("OnTrustDeviceProfileDelete");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnTrustDeviceProfileUpdate(
    const TrustDeviceProfile &oldProfile, const TrustDeviceProfile &newProfile)
{
    FI_HILOGW("OnTrustDeviceProfileUpdate");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnDeviceProfileAdd(const DeviceProfile &profile)
{
    FI_HILOGW("OnDeviceProfileAdd deviceId:%{public}s", GetAnonyString(profile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnDeviceProfileDelete(const DeviceProfile &profile)
{
    FI_HILOGW("OnDeviceProfileDelete, deviceId:%{public}s", GetAnonyString(profile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnDeviceProfileUpdate(const DeviceProfile &oldProfile,
    const DeviceProfile &newProfile)
{
    FI_HILOGW("OnDeviceProfileUpdate, oldDeviceId:%{public}s, newDeviceId:%{public}s",
        GetAnonyString(oldProfile.GetDeviceId()).c_str(), GetAnonyString(newProfile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnServiceProfileAdd(const ServiceProfile &profile)
{
    FI_HILOGW("OnServiceProfileAdd");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnServiceProfileDelete(const ServiceProfile &profile)
{
    FI_HILOGW("OnServiceProfileDelete");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnServiceProfileUpdate(const ServiceProfile &oldProfile,
    const ServiceProfile& newProfile)
{
    FI_HILOGW("OnServiceProfileUpdate");
    return RET_OK;
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnCharacteristicProfileAdd(
    const CharacteristicProfile &profile)
{
    CALL_INFO_TRACE;
    return DP_ADAPTER->OnProfileChanged(profile);
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnCharacteristicProfileDelete(
    const CharacteristicProfile &profile)
{
    CALL_INFO_TRACE;
    return DP_ADAPTER->OnProfileChanged(profile);
}

int32_t DeviceProfileAdapter::SubscribeDPChangeListener::OnCharacteristicProfileUpdate(
    const CharacteristicProfile &oldProfile, const CharacteristicProfile &newProfile)
{
    CALL_INFO_TRACE;
    return DP_ADAPTER->OnProfileChanged(newProfile);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
