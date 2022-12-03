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

#include "devicestatus_client_test.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "devicestatus_common.h"

using namespace testing::ext;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;
using namespace std;

static std::shared_ptr<DeviceStatusCallbackProxy> callbackProxy_;
static std::shared_ptr<DeviceStatusAgent> agent_;

void DeviceStatusClientTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "SetUpTestCase start";
    agent_ = std::make_shared<DeviceStatusAgent>();
    GTEST_LOG_(INFO) << "SetUpTestCase end";
}

void DeviceStatusClientTest::TearDownTestCase()
{
}

void DeviceStatusClientTest::SetUp()
{
}

void DeviceStatusClientTest::TearDown()
{
}

namespace {
/**
 * @tc.name: DeviceStatusClientTest001
 * @tc.desc: test OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest001 start";
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
    auto& client = DeviceStatusClient::GetInstance();
    DEV_HILOGE(INNERKIT, "test OnRemoteDied start");
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ =
        sptr<IRemoteObject::DeathRecipient>(new DeviceStatusClient::DeviceStatusDeathRecipient());
    deathRecipient_->OnRemoteDied(remoteObject_);

    DEV_HILOGE(INNERKIT, "test OnRemoteDied end");
    EXPECT_TRUE(client.devicestatusProxy_ == nullptr);
    GTEST_LOG_(INFO) << "DeviceStatusClientTest001 end";
}

/**
 * @tc.name: DeviceStatusClientTest002
 * @tc.desc: test OnDeviceStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest002 start";
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
    callbackProxy_ = std::make_shared<DeviceStatusCallbackProxy>(remoteObject_);
    DeviceStatusDataUtils::DeviceStatusData devicestatusData = {
        DeviceStatusDataUtils::DeviceStatusType::TYPE_HIGH_STILL,
        DeviceStatusDataUtils::DeviceStatusValue::VALUE_ENTER
    };
    DEV_HILOGE(INNERKIT, "test OnDeviceStatusChanged start");
    callbackProxy_->OnDeviceStatusChanged(devicestatusData);
    DEV_HILOGE(INNERKIT, "test OnDeviceStatusChanged end");
    GTEST_LOG_(INFO) << "DeviceStatusClientTest002 end";
}

/**
 * @tc.name: DeviceStatusClientTest003
 * @tc.desc: test null proxy subscribe
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest003 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DeviceStatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test SubscribeCallback start");
    client.SubscribeCallback(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test SubscribeCallback end");
    GTEST_LOG_(INFO) << "DeviceStatusClientTest003 end";
}

/**
 * @tc.name: DeviceStatusClientTest004
 * @tc.desc: test null proxy unsubscribe
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest004 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DeviceStatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test UnsubscribeCallback start");
    client.UnsubscribeCallback(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test UnsubscribeCallback end");
    GTEST_LOG_(INFO) << "DeviceStatusClientTest004 end";
}

/**
 * @tc.name: DeviceStatusClientTest005
 * @tc.desc: test null proxy unsubscribe
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest005 start";
    auto& client = DeviceStatusClient::GetInstance();
    client.devicestatusProxy_ = nullptr;
    DEV_HILOGE(INNERKIT, "test GetDeviceStatusData start");
    auto data = client.GetDeviceStatusData(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN);
    DEV_HILOGE(INNERKIT, "test GetDeviceStatusData end");
    EXPECT_TRUE(data.type == DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN && \
        data.value == DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) << "GetDeviceStatusData failed";
    GTEST_LOG_(INFO) << "DeviceStatusClientTest005 end";
}

/**
 * @tc.name: DeviceStatusClientTest006
 * @tc.desc: test null proxy destructor
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest006 start";
    DeviceStatusClient* client1 = new(std::nothrow) DeviceStatusClient();
    delete client1;
    client1 = nullptr;

    DeviceStatusClient* client2 = new(std::nothrow) DeviceStatusClient();
    client2->devicestatusProxy_ = nullptr;
    delete client2;
    client2 = nullptr;
    GTEST_LOG_(INFO) << "DeviceStatusClientTest006 end";
}

/**
 * @tc.name: DeviceStatusClientTest007
 * @tc.desc: test null OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest007 start";
    DEV_HILOGE(INNERKIT, "test OnRemoteDied start");
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ =
        sptr<IRemoteObject::DeathRecipient>(new DeviceStatusClient::DeviceStatusDeathRecipient());
    deathRecipient_->OnRemoteDied(nullptr);
    DEV_HILOGE(INNERKIT, "test OnRemoteDied end");
    auto& client = DeviceStatusClient::GetInstance();
    EXPECT_TRUE(client.devicestatusProxy_ != nullptr);
    GTEST_LOG_(INFO) << "DeviceStatusClientTest007 end";
}

/**
 * @tc.name: DeviceStatusClientTest008
 * @tc.desc: test proxy double subscribe
 * @tc.type: FUNC
 */
HWTEST_F (DeviceStatusClientTest, DeviceStatusClientTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTest008 start";
    sptr<IdevicestatusCallback> callback =
        sptr<IdevicestatusCallback>(new (std::nothrow) DeviceStatusAgent::DeviceStatusAgentCallback(agent_));
    auto& client = DeviceStatusClient::GetInstance();
    DEV_HILOGE(INNERKIT, "test SubscribeCallback start");
    client.SubscribeCallback(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, callback);
    client.SubscribeCallback(DeviceStatusDataUtils::DeviceStatusType::TYPE_LID_OPEN, callback);
    DEV_HILOGE(INNERKIT, "test SubscribeCallback end");
    GTEST_LOG_(INFO) << "DeviceStatusClientTest008 end";
}
}
