/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define private public
#include "coordination_softbus_adapter_test.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationSoftbusAdapterTest" };
auto g_adapter = CoordinationSoftbusAdapter::GetInstance();
bool g_sendable { false };
bool g_init { false };
bool g_cond { false };
const std::string LOCAL_NETWORKID { "testLocalNetworkId" };
const std::string REMOTE_NETWORKID { "testRemoteNetworkId" };
const std::string ORIGIN_NETWORKID { "testOriginNetworkId" };
const std::string DEVICE_ID { "testDeviceId" };
constexpr int32_t SESSION_ID { 1 };
constexpr uint32_t INTERCEPT_STRING_LENGTH { 20 };
} // namespace

void ClearCoordinationSoftbusAdapter()
{
    COOR_SOFTBUS_ADAPTER->sessionId_ = -1;
    COOR_SOFTBUS_ADAPTER->localSessionName_ = "";
    COOR_SOFTBUS_ADAPTER->registerRecvs_.clear();
    COOR_SOFTBUS_ADAPTER->sessionDevs_.clear();
    COOR_SOFTBUS_ADAPTER->channelStatuss_.clear();
}

int32_t CoordinationSoftbusAdapter::SendMsg(int32_t sessionId, const std::string &message)
{
    return g_sendable == true ? RET_OK : RET_ERR;
}

int32_t CoordinationSoftbusAdapter::Init()
{
    return g_init == true ? RET_OK : RET_ERR;
}

int32_t CoordinationSoftbusAdapter::WaitSessionOpend(const std::string &remoteNetworkId, int32_t sessionId)
{
    return g_cond == true ? RET_OK : RET_ERR;
}

void CoordinationSoftbusAdapterTest::SetUpTestCase() {}

void CoordinationSoftbusAdapterTest::TearDownTestCase() {}

void CoordinationSoftbusAdapterTest::SetUp() {}

void CoordinationSoftbusAdapterTest::TearDown() {}

/**
 * @tc.name: CoordinationSoftbusAdapterTest001
 * @tc.desc: Test func named StartRemoteCoordination, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest002
 * @tc.desc: Test func named StartRemoteCoordination, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest003
 * @tc.desc: Test func named StartRemoteCoordinationResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, REMOTE_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest004
 * @tc.desc: Test func named StartRemoteCoordinationResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, REMOTE_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, REMOTE_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest005
 * @tc.desc: Test func named StopRemoteCoordination, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest006
 * @tc.desc: Test func named StopRemoteCoordination, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest006, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest007
 * @tc.desc: Test func named StopRemoteCoordinationResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest007, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest008
 * @tc.desc: Test func named StopRemoteCoordinationResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest008, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest009
 * @tc.desc: Test func named StartCoordinationOtherResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest009, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest010
 * @tc.desc: Test func named StartCoordinationOtherResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest010, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[ORIGIN_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest011
 * @tc.desc: Test func named NotifyFilterAdded, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest011, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest012
 * @tc.desc: Test func named NotifyFilterAdded, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest012, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest013
 * @tc.desc: test normal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest013, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = SESSION_ID;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest014
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest014, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_init = false;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    EXPECT_EQ(ret, RET_ERR);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest015
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest015, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_init = true;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    EXPECT_EQ(ret, RET_ERR);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest016
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest016, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_init = true;
    g_cond = false;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    EXPECT_EQ(ret, RET_ERR);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest017
 * @tc.desc: test normal func named OnSessionOpened and OnSessionClosed in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest017, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->OnSessionOpened(SESSION_ID, RET_ERR);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->OnSessionClosed(SESSION_ID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest018
 * @tc.desc: test normal func named OnSessionOpened and OnSessionClosed in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest018, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->OnSessionOpened(SESSION_ID, RET_OK);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->OnSessionClosed(SESSION_ID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest019
 * @tc.desc: test abnormal func named SendData in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest019, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    std::string data = "TestSendData";
    g_adapter->sessionDevs_[DEVICE_ID] = SESSION_ID;
    int32_t ret = g_adapter->SendData(DEVICE_ID, CoordinationSoftbusAdapter::MIN_ID, const_cast<char *>(data.c_str()),
        INTERCEPT_STRING_LENGTH);
    EXPECT_EQ(ret, RET_ERR);
    g_adapter->sessionDevs_.clear();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest020
 * @tc.desc: Test func named NotifyUnchainedResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest020, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    int32_t ret = g_adapter->NotifyUnchainedResult(LOCAL_NETWORKID, REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest021
 * @tc.desc: Test func named NotifyUnchainedResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest021, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_TRUE(g_adapter != nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->NotifyUnchainedResult(LOCAL_NETWORKID, REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->NotifyUnchainedResult(LOCAL_NETWORKID, REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_OK);
    g_adapter->sessionDevs_.clear();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS