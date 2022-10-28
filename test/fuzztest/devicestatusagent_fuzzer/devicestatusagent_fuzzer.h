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

#ifndef DEVICESTATUSAGENT_FUZZER_H
#define DEVICESTATUSAGENT_FUZZER_H

#include <cstdio>
#include <fcntl.h>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include "securec.h"

#include "devicestatus_agent.h"
#include "devicestatus_data_utils.h"

#define FUZZ_PROJECT_NAME "fault_fuzzer"

namespace OHOS {
namespace Msdp {
enum class ApiNumber {
    NUM_ZERO = 0,
    NUM_ONE,
    NUM_TWO,
    NUM_THREE
};
class DevicestatusAgentFuzzer {
public:
    static bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size);
    static void TestSubscribeAgentEvent(const uint8_t* data);
    static void TestUnSubscribeAgentEvent(const std::shared_ptr<DeviceStatusAgent>& agent);
    static void TestSubscribeAgentEventIsNullptr(const uint8_t* data);
    static void TestSubscribeAgentEventTypeIsNullptr(const uint8_t* data);
    static void TestUnSubscribeAgentEventTypeIsNullptr(const std::shared_ptr<DeviceStatusAgent>& agent_);
    class DeviceStatusAgentClient : public DeviceStatusAgent::DeviceStatusAgentEvent {
    public:
        virtual ~DeviceStatusAgentClient() {};
        bool OnEventResult(const DevicestatusDataUtils::DevicestatusData& devicestatusData) override;
    };
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUSAGENT_FUZZER_H
