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

#include "devicestatus_agent_test.h"

#include "devicestatus_common.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS;
using namespace std;

static std::shared_ptr<DeviceStatusAgent> agent1_;
static std::shared_ptr<DeviceStatusAgent> agent2_;

void DevicestatusAgentTest::SetUpTestCase() {}

void DevicestatusAgentTest::TearDownTestCase() {}

void DevicestatusAgentTest::SetUp()
{
    agent1_ = std::make_shared<DeviceStatusAgent>();
    agent2_ = std::make_shared<DeviceStatusAgent>();
}

void DevicestatusAgentTest::TearDown() {}

bool DevicestatusAgentListenerMockFirstClient::OnEventResult(
    const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type > DevicestatusDataUtils::DevicestatusType::TYPE_INVALID
        && devicestatusData.type < DevicestatusDataUtils::DevicestatusType::TYPE_MAX);
    return true;
}

bool DevicestatusAgentListenerMockSecondClient::OnEventResult(
    const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    GTEST_LOG_(INFO) << "agent type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "agent value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type > DevicestatusDataUtils::DevicestatusType::TYPE_INVALID
        && devicestatusData.type < DevicestatusDataUtils::DevicestatusType::TYPE_MAX);
    return true;
}

namespace {
/**
 * @tc.name: DevicestatusAgentTest001
 * @tc.desc: test subscribing lid open event
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest001 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest001 end";
}

/**
 * @tc.name: DevicestatusAgentTest002
 * @tc.desc: test subscribing lid open event repeatedly
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest002 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will not report";
    sleep(2);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report again";
    sleep(2);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest002 end";
}

/**
 * @tc.name: DevicestatusAgentTest003
 * @tc.desc: test subscribing lid open event for 2 client
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest003 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent1 =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    std::shared_ptr<DevicestatusAgentListenerMockSecondClient> agentEvent2 =
        std::make_shared<DevicestatusAgentListenerMockSecondClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent1);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent2_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent2);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    GTEST_LOG_(INFO) << "UnSubscribe agentEvent1";
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    sleep(2);
    GTEST_LOG_(INFO) << "UnSubscribe agentEvent2";
    agent2_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest003 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest004 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest004 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest005 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest005 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest006 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest006 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest007 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest007 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest008 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest008 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest009 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest009 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest010 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    GTEST_LOG_(INFO) << "DevicestatusAgentTest010 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest011 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnSubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest011 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest012 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->SubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnSubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest012 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest013 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent1 = nullptr;
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID, agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10), agentEvent);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL, agentEvent1);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "Open and close the lid, and event will report";
    sleep(2);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_INVALID);
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    ret = agent1_->UnSubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
    EXPECT_TRUE(ret == ERR_OK);
    ret = agent1_->UnSubscribeAgentEvent(static_cast<DevicestatusDataUtils::DevicestatusType>(10));
    EXPECT_TRUE(ret == ERR_INVALID_VALUE);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest013 end";
}

HWTEST_F (DevicestatusAgentTest, DevicestatusAgentTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusAgentTest014 start";
    std::shared_ptr<DevicestatusAgentListenerMockFirstClient> agentEvent =
        std::make_shared<DevicestatusAgentListenerMockFirstClient>();
    int32_t ret = agent1_->SubscribeAgentEvent(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, agentEvent);
    EXPECT_TRUE(ret == ERR_OK);

    sptr<IdevicestatusCallback> callback = new DeviceStatusAgent::DeviceStatusAgentCallback(agent1_);
    DevicestatusDataUtils::DevicestatusData devicestatusData = {
        DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN,
        DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER
    };
    callback->OnDevicestatusChanged(devicestatusData);
    GTEST_LOG_(INFO) << "DevicestatusAgentTest014 end";
}
}
