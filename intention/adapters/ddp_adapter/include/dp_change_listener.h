/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DP_CHANGE_LISTENER
#define DP_CHANGE_LISTENER

#include "profile_change_listener_stub.h"

#include "i_ddp_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::DistributedDeviceProfile;

class DeviceProfileChangeListener : public OHOS::DistributedDeviceProfile::ProfileChangeListenerStub {
    public:
        explicit DeviceProfileChangeListener(std::shared_ptr<IDDPAdapter> ddp) : ddp_(ddp) {}
        ~DeviceProfileChangeListener();
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
    private:
        int32_t OnProfileChanged(const CharacteristicProfile &profile);
    private:
        std::weak_ptr<IDDPAdapter> ddp_;
    };
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DP_CHANGE_LISTENER
