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

#include "fusion_device_profile_adapter.h"

#include "devicestatus_define.h"

using namespace ::OHOS;

namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, Msdp::MSDP_DOMAIN_ID, "FusionDeviceProfile" };
} // namespace

int32_t UpdateCrossSwitchState(int32_t state)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t SyncCrossSwitchState(int32_t state, const CIStringVector *deviceIds)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t GetCrossSwitchState(const char *deviceId)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t RegisterCrossStateListener(const char *deviceId, CICrossStateListener *listener)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t UnregisterCrossStateListener(const char *deviceId)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}
