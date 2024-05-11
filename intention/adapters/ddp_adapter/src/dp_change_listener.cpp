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

#include "dp_change_listener.h"

#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "DeviceProfileChangeListener"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
using namespace OHOS::DistributedDeviceProfile;
} // namespace

DeviceProfileChangeListener::~DeviceProfileChangeListener()
{
    FI_HILOGW("Destructor");
}

int32_t DeviceProfileChangeListener::OnTrustDeviceProfileAdd(const TrustDeviceProfile &profile)
{
    FI_HILOGW("OnTrustDeviceProfileAdd");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnTrustDeviceProfileDelete(const TrustDeviceProfile &profile)
{
    FI_HILOGW("OnTrustDeviceProfileDelete");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnTrustDeviceProfileUpdate(
    const TrustDeviceProfile &oldProfile, const TrustDeviceProfile &newProfile)
{
    FI_HILOGW("OnTrustDeviceProfileUpdate");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnDeviceProfileAdd(const DeviceProfile &profile)
{
    FI_HILOGW("OnDeviceProfileAdd deviceId:%{public}s", Utility::Anonymize(profile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnDeviceProfileDelete(const DeviceProfile &profile)
{
    FI_HILOGW("OnDeviceProfileDelete, deviceId:%{public}s", Utility::Anonymize(profile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnDeviceProfileUpdate(const DeviceProfile &oldProfile,
    const DeviceProfile &newProfile)
{
    FI_HILOGW("OnDeviceProfileUpdate, oldDeviceId:%{public}s, newDeviceId:%{public}s",
        Utility::Anonymize(oldProfile.GetDeviceId()).c_str(), Utility::Anonymize(newProfile.GetDeviceId()).c_str());
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnServiceProfileAdd(const ServiceProfile &profile)
{
    FI_HILOGW("OnServiceProfileAdd");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnServiceProfileDelete(const ServiceProfile &profile)
{
    FI_HILOGW("OnServiceProfileDelete");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnServiceProfileUpdate(const ServiceProfile &oldProfile,
    const ServiceProfile& newProfile)
{
    FI_HILOGW("OnServiceProfileUpdate");
    return RET_OK;
}

int32_t DeviceProfileChangeListener::OnCharacteristicProfileAdd(
    const CharacteristicProfile &profile)
{
    return OnProfileChanged(profile);
}

int32_t DeviceProfileChangeListener::OnCharacteristicProfileDelete(
    const CharacteristicProfile &profile)
{
    return OnProfileChanged(profile);
}

int32_t DeviceProfileChangeListener::OnCharacteristicProfileUpdate(
    const CharacteristicProfile &oldProfile, const CharacteristicProfile &newProfile)
{
    return OnProfileChanged(newProfile);
}

int32_t DeviceProfileChangeListener::OnProfileChanged(const CharacteristicProfile &profile)
{
    CALL_INFO_TRACE;
    auto udId = profile.GetDeviceId();
    std::shared_ptr<IDDPAdapter> ddp = ddp_.lock();
    std::string networkId = ddp->GetNetworkIdByUdId(udId);
    if (networkId.empty()) {
        FI_HILOGE("NetworkId is empty");
        return RET_ERR;
    }
    ddp->OnProfileChanged(networkId);
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
