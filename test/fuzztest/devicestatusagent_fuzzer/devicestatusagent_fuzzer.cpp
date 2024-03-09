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

#include "devicestatusagent_fuzzer.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "DeviceStatusAgentFuzzTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t WAIT_TIME { 1000 };
} // namespace

static std::shared_ptr<DevicestatusAgentFuzzer::DeviceStatusAgentClient> agentEvent_ =
            std::make_shared<DevicestatusAgentFuzzer::DeviceStatusAgentClient>();
static std::shared_ptr<DeviceStatusAgent> agent_ = std::make_shared<DeviceStatusAgent>();

bool DevicestatusAgentFuzzer::DeviceStatusAgentClient::OnEventResult(
    const Data& devicestatusData)
{
    std::cout << "type: " << devicestatusData.type << std::endl;
    std::cout << "value: " << devicestatusData.value << std::endl;
    return true;
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEvent(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEvent: Enter" << std::endl;
    int32_t type[1] { -1 };
    int32_t idSize = 4;
    errno_t ret = memcpy_s(type, sizeof(type), data, idSize);
    if (ret != EOK) {
        FI_HILOGE("memcpy_s failed");
        return;
    }

    agent_->SubscribeAgentEvent(static_cast<Type>(type[0]), ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent_);

    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEvent(static_cast<Type>(type[0]));
}

void DevicestatusAgentFuzzer::TestUnSubscribeAgentEvent(Type type)
{
    std::cout << "TestUnSubscribeAgentEvent: Enter" << std::endl;

    agent_->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEventIsNullptr(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEventIsNullptr: Enter" << std::endl;
    int32_t type[1] { -1 };
    int32_t idSize = 4;
    errno_t ret = memcpy_s(type, sizeof(type), data, idSize);
    if (ret != EOK) {
        FI_HILOGE("memcpy_s failed");
        return;
    }
    agentEvent_ = nullptr;

    agent_->SubscribeAgentEvent(static_cast<Type>(type[0]), ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent_);

    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEvent(static_cast<Type>(type[0]));
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEventTypeIsNullptr(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEventTypeIsNullptr: Enter" << std::endl;
    int32_t type[1];
    int32_t idSize = 4;
    errno_t ret = memcpy_s(type, sizeof(type), data, idSize);
    if (ret != EOK) {
        FI_HILOGE("memcpy_s failed");
        return;
    }

    agent_->SubscribeAgentEvent(static_cast<Type>(type[0]), ActivityEvent::ENTER_EXIT,
        ReportLatencyNs::LONG, agentEvent_);

    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEventTypeIsNullptr(static_cast<Type>(type[0]));
}

void DevicestatusAgentFuzzer::TestUnSubscribeAgentEventTypeIsNullptr(Type type)
{
    std::cout << "TestUnSubscribeAgentEventTypeIsNullptr: Enter" << std::endl;

    agent_->UnsubscribeAgentEvent(type, ActivityEvent::ENTER_EXIT);
}

bool DevicestatusAgentFuzzer::DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    size_t idSize = 8;
    if (size > idSize) {
        DevicestatusAgentFuzzer::TestSubscribeAgentEvent(data);
        DevicestatusAgentFuzzer::TestSubscribeAgentEventIsNullptr(data);
        DevicestatusAgentFuzzer::TestSubscribeAgentEventTypeIsNullptr(data);
    }
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Msdp::DeviceStatus::DevicestatusAgentFuzzer::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS