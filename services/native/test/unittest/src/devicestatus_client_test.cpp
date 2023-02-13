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

#include "devicestatus_client_test.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "devicestatus_common.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS;
using namespace std;

static std::shared_ptr<DevicestatusCallbackProxy> callbackProxy_;
static std::shared_ptr<DeviceStatusAgent> agent_;

void DevicestatusClientTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase start";
    agent_ = std::make_shared<DeviceStatusAgent>();
    GTEST_LOG_(INFO) << "SetUpTestCase end";
}

void DevicestatusClientTest::TearDownTestCase()
{
}

void DevicestatusClientTest::SetUp()
{
}

void DevicestatusClientTest::TearDown()
{
}

namespace {
/**
 * @tc.name: DevicestatusClientTest001
 * @tc.desc: test OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest001 start";
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        GTEST_LOG_(INFO) << "GetSystemAbilityManager failed";
        return;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        GTEST_LOG_(INFO) << "CheckSystemAbility failed";
        return;
    }
    auto& client = DevicestatusClient::GetInstance();
    DEV_HILOGE(INNERKIT, "test OnRemoteDied start");
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ =
        sptr<IRemoteObject::DeathRecipient>(new DevicestatusClient::DevicestatusDeathRecipient());
    deathRecipient_->OnRemoteDied(remoteObject_);

    DEV_HILOGE(INNERKIT, "test OnRemoteDied end");
    EXPECT_TRUE(client.devicestatusProxy_ == nullptr);
    GTEST_LOG_(INFO) << "DevicestatusClientTest001 end";
}

/**
 * @tc.name: DevicestatusClientTest002
 * @tc.desc: test OnDevicestatusChanged
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest002 start";
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        GTEST_LOG_(INFO) << "GetSystemAbilityManager failed";
        return;
    }
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    if (remoteObject_ == nullptr) {
        GTEST_LOG_(INFO) << "CheckSystemAbility failed";
        return;
    }
    callbackProxy_ = std::make_shared<DevicestatusCallbackProxy>(remoteObject_);
    DevicestatusDataUtils::DevicestatusData devicestatusData = {
        DevicestatusDataUtils::DevicestatusType::TYPE_STILL,
        DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER
    };
    DEV_HILOGE(INNERKIT, "test OnDevicestatusChanged start");
    callbackProxy_->OnDevicestatusChanged(devicestatusData);
    DEV_HILOGE(INNERKIT, "test OnDevicestatusChanged end");
    GTEST_LOG_(INFO) << "DevicestatusClientTest002 end";
}

/**
 * @tc.name: DevicestatusClientTest003
 * @tc.desc: test null proxy subscribe
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest003 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DevicestatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test SubscribeCallback start");
    client.SubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test SubscribeCallback end");
    GTEST_LOG_(INFO) << "DevicestatusClientTest003 end";
}

/**
 * @tc.name: DevicestatusClientTest004
 * @tc.desc: test null proxy unsubscribe
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest004 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DevicestatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test UnSubscribeCallback start");
    client.UnSubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test UnSubscribeCallback end");
    GTEST_LOG_(INFO) << "DevicestatusClientTest004 end";
}

/**
 * @tc.name: DevicestatusClientTest005
 * @tc.desc: test null proxy unsubscribe
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest005 start";
    auto& client = DevicestatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test GetDevicestatusData start");
    auto data = client.GetDevicestatusData(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN);
    DEV_HILOGE(INNERKIT, "test GetDevicestatusData end");
    EXPECT_TRUE(data.type == DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN && \
        data.value == DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) << "GetDevicestatusData failed";
    GTEST_LOG_(INFO) << "DevicestatusClientTest005 end";
}

/**
 * @tc.name: DevicestatusClientTest006
 * @tc.desc: test null proxy destructor
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest006 start";
    DevicestatusClient* client1 = new(std::nothrow) DevicestatusClient();
    delete client1;
    client1 = nullptr;

    DevicestatusClient* client2 = new(std::nothrow) DevicestatusClient();
    client2->devicestatusProxy_ = nullptr;
    delete client2;
    client2 = nullptr;
    GTEST_LOG_(INFO) << "DevicestatusClientTest006 end";
}

/**
 * @tc.name: DevicestatusClientTest007
 * @tc.desc: test null OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest007 start";
    DEV_HILOGE(INNERKIT, "test OnRemoteDied start");
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ =
        sptr<IRemoteObject::DeathRecipient>(new DevicestatusClient::DevicestatusDeathRecipient());
    deathRecipient_->OnRemoteDied(nullptr);
    DEV_HILOGE(INNERKIT, "test OnRemoteDied end");
    auto& client = DevicestatusClient::GetInstance();
    EXPECT_TRUE(client.devicestatusProxy_ != nullptr);
    GTEST_LOG_(INFO) << "DevicestatusClientTest007 end";
}

/**
 * @tc.name: DevicestatusClientTest008
 * @tc.desc: test proxy double subscribe
 * @tc.type: FUNC
 */
HWTEST_F (DevicestatusClientTest, DevicestatusClientTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DevicestatusClientTest008 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new (std::nothrow) DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DevicestatusClient::GetInstance();
    DEV_HILOGE(INNERKIT, "test SubscribeCallback start");
    client.SubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, callback);
    client.SubscribeCallback(DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test SubscribeCallback end");
    GTEST_LOG_(INFO) << "DevicestatusClientTest008 end";
}
}
