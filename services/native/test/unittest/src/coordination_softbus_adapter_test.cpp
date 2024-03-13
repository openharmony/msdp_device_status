/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "parcel.h"
#include "coordination_softbus_adapter_test.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationSoftbusAdapterTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
auto g_adapter = CoordinationSoftbusAdapter::GetInstance();
bool g_sendable { false };
bool g_init { false };
bool g_cond { false };
const std::string LOCAL_NETWORKID { "testLocalNetworkId" };
const std::string REMOTE_NETWORKID { "testRemoteNetworkId" };
const std::string ORIGIN_NETWORKID { "testOriginNetworkId" };
const std::string DEVICE_ID { "testDeviceId" };
constexpr int32_t SOCKET { 1 };
} // namespace

void ClearCoordinationSoftbusAdapter()
{
    COOR_SOFTBUS_ADAPTER->socketFd_ = -1;
    COOR_SOFTBUS_ADAPTER->localSessionName_ = "";
    COOR_SOFTBUS_ADAPTER->onRecvDataCallback_ = nullptr;
    COOR_SOFTBUS_ADAPTER->sessionDevs_.clear();
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
    ASSERT_NE(g_adapter, nullptr);
    int32_t ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_ERR);
    ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
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
    ASSERT_NE(g_adapter, nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_ERR);
    ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartRemoteCoordination(LOCAL_NETWORKID, REMOTE_NETWORKID, false);
    EXPECT_TRUE(ret == RET_OK);
    ret = g_adapter->StopRemoteCoordination(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_OK);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest003
 * @tc.desc: Test func named StartRemoteCoordinationResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    int32_t ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, REMOTE_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_ERR);
    ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest004
 * @tc.desc: Test func named StartRemoteCoordinationResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, LOCAL_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_ERR);
    ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartRemoteCoordinationResult(REMOTE_NETWORKID, true, LOCAL_NETWORKID, 0, 0);
    EXPECT_TRUE(ret == RET_OK);
    ret = g_adapter->StopRemoteCoordinationResult(REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_OK);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest005
 * @tc.desc: Test func named StartCoordinationOtherResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    int32_t ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest006
 * @tc.desc: Test func named StartCoordinationOtherResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest006, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_adapter->sessionDevs_[ORIGIN_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->StartCoordinationOtherResult(ORIGIN_NETWORKID, REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_OK);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest007
 * @tc.desc: Test func named NotifyFilterAdded, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest007, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    int32_t ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest008
 * @tc.desc: Test func named NotifyFilterAdded, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest008, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = 1;
    g_sendable = false;
    int32_t ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_ERR);
    g_sendable = true;
    ret = g_adapter->NotifyFilterAdded(REMOTE_NETWORKID);
    EXPECT_TRUE(ret == RET_OK);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest009
 * @tc.desc: test normal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest009, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_adapter->sessionDevs_[REMOTE_NETWORKID] = SOCKET;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest010
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest010, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_init = false;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    ASSERT_NE(ret, RET_OK);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest011
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest011, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_init = true;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    ASSERT_NE(ret, RET_OK);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest012
 * @tc.desc: test abnormal func named OpenInputSoftbus and CloseInputSoftbus in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest012, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    g_init = true;
    g_cond = false;
    int32_t ret = g_adapter->OpenInputSoftbus(REMOTE_NETWORKID);
    ASSERT_NE(ret, RET_OK);
    g_adapter->CloseInputSoftbus(REMOTE_NETWORKID);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest013
 * @tc.desc: test normal func named OnBind and OnShutdown in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest013, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    PeerSocketInfo info;
    char deviceId[] = "softbus";
    info.networkId = deviceId;
    int32_t ret = g_adapter->OnBind(SOCKET, info);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->OnShutdown(SOCKET, SHUTDOWN_REASON_UNKNOWN);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest014
 * @tc.desc: test normal func named OnBind and OnShutdown in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest014, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    PeerSocketInfo info;
    char deviceId[] = "softbus";
    info.networkId = deviceId;
    int32_t ret = g_adapter->OnBind(SOCKET, info);
    EXPECT_EQ(ret, RET_OK);
    g_adapter->OnShutdown(SOCKET, SHUTDOWN_REASON_UNKNOWN);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest015
 * @tc.desc: test abnormal func named SendData in devicestatus
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest015, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    std::string data = "TestSendData";
    Parcel parcel;
    if (!parcel.WriteBuffer(reinterpreter_cast<void *> (const_cast<void *> (data.c_str())), data.size())) {
        EXPECT_EQ(RET_ERR, RET_ERR);
    }
    g_adapter->sessionDevs_[DEVICE_ID] = SOCKET;
    int32_t ret = g_adapter->SendData(DEVICE_ID, parcel);
    EXPECT_EQ(ret, RET_ERR);
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest016
 * @tc.desc: Test func named NotifyUnchainedResult, sessionDevs_ is null
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest016, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
    int32_t ret = g_adapter->NotifyUnchainedResult(LOCAL_NETWORKID, REMOTE_NETWORKID, true);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: CoordinationSoftbusAdapterTest017
 * @tc.desc: Test func named NotifyUnchainedResult, sessionDevs_ is not null, sendMsg is err or ok
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSoftbusAdapterTest, CoordinationSoftbusAdapterTest017, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NE(g_adapter, nullptr);
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
