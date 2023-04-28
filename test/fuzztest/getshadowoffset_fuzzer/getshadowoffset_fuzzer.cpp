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

#include "getshadowoffset_fuzzer.h"

#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "GetShadowOffsetFuzzTest" };
} // namespace

void GetShadowOffsetFuzzTest()
{
    CALL_DEBUG_ENTER;
    int32_t offsetX = 0;
    int32_t offsetY = 0;
    int32_t width = 0;
    int32_t height = 0;
    InteractionManager::GetInstance()->GetShadowOffset(offsetX, offsetY, width, height);
    FI_HILOGD("offsetX:%{public}d,offsetY:%{public}d,width:%{public}d,height:%{public}d",
        offsetX, offsetY, width, height);
    return;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::Msdp::DeviceStatus::GetShadowOffsetFuzzTest();
    return 0;
}