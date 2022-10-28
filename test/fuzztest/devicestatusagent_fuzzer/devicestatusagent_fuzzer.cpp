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

#include "devicestatusagent_fuzzer.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::Msdp;
namespace {
const int WAIT_TIME = 1000;
}
static std::shared_ptr<DevicestatusAgentFuzzer::DeviceStatusAgentClient> agentEvent_ =
            std::make_shared<DevicestatusAgentFuzzer::DeviceStatusAgentClient>();
static std::shared_ptr<DeviceStatusAgent> agent_ = std::make_shared<DeviceStatusAgent>();

bool DevicestatusAgentFuzzer::DeviceStatusAgentClient::OnEventResult(
    const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    std::cout << "type: " << devicestatusData.type << std::endl;
    std::cout << "value: " << devicestatusData.value << std::endl;
    return true;
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEvent(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEvent: Enter " << std::endl;
    agent_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEvent(agent_);
}

void DevicestatusAgentFuzzer::TestUnSubscribeAgentEvent(const std::shared_ptr<DeviceStatusAgent>& agent_)
{
    std::cout << "TestUnSubscribeAgentEvent: Enter " << std::endl;
    agent_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEventIsNullptr(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEventIsNullptr: Enter " << std::endl;
    agentEvent_ = nullptr;
    agent_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEvent(agent_);
}

void DevicestatusAgentFuzzer::TestSubscribeAgentEventTypeIsNullptr(const uint8_t* data)
{
    std::cout << "TestSubscribeAgentEventTypeIsNullptr: Enter " << std::endl;
    agent_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent_);
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeAgentEventTypeIsNullptr(agent_);
}

void DevicestatusAgentFuzzer::TestUnSubscribeAgentEventTypeIsNullptr(const std::shared_ptr<DeviceStatusAgent>& agent_)
{
    std::cout << "TestUnSubscribeAgentEventTypeIsNullptr: Enter " << std::endl;
    agent_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
}

bool DevicestatusAgentFuzzer::DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    int idSize = 8;
    if (static_cast<int>(size) > idSize) {
        TestSubscribeAgentEvent(data);
        TestSubscribeAgentEventIsNullptr(data);
        TestSubscribeAgentEventTypeIsNullptr(data);
    }
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::Msdp::DevicestatusAgentFuzzer::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
