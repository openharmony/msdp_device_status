/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>

#include "gtest/gtest.h"
#include "ipc_skeleton.h"

#include "cooperate_params.h"
#include "cooperate_server.h"
#include "default_params.h"
#include "fi_log.h"
#include "test_context.h"

#undef LOG_TAG
#define LOG_TAG "CooperateServerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

class CooperateServerTest : public testing::Test {
public:
    CooperateServerTest();
    ~CooperateServerTest() = default;

    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Intention intention_ { Intention::COOPERATE };
    std::shared_ptr<TestContext> context_ { nullptr };
    std::shared_ptr<CooperateServer> cooperateServer_ { nullptr };
};

CooperateServerTest::CooperateServerTest()
{
    context_ = std::make_shared<TestContext>();
    cooperateServer_ = std::make_shared<CooperateServer>(context_.get());
}

void CooperateServerTest::SetUp()
{}

void CooperateServerTest::TearDown()
{}

void CooperateServerTest::SetUpTestCase()
{}

void CooperateServerTest::TearDownTestCase()
{}

/**
 * @tc.name: EnableTest1
 * @tc.desc: Test func named enable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, EnableTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = cooperateServer_->Enable(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DisableTest1
 * @tc.desc: Test func named disable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, DisableTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = cooperateServer_->Disable(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: StartTest1
 * @tc.desc: Test func named start
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, StartTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = cooperateServer_->Start(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: StopTest1
 * @tc.desc: Test func named stop
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, StopTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = cooperateServer_->Stop(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: EnableTest2
 * @tc.desc: Test func named enable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, EnableTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel data;
    MessageParcel reply;
    DefaultParam  param { 1 };
    ASSERT_TRUE(param.Marshalling(data));
    auto ret = cooperateServer_->Enable(context, data, reply);
    EXPECT_EQ(ret, RET_OK);
    context_->GetPluginManager().UnloadCooperate();
}

/**
 * @tc.name: AddWatchTest1
 * @tc.desc: Test func named add watch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, AddWatchTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel data;
    MessageParcel reply;
    DefaultParam  param { 1 };
    ASSERT_TRUE(param.Marshalling(data));
    auto ret = cooperateServer_->AddWatch(context, CooperateRequestID::REGISTER_LISTENER, data, reply);
    EXPECT_EQ(ret, RET_OK);
    context_->GetPluginManager().UnloadCooperate();
}

/**
 * @tc.name: DisableTest2
 * @tc.desc: Test func named disable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateServerTest, DisableTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = intention_,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DefaultParam  param { 1 };
    MessageParcel datas;
    MessageParcel reply;
    param.Marshalling(datas);
    ASSERT_TRUE(param.Marshalling(datas));
    int32_t ret = cooperateServer_->Disable(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    context_->GetPluginManager().UnloadCooperate();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS