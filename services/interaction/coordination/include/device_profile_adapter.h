/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef DEVICE_PROFILE_ADAPTER_H
#define DEVICE_PROFILE_ADAPTER_H

#include <functional>
#include <memory>
#include <vector>

#include "coordination_util.h"
#include "dp_subscribe_info.h"
#include "nocopyable.h"
#include "profile_change_listener_stub.h"
#include "singleton.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::DistributedDeviceProfile;
class DeviceProfileAdapter final {
    DECLARE_DELAYED_SINGLETON(DeviceProfileAdapter);
public:
    using DPCallback = std::function<void(const std::string &, bool)>;
    DISALLOW_COPY_AND_MOVE(DeviceProfileAdapter);

    int32_t UpdateCrossingSwitchState(bool state);
    int32_t GetCrossingSwitchState(const std::string &udId, bool &state);
    int32_t RegisterCrossingStateListener(const std::string &networkId, DPCallback callback);
    int32_t UnregisterCrossingStateListener(const std::string &networkId);

private:
    void OnDeviceOnline(const std::string &networkId);
    void OnDeviceOffline(const std::string &networkId);
    int32_t RegisterProfileListener(const std::string &networkId, DPCallback callback);
    int32_t UnregisterProfileListener(const std::string &networkId);
    std::string GetNetworkIdByUdId(const std::string &udId);
    int32_t OnProfileChanged(const CharacteristicProfile &profile);

    struct CrossingSwitchListener {
        SubscribeInfo subscribeInfo;
        DPCallback dpCallback;
    };

private:
    std::mutex adapterLock_;
    std::unordered_map<std::string, CrossingSwitchListener> crossingSwitchListener_;
    std::unordered_map<std::string, std::string> onlineDevUdId2NetworkId_;
    bool serviceProfileExist_ { false };

    class SubscribeDPChangeListener : public OHOS::DistributedDeviceProfile::ProfileChangeListenerStub {
    public:
        SubscribeDPChangeListener();
        ~SubscribeDPChangeListener();
        int32_t OnTrustDeviceProfileAdd(const TrustDeviceProfile &profile);
        int32_t OnTrustDeviceProfileDelete(const TrustDeviceProfile &profile);
        int32_t OnTrustDeviceProfileUpdate(const TrustDeviceProfile &oldProfile, const TrustDeviceProfile &newProfile);
        int32_t OnDeviceProfileAdd(const DeviceProfile &profile);
        int32_t OnDeviceProfileDelete(const DeviceProfile &profile);
        int32_t OnDeviceProfileUpdate(const DeviceProfile &oldProfile, const DeviceProfile &newProfile);
        int32_t OnServiceProfileAdd(const ServiceProfile &profile);
        int32_t OnServiceProfileDelete(const ServiceProfile &profile);
        int32_t OnServiceProfileUpdate(const ServiceProfile &oldProfile, const ServiceProfile &newProfile);
        int32_t OnCharacteristicProfileAdd(const CharacteristicProfile &profile);
        int32_t OnCharacteristicProfileDelete(const CharacteristicProfile &profile);
        int32_t OnCharacteristicProfileUpdate(const CharacteristicProfile &oldProfile,
        const CharacteristicProfile &newProfile);
    };
};
#define DP_ADAPTER OHOS::DelayedSingleton<DeviceProfileAdapter>::GetInstance()
#define DP_CLIENT DistributedDeviceProfile::DistributedDeviceProfileClient::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DEVICE_PROFILE_ADAPTER_H