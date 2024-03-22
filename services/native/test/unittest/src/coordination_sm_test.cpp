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
#define protected public

#include "coordination_sm_test.h"

#include "accesstoken_kit.h"
#include <gtest/gtest.h>
#include "nativetoken_kit.h"
#include "nocopyable.h"
#include "token_setproc.h"
#include "pointer_event.h"
#include "coordination_device_manager.h"
#include "coordination_event_handler.h"
#include "coordination_message.h"
#include "coordination_sm.h"
#include "coordination_softbus_adapter.h"
#include "coordination_state_in.h"
#include "coordination_util.h"
#include "device.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationSMTest"

using namespace ::OHOS;
using namespace ::OHOS::Security::AccessToken;

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
const std::string REMOTE_NETWORKID { "Test_Remote_NetworkId" };
const std::string ORIGIN_NETWORKID { "Test_Origin_NetworkId" };
constexpr int32_t DEVICE_ID { 0 };
constexpr int32_t UNKNOWN_STATE { 3 };
} // namespace

void ClearCoordiantionSM()
{
    COOR_SM->preparedNetworkId_ = { "", "" };
    COOR_SM->startDeviceDhid_ = "";
    COOR_SM->remoteNetworkId_ = "";
    COOR_SM->sinkNetworkId_ = "";
    COOR_SM->isUnchained_ = false;
    COOR_SM->currentState_ = CoordinationState::STATE_FREE;
    COOR_SM->initCallback_ = nullptr;
    COOR_SM->stateCallback_ = nullptr;
    COOR_SM->isStarting_ = false;
    COOR_SM->isStopping_ = false;
    COOR_SM->mouseLocation_ = std::make_pair(0, 0);
    COOR_SM->lastPointerEvent_ = nullptr;
    COOR_SM->displayX_ = -1;
    COOR_SM->displayY_ = -1;
    COOR_SM->monitorId_ = -1;
    COOR_SM->interceptorId_ = -1;
    COOR_SM->filterId_ = -1;
    COOR_SM->remoteNetworkIdCallback_ = nullptr;
    COOR_SM->mouseLocationCallback_ = nullptr;
    COOR_SM->notifyDragCancelCallback_ = nullptr;
    COOR_SM->runner_ = nullptr;
    COOR_SM->onlineDevice_.clear();
    COOR_SM->stateChangedCallbacks_.clear();
    COOR_SM->coordinationStates_.clear();
}

void ClearCoordinationSoftbusAdapter()
{
    COOR_SOFTBUS_ADAPTER->socketFd_ = -1;
    COOR_SOFTBUS_ADAPTER->localSessionName_ = "";
    COOR_SOFTBUS_ADAPTER->onRecvDataCallback_ = nullptr;
    COOR_SOFTBUS_ADAPTER->sessionDevs_.clear();
}

class CoordinationSMTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown() {}
    void AddPermission();
    void SetAceessTokenPermission(const char** perms, size_t permAmount);
};

void CoordinationSMTest::SetUp()
{
    AddPermission();
}

void CoordinationSMTest::AddPermission()
{
    const char *perms[] = { "ohos.permission.DISTRIBUTED_DATASYNC" };
    SetAceessTokenPermission(perms, sizeof(perms) / sizeof(perms[0]));
}

