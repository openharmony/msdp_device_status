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

#ifndef DP_ADAPTER_H
#define DP_ADAPTER_H

#include <list>
#include <mutex>
#include <set>
#include <string>
#include <variant>
#include <vector>

#include "cJSON.h"
#include "distributed_device_profile_client.h"
#include "dp_define.h"
#include "i_device_profile.h"
#include "iprofile_event_callback.h"
#include "nocopyable.h"
#include "service_characteristic_profile.h"
#include "singleton.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::DeviceProfile;
class DeviceProfileAdapter {
    DECLARE_DELAYED_SINGLETON(DeviceProfileAdapter);
    class ProfileEventCallbackImpl final : public DeviceProfile::IProfileEventCallback {
    public:
        void OnProfileChanged(const DeviceProfile::ProfileChangeNotification &changeNotification) override;
    };

public:
    using ProfileEventCallback = std::shared_ptr<DeviceProfile::IProfileEventCallback>;
    using ProfileChangedCallback = std::function<void(const std::string &)>;
    DISALLOW_COPY_AND_MOVE(DeviceProfileAdapter);

    void AddProfileCallback(const std::string &pluginName, ProfileChangedCallback callback);
    void UpdateProfileCallback(const std::string &pluginName, ProfileChangedCallback callback);
    void RemoveProfileCallback(const std::string &pluginName);

    int32_t RegisterProfileListener(const std::string &deviceId);
    int32_t UnRegisterProfileListener(const std::string &deviceId);

    int32_t GetProperty(const std::string &deviceId, const std::string &characteristicsName, ValueType valueType,
        DP_VALUE &dpValue);
    int32_t SetProperty(const DP_VALUE &dpValue, ValueType value,
        const std::string &characteristicsName, const std::vector<std::string> &deviceIds);

private:
    int32_t SyncDeviceFile(const std::vector<std::string> &deviceIds);
    int32_t CreatJsonItem(cJSON* item, const DP_VALUE &dpValue, ValueType valueType);
    int32_t ModifyJsonItem(cJSON* data, const DP_VALUE &dpValue, ValueType valueType,
        const std::string &characteristicsName);
    void PackSubscribeInfos(std::list<SubscribeInfo> &subscribeInfos, const std::string &deviceId);
    int32_t SetProfile(ServiceCharacteristicProfile &profile, const std::string &characteristicsName,
        const DP_VALUE &dpValue, ValueType valueType);
    int32_t GetDPValue(ValueType valueType, cJSON* jsonValue, DP_VALUE &dpValue);

    std::mutex mutex_;
    std::map<std::string, ProfileChangedCallback> profileChangedCallbacks_;
    std::map<std::string, DeviceProfileAdapter::ProfileEventCallback> profileEventCallbacks_;
};
#define DP_ADAPTER OHOS::DelayedSingleton<DeviceProfileAdapter>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DP_ADAPER_H