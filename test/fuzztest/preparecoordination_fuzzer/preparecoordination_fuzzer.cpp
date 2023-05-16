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

#include "preparecoordination_fuzzer.h"
#include "securec.h"

#include "coordination_message.h"
#include "interaction_manager.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "PrepareCoordinationFuzzTest" };
} // namespace

template<class T>
size_t GetObject(const uint8_t *data, size_t size, T &object)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    errno_t ret = ::memcpy_s(&object, objectSize, data, objectSize);
    if (ret != EOK) {
        return 0;
    }
    return objectSize;
}

void PrepareCoordinationFuzzTest(const uint8_t* data, size_t size)
{
    CALL_DEBUG_ENTER;
    int32_t random = 0;
    (void)GetObject<int32_t>(data, 0, random);
    bool enabled = (random % 2) ? false : true;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Prepare coordination fuzz test");
    };
    if (enabled) {
        InteractionManager::GetInstance()->PrepareCoordination(fun);
    } else {
        InteractionManager::GetInstance()->UnprepareCoordination(fun);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::Msdp::DeviceStatus::PrepareCoordinationFuzzTest(data, size);
    return 0;
}