void CoordinationSMTest::SetAceessTokenPermission(const char** perms, size_t permAmount)
{
    if (perms == nullptr || permAmount == 0) {
        FI_HILOGE("The perms is empty");
        return;
    }
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permAmount,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = "CoordinationSMTest",
        .aplStr = "system_basic",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

int32_t CoordinationSoftbusAdapter::SendMsg(int32_t sessionId, const std::string &message)
{
    CALL_DEBUG_ENTER;
    return sessionId == 1 ? RET_OK : RET_ERR;
}

Device::Device(int32_t deviceId) :deviceId_(deviceId) {}

Device::~Device() {}

int32_t Device::Open()
{
    return 0;
}

void Device::Close() {}

void Device::Dispatch(const struct epoll_event &ev) {}

/**
 * @tc.name: CoordinationSMTest001
 * @tc.desc: test IsNeedFilterOut state == CoordinationState::STATE_OUT
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    keyEvent->SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_BACK);
    keyEvent->SetActionTime(1);
    keyEvent->SetKeyAction(OHOS::MMI::KeyEvent::KEY_ACTION_DOWN);
    OHOS::MMI::KeyEvent::KeyItem item;
    item.SetKeyCode(OHOS::MMI::KeyEvent::KEYCODE_BACK);
    item.SetDownTime(1);
    item.SetPressed(true);
    keyEvent->AddKeyItem(item);
    bool ret = COOR_SM->IsNeedFilterOut(localNetworkId, keyEvent);
    EXPECT_EQ(false, ret);
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSMTest002
 * @tc.desc: test abnormal GetCoordinationState when local network id is empty
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = COOR_SM->GetCoordinationState("");
    EXPECT_TRUE(ret == COMMON_PARAMETER_ERROR);
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSMTest003
 * @tc.desc: test normal GetCoordinationState when local network id is correct
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    if (!localNetworkId.empty()) {
        int32_t ret = COOR_SM->GetCoordinationState(localNetworkId);
        EXPECT_TRUE(ret == 0);
        ClearCoordiantionSM();
        ClearCoordinationSoftbusAdapter();
    }
}

/**
 * @tc.name: CoordinationSMTest004
 * @tc.desc: Interface (GetDeviceCoordinationState) testing
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string state = COOR_SM->GetDeviceCoordinationState(CoordinationState::STATE_FREE);
    EXPECT_TRUE(state == "free");
    state = COOR_SM->GetDeviceCoordinationState(CoordinationState::STATE_IN);
    EXPECT_TRUE(state == "in");
    state = COOR_SM->GetDeviceCoordinationState(CoordinationState::STATE_OUT);
    EXPECT_TRUE(state == "out");
    state = COOR_SM->GetDeviceCoordinationState(static_cast<CoordinationState>(UNKNOWN_STATE));
    EXPECT_TRUE(state == "unknown");
}

/**
 * @tc.name: CoordinationSMTest005
 * @tc.desc: Interface (UpdateLastPointerEventCallback) testing
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    COOR_SM->UpdateLastPointerEventCallback(pointerEvent);
    EXPECT_TRUE(COOR_SM->lastPointerEvent_ == pointerEvent);
}

/**
 * @tc.name: CoordinationSMTest006
 * @tc.desc: Interface (GetLastPointerEvent) testing
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest006, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    COOR_SM->UpdateLastPointerEventCallback(pointerEvent);
    auto lastPointerEvent = COOR_SM->GetLastPointerEvent();
    ASSERT_NE(lastPointerEvent, nullptr);
    EXPECT_TRUE(lastPointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_CANCEL);
}

/**
 * @tc.name: CoordinationSMTest007
 * @tc.desc: Interface (SetSinkNetworkId) testing
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest007, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string remoteNetworkId("cde2b5b4453a5b3ec566f836ffa7a4aab52c4b9c8a0b34f3d6aaca4566db24f0");
    COOR_SM->SetSinkNetworkId(remoteNetworkId);
    EXPECT_TRUE(COOR_SM->sinkNetworkId_ == remoteNetworkId);
    COOR_SM->sinkNetworkId_.clear();
}

/**
 * @tc.name: CoordinationSMTest008
 * @tc.desc: Interface (Reset) testing
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest008, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    COOR_SM->isStarting_ = true;
    std::string networkId("cde2b5b4453a5b3ec566f836ffa7a4aab52c4b9c8a0b34f3d6aaca4566db24f0");
    COOR_SM->Reset(networkId);
    EXPECT_TRUE(!COOR_SM->isStarting_);
    COOR_SM->remoteNetworkId_.clear();
    COOR_SM->sinkNetworkId_.clear();
}

/**
 * @tc.name: CoordinationSMTest009
 * @tc.desc: test normal ActivateCoordination return the correct value
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest009, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t startDeviceId = 1;
    COOR_SOFTBUS_ADAPTER->sessionDevs_[REMOTE_NETWORKID] = 0;
    COOR_SM->currentState_ = CoordinationState::STATE_IN;
    COOR_SM->coordinationStates_[CoordinationState::STATE_IN] = std::make_shared<CoordinationStateIn>();
    int32_t ret = COOR_SM->ActivateCoordination(REMOTE_NETWORKID, startDeviceId);
    EXPECT_EQ(ret, COMMON_PARAMETER_ERROR);
    COOR_SOFTBUS_ADAPTER->sessionDevs_[REMOTE_NETWORKID] = 1;
    ret = COOR_SM->ActivateCoordination(REMOTE_NETWORKID, startDeviceId);
    EXPECT_EQ(ret, COMMON_PARAMETER_ERROR);
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSMTest010
 * @tc.desc: test normal DeactivateCoordination return the correct value
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest010, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    COOR_SM->currentState_ = CoordinationState::STATE_IN;
    COOR_SM->coordinationStates_[CoordinationState::STATE_IN] = std::make_shared<CoordinationStateIn>();
    COOR_SM->startDeviceDhid_ = "teststartDeviceDhid";
    auto curdevice= std::make_shared<Device>(DEVICE_ID);
    curdevice->name_ = "DistributedInput ";
    auto dev = std::make_shared<CoordinationDeviceManager::Device>(curdevice);
    dev->dhid_ = COOR_SM->startDeviceDhid_;
    dev->networkId_ = "testNetworkId";
    std::function<void(void)> mycallback = [&](void) {
        GTEST_LOG_(INFO) << "notifyDragCancelCallback_ callback test";
    };
    COOR_SM->notifyDragCancelCallback_ = mycallback;
    COOR_DEV_MGR->devices_[0] = dev;
    COOR_SOFTBUS_ADAPTER->sessionDevs_["testNetworkId"] = 0;
    int32_t ret = COOR_SM->DeactivateCoordination(false);
    EXPECT_EQ(ret, RET_ERR);
    COOR_SOFTBUS_ADAPTER->sessionDevs_["testNetworkId"] = 1;
    ret = COOR_SM->DeactivateCoordination(false);
    EXPECT_EQ(ret, RET_OK);
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSMTest011
 * @tc.desc: test normal UpdateState
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest011, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    COOR_SM->UpdateState(CoordinationState::STATE_IN);
    auto curstate = COOR_SM->GetCurrentCoordinationState();
    EXPECT_EQ(curstate, CoordinationState::STATE_IN);
    std::pair<std::string, std::string> devicelist = COOR_SM->GetPreparedDevices();
    EXPECT_TRUE(devicelist.first.empty() && devicelist.second.empty());
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}

/**
 * @tc.name: CoordinationSMTest012
 * @tc.desc: test normal UpdatePreparedDevices and obtain correct value
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest012, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    COOR_SM->UpdatePreparedDevices(REMOTE_NETWORKID, ORIGIN_NETWORKID);
    std::pair<std::string, std::string> devicelist = COOR_SM->GetPreparedDevices();
    EXPECT_TRUE((devicelist.first == REMOTE_NETWORKID) && (devicelist.second == ORIGIN_NETWORKID));
    ClearCoordiantionSM();
    ClearCoordinationSoftbusAdapter();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
