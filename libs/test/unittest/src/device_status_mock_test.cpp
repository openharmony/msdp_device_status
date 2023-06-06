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

#include <cstdio>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "devicestatus_data_define.h"
#include "devicestatus_define.h"
#define private public
#include "devicestatus_data_parse.h"
#include "devicestatus_msdp_mock.h"
#undef private
#include "devicestatus_msdp_interface.h"
#include "devicestatus_msdp_mock.h"
#include "devicestatus_msdp_client_impl.h"
#include "sensor_data_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
std::shared_ptr<DeviceStatusMsdpMock> g_testMock;
} // namespace

class DeviceStatusMsdpMoclTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DeviceStatusMsdpMoclTest::SetUpTestCase()
{
    g_testMock = std::make_shared<DeviceStatusMsdpMock>();
}

void DeviceStatusMsdpMoclTest::TearDownTestCase()
{
    g_testMock = nullptr;
}

void DeviceStatusMsdpMoclTest::SetUp() {}

void DeviceStatusMsdpMoclTest::TearDown() {}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest001 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest001 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest002 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_INVALID) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_INVALID) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest002 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest003 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_ABSOLUTE_STILL) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_ABSOLUTE_STILL) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest003 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest004 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_HORIZONTAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_HORIZONTAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest004 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest005 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_VERTICAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_VERTICAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest005 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest006 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_LID_OPEN) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_LID_OPEN) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest006 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest007 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_MAX) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_MAX) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest007 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus DisableCount
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest008 start";
    EXPECT_TRUE(g_testMock->Init());
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_TRUE(g_testMock->Enable(Type::TYPE_HORIZONTAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->Disable(Type::TYPE_HORIZONTAL_POSITION) == ERR_OK);
    EXPECT_TRUE(g_testMock->UnregisterCallback() == ERR_OK);
    EXPECT_TRUE(g_testMock->DisableCount(Type::TYPE_HORIZONTAL_POSITION) == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest008 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus NotifyMsdpImpl
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest009 start";
    EXPECT_FALSE(g_testMock->NotifyMsdpImpl({}) == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest009 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest010 start";
    g_testMock->InitTimer();
    g_testMock->StartThread();
    std::make_unique<std::thread>(&DeviceStatusMsdpMock::LoopingThreadEntry, g_testMock)->detach();
    constexpr int32_t TIMER_INTERVAL = 3;
    int ret = g_testMock->SetTimerInterval(TIMER_INTERVAL);
    g_testMock->CloseTimer();
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest011 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest011 start";
    g_testMock->InitTimer();
    g_testMock->StartThread();
    std::make_unique<std::thread>(&DeviceStatusMsdpMock::LoopingThreadEntry, g_testMock)->detach();
    constexpr int32_t TIMER_INTERVAL = -1;
    int ret = g_testMock->SetTimerInterval(TIMER_INTERVAL);
    g_testMock->CloseTimer();
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest011 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest012 start";
    g_testMock->InitTimer();
    g_testMock->StartThread();
    std::make_unique<std::thread>(&DeviceStatusMsdpMock::LoopingThreadEntry, g_testMock)->detach();
    constexpr int32_t TIMER_INTERVAL = 0;
    int ret = g_testMock->SetTimerInterval(TIMER_INTERVAL);
    g_testMock->CloseTimer();
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest012 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest013 start";
    g_testMock->TimerCallback();
    int ret = g_testMock->GetDeviceStatusData();
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest013 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus NotifyMsdpImpl
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest014 start";
    g_testMock->GetCallbackImpl() = nullptr;
    EXPECT_FALSE(g_testMock->NotifyMsdpImpl({}) == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest014 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus NotifyMsdpImpl
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest015 start";
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    EXPECT_TRUE(g_testMock->RegisterCallback(callback_) == ERR_OK);
    EXPECT_FALSE(g_testMock->NotifyMsdpImpl({TYPE_INVALID, VALUE_INVALID}) == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest015 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus NotifyMsdpImpl
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest016 start";
    g_testMock->dataParse_ = nullptr;
    int32_t ret = g_testMock->GetDeviceStatusData();
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest016 end";
}

/**
 * @tc.name: DeviceStatusMsdpMockTest
 * @tc.desc: test devicestatus Mock in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusMsdpMoclTest, DeviceStatusMsdpMoclTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest017 start";
    constexpr int32_t TIMER_INTERVAL = 0;
    int32_t ret = g_testMock->SetTimerInterval(TIMER_INTERVAL);
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusMsdpMoclTest017 end";
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
