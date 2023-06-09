/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "activatecoordination_fuzzer.h"

#include "coordination_message.h"
#include "interaction_manager.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "ActivateCoordinationFuzzTest" };
} // namespace

void ActivateCoordinationFuzzTest(const uint8_t* data, size_t  size)
{
    CALL_DEBUG_ENTER;
    if (data == nullptr) {
        return;
    }
    const std::string remoteNetworkId(reinterpret_cast<const char*>(data), size);
    const int32_t startDeviceId = *(reinterpret_cast<const int32_t*>(data));
    auto fun = [](const std::string &listener, CoordinationMessage cooperateMessages) {
        FI_HILOGD("Activate coordination fuzz test");
    };
    InteractionManager::GetInstance()->ActivateCoordination(remoteNetworkId, startDeviceId, fun);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int32_t LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < sizeof(int32_t)) {
        return 0;
    }
    OHOS::Msdp::DeviceStatus::ActivateCoordinationFuzzTest(data, size);
    return 0;
}

