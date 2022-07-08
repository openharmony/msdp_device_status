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

#include "devicestatus_service_test.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <string_ex.h>

#include "devicestatus_common.h"
#include "devicestatus_client.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS;
using namespace std;

void DevicestatusServiceTest::DevicestatusServiceTestCallback::OnDevicestatusChanged(const \
    DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "DevicestatusServiceTestCallback type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "DevicestatusServiceTestCallback value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL && \
        devicestatusData.value == DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER) << \
        "DevicestatusServiceTestCallback failed";
}

/**
 * @tc.name: DevicestatusCallbackTest
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusServiceTest, DevicestatusCallbackTest, TestSize.Level0)
{
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    sptr<IdevicestatusCallback> cb = new DevicestatusServiceTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    devicestatusClient.SubscribeCallback(type, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    devicestatusClient.UnSubscribeCallback(type, cb);
}

/**
 * @tc.name: GetDevicestatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest001, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest001 Enter");
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusData failed";
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest001 end");
}

/**
 * @tc.name: GetDevicestatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest002, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest002 Enter");
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusData failed";
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest002 end");
}

/**
 * @tc.name: GetDevicestatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest003, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest003 Enter");
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusData failed";
    DEV_HILOGI(SERVICE, "GetDevicestatusDataTest003 end");
}

HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest004 Enter";
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusDataTest004 failed";
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest004 end";
}

HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest005 Enter";
    DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType::TYPE_INVALID;
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_INVALID && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusDataTest005 failed";
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest005 end";
}

HWTEST_F (DevicestatusServiceTest, GetDevicestatusDataTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest006 Enter";
    DevicestatusDataUtils::DevicestatusType type = static_cast<DevicestatusDataUtils::DevicestatusType>(10);
    auto& devicestatusClient = DevicestatusClient::GetInstance();
    DevicestatusDataUtils::DevicestatusData data = devicestatusClient.GetDevicestatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_INVALID && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusDataTest006 failed";
    GTEST_LOG_(INFO) << "GetDevicestatusDataTest006 end";
}