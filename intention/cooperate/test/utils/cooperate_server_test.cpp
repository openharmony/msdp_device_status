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


#define private public
#include <gtest/gtest.h>

#include "cooperate_server.h"
#include "cooperate_params.h"
#include "default_params.h"
#include "devicestatus_service.h"
#include "fi_log.h"
#include "ipc_skeleton.h"

#undef LOG_TAG
#define LOG_TAG "CooperateServerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
Intention g_intention { Intention::UNKNOWN_INTENTION };
std::shared_ptr<CooperateServer> g_cooperateServer { nullptr };
IContext *g_context { nullptr };
} // namespace

class CooperateServerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CooperateServerTest::SetUp()
{
    auto deviceStatusService = DelayedSingleton<DeviceStatusService>::GetInstance();
    g_context = dynamic_cast<IContext *>(deviceStatusService.get());
    g_cooperateServer = std::make_shared<CooperateServer>(g_context);
}

void CooperateServerTest::SetUpTestCase() {}

void CooperateServerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

void CooperateServerTest::TearDownTestCase() {}

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
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = g_cooperateServer->Enable(context, datas, reply);
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
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = g_cooperateServer->Disable(context, datas, reply);
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
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = g_cooperateServer->Start(context, datas, reply);
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
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = g_cooperateServer->Stop(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS