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

#include "devicestatus_module_test.h"

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
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;
using namespace std;

void DeviceStatusModuleTest::DeviceStatusModuleTestCallback::OnDeviceStatusChanged(const \
    DeviceStatusDataUtils::DeviceStatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "DeviceStatusModuleTestCallback type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "DeviceStatusModuleTestCallback value: " << devicestatusData.value;
    EXPECT_EQ(true, devicestatusData.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL && \
        devicestatusData.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_ENTER) << \
        "DeviceStatusModuleTestCallback failed";
}

/**
 * @tc.name: DeviceStatusCallbackTest
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusModuleTest, DeviceStatusCallbackTest, TestSize.Level0)
{
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    sptr<IdevicestatusCallback> cb = new (std::nothrow) DeviceStatusModuleTestCallback();
    EXPECT_FALSE(cb == nullptr);
    GTEST_LOG_(INFO) << "Start register";
    devicestatusClient.SubscribeCallback(type, cb);
    GTEST_LOG_(INFO) << "Cancel register";
    devicestatusClient.UnsubscribeCallback(type, cb);
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusModuleTest, GetDeviceStatusDataTest001, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest001 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_EQ(true, data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusModuleTest, GetDeviceStatusDataTest002, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest002 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_EQ(true, data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusModuleTest, GetDeviceStatusDataTest003, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest003 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_EQ(true, data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
}
