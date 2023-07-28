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

#include "fusion_device_profile.h"

#include "devicestatus_define.h"

namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, ::OHOS::Msdp::MSDP_DOMAIN_ID, "FusionDeviceProfile" };
} // namespace

int32_t PutDeviceProfile(const CServiceCharacteristicProfile *profile)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t GetDeviceProfile(const char *udId, const char *serviceId, CServiceCharacteristicProfile *profile)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t SubscribeProfileEvents(const CSubscribeInfos *subscribeInfos,
                               CIProfileEventCb *eventCb,
                               CIProfileEvents **failedEvents)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t UnsubscribeProfileEvents(const CIProfileEvents *profileEvents,
                                 CIProfileEventCb *eventCb,
                                 CIProfileEvents **failedEvents)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t SyncDeviceProfile(const CSyncOptions *syncOptions, CIProfileEventCb *syncCb)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}
