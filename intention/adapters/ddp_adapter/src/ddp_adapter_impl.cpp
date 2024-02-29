/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "devicestatus_define.h"
#include "json_parser.h"
#include "utility.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DDPAdapterImpl" };
const std::string SERVICE_ID { "deviceStatus" };
const std::string SERVICE_TYPE { "deviceStatus" };
} // namespace

#define DDP_CLIENT  DeviceProfile::DistributedDeviceProfileClient::GetInstance()

void DDPAdapterImpl::ProfileEventCallback::OnProfileChanged(
    const DeviceProfile::ProfileChangeNotification &changeNotification)
{
    CALL_DEBUG_ENTER;
    std::string networkId = changeNotification.GetDeviceId();
    FI_HILOGI("Profile of \'%{public}s\' has changed", Utility::Anonymize(networkId));
    std::shared_ptr<DDPAdapterImpl> ddp = ddp_.lock();
    if (ddp != nullptr) {
        ddp->OnProfileChanged(networkId);
    }
}

void DDPAdapterImpl::OnProfileChanged(const std::string &networkId)
{
    std::lock_guard guard(mutex_);
    FI_HILOGI("Profile of \'%{public}s\' has changed", Utility::Anonymize(networkId));
    for (const auto &item : observers_) {
        std::shared_ptr<IDeviceProfileObserver> observer = item.Lock();
        if (observer != nullptr) {
            FI_HILOGD("Notify profile change: \'%{public}s\'", Utility::Anonymize(networkId));
            observer->OnProfileChanged(networkId);
        }
    }
}

void DDPAdapterImpl::AddObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(mutex_);
    CHKPV(observer);
    observers_.erase(Observer());
    observers_.emplace(observer);
}

void DDPAdapterImpl::RemoveObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(mutex_);
    if (auto iter = observers_.find(Observer(observer)); iter != observers_.end()) {
        observers_.erase(iter);
    }
    observers_.erase(Observer());
}

void DDPAdapterImpl::AddWatch(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(mutex_);
    FI_HILOGD("Add watch \'%{public}s\'", Utility::Anonymize(networkId));
    RegisterProfileListener(networkId);
    siblings_.insert(networkId);
}

void DDPAdapterImpl::RemoveWatch(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(mutex_);
    FI_HILOGD("Remove watch \'%{public}s\'", Utility::Anonymize(networkId));
    siblings_.erase(networkId);
    UnregisterProfileListener(networkId);
}

int32_t DDPAdapterImpl::RegisterProfileListener(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("Register profile listener for \'%{public}s\'", Utility::Anonymize(networkId));
    if (auto iter = profileEventCbs_.find(networkId); iter != profileEventCbs_.end()) {
        FI_HILOGD("Profile listener has been registered for \'%{public}s\'", Utility::Anonymize(networkId));
        return RET_OK;
    }
    std::list<std::string> serviceIds;
    serviceIds.emplace_back(SERVICE_ID);

    DeviceProfile::ExtraInfo extraInfo;
    extraInfo["deviceId"] = networkId;
    extraInfo["serviceIds"] = serviceIds;

    DeviceProfile::SubscribeInfo changeEventInfo;
    changeEventInfo.profileEvent = DeviceProfile::ProfileEvent::EVENT_PROFILE_CHANGED;
    changeEventInfo.extraInfo = std::move(extraInfo);

    std::list<DeviceProfile::SubscribeInfo> subscribeInfos;
    subscribeInfos.emplace_back(changeEventInfo);

    auto profileEventCb = std::make_shared<ProfileEventCallback>(shared_from_this());
    std::list<DeviceProfile::ProfileEvent> failedEvents;
    DDP_CLIENT.SubscribeProfileEvents(subscribeInfos, profileEventCb, failedEvents);

    if (std::any_of(failedEvents.cbegin(), failedEvents.cend(),
        [](DeviceProfile::ProfileEvent event) {
            return (event == DeviceProfile::ProfileEvent::EVENT_PROFILE_CHANGED);
        })) {
        FI_HILOGE("SubscribeProfileEvents(%{public}s) failed", Utility::Anonymize(networkId));
        return RET_ERR;
    }
    profileEventCbs_.emplace(networkId, profileEventCb);
    return RET_OK;
}

void DDPAdapterImpl::UnregisterProfileListener(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("Unregister profile listener for \'%{public}s\'", Utility::Anonymize(networkId));
    auto iter = profileEventCbs_.find(networkId);
    if (iter == profileEventCbs_.end()) {
        FI_HILOGD("No profile listener for \'%{public}s\'", Utility::Anonymize(networkId));
        return;
    }
    std::shared_ptr<ProfileEventCallback> profileEventCb = iter->second;
    profileEventCbs_.erase(iter);

    std::list<DeviceProfile::ProfileEvent> profileEvents;
    profileEvents.emplace_back(DeviceProfile::ProfileEvent::EVENT_PROFILE_CHANGED);

    std::list<DeviceProfile::ProfileEvent> failedEvents;
    DDP_CLIENT.UnsubscribeProfileEvents(profileEvents, profileEventCb, failedEvents);
}

int32_t DDPAdapterImpl::GetProperty(const std::string &networkId, const std::string &name, bool &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(networkId, name, [&name, &value](cJSON *json) {
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

int32_t DDPAdapterImpl::GetProperty(const std::string &networkId, const std::string &name, int32_t &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(networkId, name, [&name, &value](cJSON *json) {
        FI_HILOGD("Get integer property: %{public}s", name.c_str());
        if (!cJSON_IsNumber(json)) {
            FI_HILOGE("Unexpected data type");
            return RET_ERR;
        }
        value = static_cast<int32_t>(json->valuedouble);
        return RET_OK;
    });
}

int32_t DDPAdapterImpl::GetProperty(const std::string &networkId, const std::string &name, std::string &value)
{
    CALL_DEBUG_ENTER;
    return GetProperty(networkId, name, [&name, &value](cJSON *json) {
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

int32_t DDPAdapterImpl::GetProperty(const std::string &networkId, const std::string &name,
    std::function<int32_t(cJSON *json)> parse)
{
    CALL_DEBUG_ENTER;
    DeviceProfile::ServiceCharacteristicProfile profile;
    int32_t ret = DDP_CLIENT.GetDeviceProfile(networkId, SERVICE_ID, profile);
    if (ret != RET_OK) {
        FI_HILOGE("DP::GetDeviceProfile failed");
        return RET_ERR;
    }
    std::string jsonData = profile.GetCharacteristicProfileJson();
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

int32_t DDPAdapterImpl::PutProfile()
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
    std::string sProfile(cProfile);
    cJSON_free(cProfile);

    DeviceProfile::ServiceCharacteristicProfile profile;
    profile.SetServiceId(SERVICE_ID);
    profile.SetServiceType(SERVICE_TYPE);
    profile.SetCharacteristicProfileJson(sProfile);
    int32_t ret = DDP_CLIENT.PutDeviceProfile(profile);
    if (ret != RET_OK) {
        FI_HILOGE("DP::PutDeviceProfile fail");
        return RET_ERR;
    }
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
