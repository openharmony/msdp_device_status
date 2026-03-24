/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "intention_client_test.h"

#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "system_ability_definition.h"

#include "devicestatus_define.h"
#include "devicestatus_common.h"
#include "i_context.h"
#include "iremote_on_screen_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };

class MockIRemoteObject : public IRemoteObject {
public:
    MockIRemoteObject() = default;
    ~MockIRemoteObject() override = default;
    
    int32_t GetObjectRefCount() override { return 0; }
    int SendRequest(uint32_t code, MessageParcel &data,
                   MessageParcel &reply, MessageOption &option) override { return 0; }
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) override { return true; }
    int Dump(int fd, const std::vector<std::u16string> &args) override { return 0; }
    bool IsProxyObject() const override { return false; }
};
} // namespace

void IntentionClientTest::SetUpTestCase() {}

void IntentionClientTest::SetUp() {}
void IntentionClientTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

class IRemoteOnScreenCallbackTest : public OnScreen::IRemoteOnScreenCallback {
public:
    void OnScreenChange(const std::string &changeInfo) override{};
    void OnScreenAwareness(const OnScreen::OnscreenAwarenessInfo &info) override{};
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

/**
 * @tc.name: IntentionClientTest1
 * @tc.desc: IntentionClientTest1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    uint64_t displayId = 0;
    uint64_t screenId = 0;
    int32_t ret = env->SetDragWindowScreenId(displayId, screenId);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: IntentionClientTest2
 * @tc.desc: IntentionClientTest2
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest2, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    uint64_t displayId = UINT64_MAX;
    uint64_t screenId = 0;
    int32_t ret = env->SetDragWindowScreenId(displayId, screenId);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: IntentionClientTest3
 * @tc.desc: IntentionClientTest3
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest3, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    uint64_t displayId = 0;
    uint64_t screenId = UINT64_MAX;
    int32_t ret = env->SetDragWindowScreenId(displayId, screenId);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: IntentionClientTest4
 * @tc.desc: IntentionClientTest4
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest4, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    uint64_t displayId = UINT64_MAX;
    uint64_t screenId = UINT64_MAX;
    int32_t ret = env->SetDragWindowScreenId(displayId, screenId);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: IntentionClientTest5
 * @tc.desc: IntentionClientTest5
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest5, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    OnScreen::AwarenessCap cap;
    sptr<IRemoteOnScreenCallbackTest> callback = new (std::nothrow) IRemoteOnScreenCallbackTest();
    EXPECT_NE(callback, nullptr);
    OnScreen::AwarenessOptions option;
    int32_t ret = env->RegisterAwarenessCallback(cap, callback, option);
    ASSERT_NE(ret, RET_OK);
}
 
/**
 * @tc.name: IntentionClientTest6
 * @tc.desc: IntentionClientTest6
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest6, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    OnScreen::AwarenessCap cap;
    sptr<IRemoteOnScreenCallbackTest> callback = new (std::nothrow) IRemoteOnScreenCallbackTest();
    EXPECT_NE(callback, nullptr);
    int32_t ret = env->UnregisterAwarenessCallback(cap, callback);
    ASSERT_NE(ret, RET_OK);
}
 
/**
 * @tc.name: IntentionClientTest7
 * @tc.desc: IntentionClientTest7
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest7, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    OnScreen::AwarenessCap cap;
    OnScreen::AwarenessOptions option;
    OnScreen::OnscreenAwarenessInfo info;
    int32_t ret = env->Trigger(cap, option, info);
    ASSERT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionClientTest_ServiceStatusListener_001
 * @tc.desc: Test ServiceStatusListener creation and SA subscription
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ServiceStatusListener_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = nullptr;
    EXPECT_EQ(env->statusListener_, nullptr);
    sptr<MockIRemoteObject> remote = new (std::nothrow) MockIRemoteObject();
    EXPECT_NE(remote, nullptr);
    wptr<IRemoteObject> weakRemote(remote);
    env->deathRecipient_->OnRemoteDied(weakRemote);
    EXPECT_NE(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_ServiceStatusListener_002
 * @tc.desc: Test no duplicate subscription on multiple service deaths
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ServiceStatusListener_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = nullptr;
    sptr<MockIRemoteObject> remote1 = new (std::nothrow) MockIRemoteObject();
    EXPECT_NE(remote1, nullptr);
    wptr<IRemoteObject> weakRemote1(remote1);
    env->deathRecipient_->OnRemoteDied(weakRemote1);
    auto firstListener = env->statusListener_;
    EXPECT_NE(firstListener, nullptr);
    sptr<MockIRemoteObject> remote2 = new (std::nothrow) MockIRemoteObject();
    EXPECT_NE(remote2, nullptr);
    wptr<IRemoteObject> weakRemote2(remote2);
    env->deathRecipient_->OnRemoteDied(weakRemote2);
    EXPECT_EQ(env->statusListener_, firstListener);
}

/**
 * @tc.name: IntentionClientTest_ServiceStatusListener_003
 * @tc.desc: Test unsubscribe when SA comes online
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ServiceStatusListener_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = sptr<IntentionClient::ServiceStatusListener>::MakeSptr(env->shared_from_this());
    EXPECT_NE(env->statusListener_, nullptr);
    env->statusListener_->OnAddSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, "");
    EXPECT_EQ(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_ServiceStatusListener_004
 * @tc.desc: Test OnAddSystemAbility with wrong SA ID
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ServiceStatusListener_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = sptr<IntentionClient::ServiceStatusListener>::MakeSptr(env->shared_from_this());
    auto originalListener = env->statusListener_;
    EXPECT_NE(originalListener, nullptr);
    env->statusListener_->OnAddSystemAbility(9999, "");
    EXPECT_EQ(env->statusListener_, originalListener);
}

/**
 * @tc.name: IntentionClientTest_ServiceStatusListener_005
 * @tc.desc: Test OnRemoveSystemAbility callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ServiceStatusListener_005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = sptr<IntentionClient::ServiceStatusListener>::MakeSptr(env->shared_from_this());
    EXPECT_NE(env->statusListener_, nullptr);
    env->statusListener_->OnRemoveSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, "");
    EXPECT_NE(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_SubscribeSaListener_001
 * @tc.desc: Test SubscribeSaListener when statusListener is nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_SubscribeSaListener_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = nullptr;
    EXPECT_EQ(env->statusListener_, nullptr);
    env->SubscribeSaListener();
    EXPECT_NE(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_SubscribeSaListener_002
 * @tc.desc: Test SubscribeSaListener when statusListener already exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_SubscribeSaListener_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = sptr<IntentionClient::ServiceStatusListener>::MakeSptr(env->shared_from_this());
    auto firstListener = env->statusListener_;
    EXPECT_NE(firstListener, nullptr);
    env->SubscribeSaListener();
    EXPECT_EQ(env->statusListener_, firstListener);
}

/**
 * @tc.name: IntentionClientTest_UnsubscribeSaListener_001
 * @tc.desc: Test UnsubscribeSaListener when statusListener exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_UnsubscribeSaListener_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = sptr<IntentionClient::ServiceStatusListener>::MakeSptr(env->shared_from_this());
    EXPECT_NE(env->statusListener_, nullptr);
    env->UnsubscribeSaListener();
    EXPECT_EQ(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_UnsubscribeSaListener_002
 * @tc.desc: Test UnsubscribeSaListener when statusListener is nullptr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_UnsubscribeSaListener_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    env->statusListener_ = nullptr;
    EXPECT_EQ(env->statusListener_, nullptr);
    env->UnsubscribeSaListener();
    EXPECT_EQ(env->statusListener_, nullptr);
}

/**
 * @tc.name: IntentionClientTest_ResetDragWindowScreenId_001
 * @tc.desc: Test ResetDragWindowScreenId with valid parameters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ResetDragWindowScreenId_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    uint64_t displayId = 1;
    uint64_t screenId = 2;
    env->ResetDragWindowScreenId(displayId, screenId);
}

/**
 * @tc.name: IntentionClientTest_ResetDragWindowScreenId_002
 * @tc.desc: Test ResetDragWindowScreenId with UINT64_MAX parameters
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionClientTest, IntentionClientTest_ResetDragWindowScreenId_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = IntentionClient::GetInstance();
    ASSERT_NE(env, nullptr);
    uint64_t displayId = UINT64_MAX;
    uint64_t screenId = UINT64_MAX;
    env->ResetDragWindowScreenId(displayId, screenId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
