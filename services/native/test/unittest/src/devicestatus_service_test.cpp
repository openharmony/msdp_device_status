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
#include "devicestatus_dumper.h"
#include "devicestatus_service.h"

using namespace testing::ext;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;
using namespace std;

void DeviceStatusServiceTest::DeviceStatusServiceTestCallback::OnDeviceStatusChanged(const \
    DeviceStatusDataUtils::DeviceStatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "DeviceStatusServiceTestCallback type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "DeviceStatusServiceTestCallback value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL && \
        devicestatusData.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_ENTER) << \
        "DeviceStatusServiceTestCallback failed";
}

/**
 * @tc.name: DeviceStatusCallbackTest
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusServiceTest, DeviceStatusCallbackTest, TestSize.Level0)
{
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    sptr<IdevicestatusCallback> cb = new (std::nothrow) DeviceStatusServiceTestCallback();
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
HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest001, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest001 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest001 end");
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest002, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest002 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest002 end");
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest003, TestSize.Level0)
{
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest003 Enter");
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
    DEV_HILOGI(SERVICE, "GetDeviceStatusDataTest003 end");
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest004 Enter";
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusDataTest004 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest004 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest005 Enter";
    DeviceStatusDataUtils::DeviceStatusType type = DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID;
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusDataTest005 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest005 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest006 Enter";
    DeviceStatusDataUtils::DeviceStatusType type = static_cast<DeviceStatusDataUtils::DeviceStatusType>(10);
    auto& devicestatusClient = DeviceStatusClient::GetInstance();
    DeviceStatusDataUtils::DeviceStatusData data = devicestatusClient.GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusDataTest006 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest006 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest007 Enter";
    auto devicestatusService = DelayedSpSingleton<DeviceStatusService>::GetInstance();
    devicestatusService->OnDump();
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest007 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest008 Enter";
    auto devicestatusService = DelayedSpSingleton<DeviceStatusService>::GetInstance();
    std::vector<std::u16string> args;
    int32_t ret = devicestatusService->Dump(-1, args);
    EXPECT_TRUE(ret == RET_NG);
    ret = devicestatusService->Dump(1, args);
    EXPECT_TRUE(ret == RET_NG);
    args.emplace_back(ARGS_H);
    ret = devicestatusService->Dump(1, args);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest008 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest009 Enter";
    auto devicestatusCBDeathRecipient = new (std::nothrow) DeviceStatusManager::DeviceStatusCallbackDeathRecipient();
    devicestatusCBDeathRecipient->OnRemoteDied(nullptr);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest009 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 Enter";
    auto devicestatusService = DelayedSpSingleton<DeviceStatusService>::GetInstance();
    auto devicestatusManager = std::make_shared<DeviceStatusManager>(devicestatusService);
    Security::AccessToken::AccessTokenID tokenId = static_cast<Security::AccessToken::AccessTokenID>(1);
    std::string packageName;
    devicestatusManager->GetPackageName(tokenId, packageName);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 Enter";
    auto devicestatusService = DelayedSpSingleton<DeviceStatusService>::GetInstance();
    devicestatusService->OnStop();
    devicestatusService->OnStart();
    devicestatusService->OnStart();
    devicestatusService->OnStop();
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 end";
}

HWTEST_F (DeviceStatusServiceTest, GetDeviceStatusDataTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest012 Enter";
    auto devicestatusService = DelayedSpSingleton<DeviceStatusService>::GetInstance();
    auto devicestatusManager = std::make_shared<DeviceStatusManager>(devicestatusService);
    bool result = devicestatusManager->Init();
    EXPECT_TRUE(result);
    int32_t ret = devicestatusManager->LoadAlgorithm(false);
    EXPECT_TRUE(ret == RET_OK);
    ret = devicestatusManager->UnloadAlgorithm(false);
    EXPECT_TRUE(ret == RET_OK);
    DelayedSpSingleton<DeviceStatusService>::DestroyInstance();
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest012 end";
}