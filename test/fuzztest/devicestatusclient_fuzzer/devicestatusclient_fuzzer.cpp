/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "devicestatusclient_fuzzer.h"

#include <stddef.h>
#include <stdint.h>

using namespace std;
using namespace OHOS;
using namespace OHOS::Msdp;
auto &client_ = DevicestatusClient::GetInstance();
sptr<DevicestatusClientFuzzer::DevicestatusTestCallback> cb = new DevicestatusClientFuzzer::DevicestatusTestCallback();
const int WAIT_TIME = 1000;
void DevicestatusClientFuzzer::DevicestatusTestCallback::OnDevicestatusChanged(const \
    DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    std::cout << "DevicestatusTestCallback type: " << devicestatusData.type << std::endl;
    std::cout << "DevicestatusTestCallback value: " << devicestatusData.value << std::endl;
}

void DevicestatusClientFuzzer::TestSubscribeCallback(const uint8_t* data)
{
    std::cout << "TestSubscribeCallback: Enter " << std::endl;
    
    client_.SubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, cb);

    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestGetDevicestatusData();
}

void DevicestatusClientFuzzer::TestGetDevicestatusData()
{
    std::cout << "TestGetDevicestatusData: Enter " << std::endl;
    client_.GetDevicestatusData(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_TIME));
    TestUnSubscribeCallback();
}

void DevicestatusClientFuzzer::TestUnSubscribeCallback()
{
    std::cout << "TestUnSubscribeCallback: Enter " << std::endl;
    
    client_.UnSubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, cb);
}

bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    DevicestatusClientFuzzer::TestSubscribeCallback(data);
    return true;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
