/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "getinputdevicecooperatestate_fuzzer.h"

#include "securec.h"

#include "interaction_manager.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "GetInputDeviceCoordinationStateFuzzTest" };
} // namespace

void GetInputDeviceCoordinationStateFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }
    const std::string deviceId(reinterpret_cast<const char*>(data), size);
    auto fun = [](bool inputdevice) {
        FI_HILOGD("Get inputdevice state success");
    };
    InteractionManager::GetInstance()->GetInputDeviceCoordinationState(deviceId, fun);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size)
{
    /* Run your code on data */
    OHOS::Msdp::DeviceStatus::GetInputDeviceCoordinationStateFuzzTest(data, size);
    return 0;
}
