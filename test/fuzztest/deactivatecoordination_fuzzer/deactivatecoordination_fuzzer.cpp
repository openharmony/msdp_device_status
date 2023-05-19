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

#include "deactivatecoordination_fuzzer.h"

#include "coordination_message.h"
#include "interaction_manager.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeactivateCoordinationFuzzTest" };
} // namespace

void DeactivateCoordinationFuzzTest()
{
    CALL_DEBUG_ENTER;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Deactivate coordination fuzz test");
    };
    bool isUnchained = false;

    InteractionManager::GetInstance()->DeactivateCoordination(isUnchained, fun);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    (void)data;
    (void)size;
    OHOS::Msdp::DeviceStatus::DeactivateCoordinationFuzzTest();
    return 0;
}
