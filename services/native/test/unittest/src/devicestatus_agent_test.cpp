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

#include "devicestatus_agent_test.h"

#include "devicestatus_common.h"

using namespace testing::ext;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;
using namespace std;

static std::shared_ptr<DeviceStatusAgent> agent1_;
static std::shared_ptr<DeviceStatusAgent> agent2_;

void DeviceStatusAgentTest::SetUpTestCase()
{
}

void DeviceStatusAgentTest::TearDownTestCase()
{
}

void DeviceStatusAgentTest::SetUp()
{
    agent1_ = std::make_shared<DeviceStatusAgent>();
    agent2_ = std::make_shared<DeviceStatusAgent>();
}

void DeviceStatusAgentTest::TearDown()
{
}

bool DeviceStatusAgentListenerMockFirstClient::OnEventResult(
    const DeviceStatusDataUtils::DeviceStatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    return true;
}

bool DeviceStatusAgentListenerMockSecondClient::OnEventResult(
    const DeviceStatusDataUtils::DeviceStatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    return true;
}

namespace {
/**
 * @tc.name: DeviceStatusAgentTest001
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest001 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest001 end";
}

/**
 * @tc.name: DeviceStatusAgentTest002
 * @tc.desc: test subscribing lid open event repeatedly
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest002 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will not report";
    sleep(2);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report again";
    sleep(2);
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest002 end";
}

/**
 * @tc.name: DeviceStatusAgentTest003
 * @tc.desc: test subscribing lid open event for 2 client
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest003 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockSecondClient> agentEvent2 =
        std::make_shared<DeviceStatusAgentListenerMockSecondClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent1);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent2_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent2);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    GTEST_LOG_(INFO) << "Unsubscribe agentEvent1";
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    sleep(2);
    GTEST_LOG_(INFO) << "Unsubscribe agentEvent2";
    agent2_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest003 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest004 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest004 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest005 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest005 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest006 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest006 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest007 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest007 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest008 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest008 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest009 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest009 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest010 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest010 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest011 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnsubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest011 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest012 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_CAR_BLUETOOTH, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnsubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_FINE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest012 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest013 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnsubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnsubscribeAgentEvent(static_cast<DeviceStatusDataUtils::DeviceStatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest013 end";
}

HWTEST_F (DeviceStatusAgentTest, DeviceStatusAgentTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest014 start";
    std::shared_ptr<DeviceStatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DeviceStatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);

    sptr<IdevicestatusCallback> callback = new DeviceStatusAgent::DeviceStatusAgentCallback(agent1_);
    DeviceStatusDataUtils::DeviceStatusData devicestatusData = {
        DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN,
        DeviceStatusDataUtils::DeviceStatusValue::VALUE_ENTER
    };
    callback->OnDeviceStatusChanged(devicestatusData);
    GTEST_LOG_(INFO) << "DeviceStatusAgentTest014 end";
}
}
