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

#include "updatedragstyle_fuzzer.h"

#include "securec.h"

#include "devicestatus_define.h"
#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "UpdateDragStyleFuzzTest" };
constexpr int32_t DRAGCURSORSTYLE_MAX { 4 };
} // namespace

template<class T>
void GetObject(const uint8_t *data, size_t size, T &object)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return;
    }
    errno_t ret = memcpy_s(&object, objectSize, data, objectSize);
    if (ret != EOK) {
        return;
    }
}

void UpdateDragStyleFuzzTest(const uint8_t* data, size_t size)
{
    CALL_DEBUG_ENTER;
    int32_t num = 0;
    GetObject<int32_t>(data, size, num);
    InteractionManager::GetInstance()->UpdateDragStyle(static_cast<DragCursorStyle>(num % DRAGCURSORSTYLE_MAX));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int32_t LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    if (size < sizeof(int32_t)) {
        return 0;
    }
    OHOS::Msdp::DeviceStatus::UpdateDragStyleFuzzTest(data, size);
    return 0;
}

