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

#include "registercoordinationlistener_fuzzer.h"

#include "coordination_message.h"
#include "interaction_manager.h"
#include "fi_log.h"
#include "i_coordination_listener.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "RegisterCoordinationListenerFuzzTest" };
} // namespace

class CoordinationListenerTest : public ICoordinationListener {
public:
    CoordinationListenerTest() : ICoordinationListener() {}
    void OnCoordinationMessage(const std::string &deviceId, CoordinationMessage msg) override
    {
        FI_HILOGD("Register coordination listener fuzz test");
    };
};

void RegisterCoordinationListenerFuzzTest()
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<CoordinationListenerTest> consumer = std::make_shared<CoordinationListenerTest>();
    InteractionManager::GetInstance()->RegisterCoordinationListener(consumer);
    InteractionManager::GetInstance()->UnregisterCoordinationListener(consumer);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t * data, size_t size)
{
    (void)data;
    (void)size;
    OHOS::Msdp::DeviceStatus::RegisterCoordinationListenerFuzzTest();
    return 0;
}
