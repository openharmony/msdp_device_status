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

#include <memory>
#include <optional>
#include <utility>

#include "pointer_event.h"
#include "stopdrag_fuzzer.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr bool HAS_CUSTOM_ANIMATION { true };
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StopDragFuzzTest" };
} // namespace

void StopDragFuzzTest(const uint8_t* data, size_t  size)
{
    CALL_DEBUG_ENTER;
    if (data == nullptr) {
        return;
    }
    int32_t result = *(reinterpret_cast<const int32_t*>(data));
    InteractionManager::GetInstance()->StopDrag(static_cast<DragResult>(result), HAS_CUSTOM_ANIMATION);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int32_t LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < sizeof(int32_t)) {
        return 0;
    }
    OHOS::Msdp::DeviceStatus::StopDragFuzzTest(data, size);
    return 0;
}

