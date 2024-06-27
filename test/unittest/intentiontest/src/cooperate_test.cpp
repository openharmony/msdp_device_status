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
#include "cooperate_test.h"

#include "cooperate.h"
#include "cooperate_params.h"
#include "input_adapter.h"
#include "i_cooperate.h"
#include "ipc_skeleton.h"
#include "dsoftbus_adapter.h"
#include "plugin_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
using namespace Cooperate;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };

DelegateTasks g_delegateTasks;
DeviceManager g_devMgr;
TimerManager g_timerMgr;
DragManager g_dragMgr;
ContextService *g_instance = nullptr;
SocketSessionManager g_socketSessionMgr;
std::unique_ptr<IInputAdapter> g_input;
std::unique_ptr<IPluginManager> g_pluginMgr;
std::unique_ptr<IDSoftbusAdapter> g_dsoftbus;
ICooperate* g_cooperate { nullptr };
Channel<CooperateEvent>::Sender g_sender;
} // namespace

ContextService::ContextService()
{
}

ContextService::~ContextService()
{
}

IDelegateTasks& ContextService::GetDelegateTasks()
{
    return g_delegateTasks;
}

IDeviceManager& ContextService::GetDeviceManager()
{
    return g_devMgr;
}

ITimerManager& ContextService::GetTimerManager()
{
    return g_timerMgr;
}

IDragManager& ContextService::GetDragManager()
{
    return g_dragMgr;
}

ContextService* ContextService::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        ContextService *cooContext = new (std::nothrow) ContextService();
        CHKPL(cooContext);
        g_instance = cooContext;
    });
    return g_instance;
}

ISocketSessionManager& ContextService::GetSocketSessionManager()
{
    return g_socketSessionMgr;
}

IPluginManager& ContextService::GetPluginManager()
{
    return *g_pluginMgr;
}

IInputAdapter& ContextService::GetInput()
{
    return *g_input;
}

IDSoftbusAdapter& ContextService::GetDSoftbus()
{
    return *g_dsoftbus;
}

class CooperateObserver final : public ICooperateObserver {
public:
    CooperateObserver() = default;
    virtual ~CooperateObserver() = default;

    virtual bool IsAllowCooperate()
    {
        return true;
    }
    virtual void OnStartCooperate(StartCooperateData &data) {}
    virtual void OnRemoteStartCooperate(RemoteStartCooperateData &data) {}
    virtual void OnTransitionOut(const std::string &remoteNetworkId, const NormalizedCoordinate &cursorPos) {}
    virtual void OnTransitionIn(const std::string &remoteNetworkId, const NormalizedCoordinate &cursorPos) {}
    virtual void OnBack(const std::string &remoteNetworkId, const NormalizedCoordinate &cursorPos) {}
    virtual void OnRelay(const std::string &remoteNetworkId, const NormalizedCoordinate &cursorPos) {}
    virtual void OnReset() {}
    virtual void CloseDistributedFileConnection(const std::string &remoteNetworkId) {}
};

void CooperateTest::SetUpTestCase() {}

void CooperateTest::SetUp()
{
    g_input = std::make_unique<InputAdapter>();
    g_dsoftbus = std::make_unique<DSoftbusAdapter>();
    auto env = ContextService::GetInstance();
    g_pluginMgr = std::make_unique<PluginManager>(env);
    g_cooperate = env->GetPluginManager().LoadCooperate();
}

void CooperateTest::TearDown()
{
}

void CooperateTest::TearDownTestCase()
{
    if (g_cooperate == nullptr) {
        GTEST_LOG_(INFO) << "g_cooperate is nullptr";
        return;
    }
    ContextService::GetInstance()->GetPluginManager().UnloadCooperate();
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

/**
 * @tc.name: CooperateTest1
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        std::shared_ptr<ICooperateObserver> observer = std::make_shared<CooperateObserver>();
        g_cooperate->AddObserver(observer);
        g_cooperate->RemoveObserver(observer);
        ret = g_cooperate->RegisterListener(IPCSkeleton::GetCallingPid());
        EXPECT_EQ(ret, RET_OK);
        ret = g_cooperate->UnregisterListener(IPCSkeleton::GetCallingPid());
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest2
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        int32_t ret = g_cooperate->RegisterHotAreaListener(IPCSkeleton::GetCallingPid());
        EXPECT_EQ(ret, RET_OK);
        ret = g_cooperate->UnregisterHotAreaListener(IPCSkeleton::GetCallingPid());
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest3
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest3, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        int32_t ret = g_cooperate->Enable(1, IPCSkeleton::GetCallingPid(), 1);
        EXPECT_EQ(ret, RET_OK);
        ret = g_cooperate->Disable(IPCSkeleton::GetCallingPid(), 1);
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest4
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest4, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        int32_t ret = g_cooperate->Start(IPCSkeleton::GetCallingPid(), 1, "test", 1);
        EXPECT_GE(ret, 0);
        ret = g_cooperate->Stop(IPCSkeleton::GetCallingPid(), 1, true);
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest5
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest5, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        int32_t ret = g_cooperate->GetCooperateState(IPCSkeleton::GetCallingPid(), 1, "test");
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest6
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest6, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        int32_t ret = g_cooperate->RegisterEventListener(1, "test");
        EXPECT_EQ(ret, RET_OK);
        ret = g_cooperate->UnregisterEventListener(1, "test");
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}

/**
 * @tc.name: CooperateTest7
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperateTest, CooperateTest7, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = RET_ERR;
    if (g_cooperate != nullptr) {
        g_cooperate->Dump(1);
        GetCooperateStateSyncParam param;
        bool state { false };
        int32_t ret = g_cooperate->GetCooperateState(param.udId, state);
        EXPECT_EQ(ret, RET_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
    } else {
        GTEST_LOG_(INFO) << "The product does not intention_cooperate so";
        EXPECT_EQ(!ret, RET_OK);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS



/**
 * @tc.name: inputDevcieMgr_test056
 * @tc.desc: Test cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test055, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Channel<CooperateEvent>::Sender sender;
    g_context->inputDevMgr_.Enable(sender);
    bool switchStatus = false;
    DSoftbusSessionOpened notice = {
            .networkId = LOCAL_NETWORKID,
            .normal = switchStatus,
    };
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnSoftbusSessionOpened(notice));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnSoftbusSessionClosed(notice));
    bool ret = g_context->inputDevMgr_.OnRawData(LOCAL_NETWORKID, nullptr, 0);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: inputDevcieMgr_test056
 * @tc.desc: Test cooperate plugin OnPacket
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test056, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    NetPacket pkt1(MessageId::INVALID);
    bool ret = g_context->inputDevMgr_.OnPacket(LOCAL_NETWORKID, pkt1);
    EXPECT_FALSE(ret);
    NetPacket pkt2(MessageId::DSOFTBUS_INPUT_DEV_SYNC);
    ret = g_context->inputDevMgr_.OnPacket(LOCAL_NETWORKID, pkt2);
    EXPECT_TRUE(ret);
    NetPacket pkt3(MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG);
    ret = g_context->inputDevMgr_.OnPacket(LOCAL_NETWORKID, pkt3);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: inputDevcieMgr_test057
 * @tc.desc: Test cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test057, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    NetPacket pkt1(MessageId::DSOFTBUS_INPUT_DEV_SYNC);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnRemoteInputDevice(LOCAL_NETWORKID, pkt1));
    NetPacket pkt2(MessageId::INVALID);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnRemoteInputDevice(LOCAL_NETWORKID, pkt2));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnRemoteHotPlug(LOCAL_NETWORKID, pkt2));
    NetPacket pkt3(MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.OnRemoteHotPlug(LOCAL_NETWORKID, pkt3));
}

/**
 * @tc.name: inputDevcieMgr_test058
 * @tc.desc: Test cooperate plugin HandleRemoteHotPlug
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test058, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    RemoteHotPlugEvent notice1 = {
            .networkId = LOCAL_NETWORKID,
            .remoteDeviceId = VREMOTE_NETWORKID,
            .type = InputHotplugType::PLUG,
    };
    RemoteHotPlugEvent notice2 = {
            .networkId = LOCAL_NETWORKID,
            .remoteDeviceId = VREMOTE_NETWORKID,
            .type = InputHotplugType::UNPLUG,
    };
    g_context->inputDevMgr_.AddVirtualInputDevice(notice1.networkId, 987654321);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.HandleRemoteHotPlug(notice2));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.HandleRemoteHotPlug(notice1));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.HandleRemoteHotPlug(notice2));
}

/**
 * @tc.name: inputDevcieMgr_test059
 * @tc.desc: Test cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test059, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.NotifyInputDeviceToRemote(REMOTE_NETWORKID));
    std::shared_ptr<IDevice> g_device = std::make_shared<Device>(VREMOTE_NETWORKID);
    g_context->inputDevMgr_.AddRemoteInputDevice(LOCAL_NETWORKID, g_device);
    g_context->inputDevMgr_.AddRemoteInputDevice(LOCAL_NETWORKID, g_device);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.RemoveRemoteInputDevice(LOCAL_NETWORKID, g_device));
    g_context->inputDevMgr_.AddRemoteInputDevice(LOCAL_NETWORKID, g_device);
    g_context->inputDevMgr_.RemoveAllRemoteInputDevice(REMOTE_NETWORKID);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.RemoveAllRemoteInputDevice(LOCAL_NETWORKID));
    g_context->inputDevMgr_.DumpRemoteInputDevice(LOCAL_NETWORKID);
    g_context->inputDevMgr_.AddRemoteInputDevice(LOCAL_NETWORKID, g_device);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.DumpRemoteInputDevice(LOCAL_NETWORKID));
    NetPacket packet(MessageId::DSOFTBUS_INPUT_DEV_SYNC);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.SerializeDevice(g_device, packet));
    g_device = nullptr;
}

/**
 * @tc.name: inputDevcieMgr_test060
 * @tc.desc: Test cooperate plugin HandleRemoteHotPlug
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test060, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<IDevice> g_device = std::make_shared<Device>(VREMOTE_NETWORKID);
    NetPacket pkt1(MessageId::INVALID);
    int32_t ret = g_context->inputDevMgr_.DeserializeDevice(g_device, pkt1);
    EXPECT_EQ(ret, RET_ERR);
    NetPacket pkt2(MessageId::DSOFTBUS_INPUT_DEV_SYNC);
    ret = g_context->inputDevMgr_.DeserializeDevice(g_device, pkt2);
    EXPECT_EQ(ret, RET_ERR);
    NetPacket pkt3(MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG);
    ret = g_context->inputDevMgr_.DeserializeDevice(g_device, pkt3);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: inputDevcieMgr_test061
 * @tc.desc: Test cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test061, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<IDevice> g_device = std::make_shared<Device>(VREMOTE_NETWORKID);
    auto inputDev = std::make_shared<MMI::InputDevice>();
    g_device->AddCapability(Device::Capability::DEVICE_CAP_MAX);
    inputDev = g_context->inputDevMgr_.Transform(g_device);
    g_device->AddCapability(Device::Capability::DEVICE_CAP_KEYBOARD);
    inputDev = g_context->inputDevMgr_.Transform(g_device);
    EXPECT_NE(inputDev, nullptr);
    g_device->AddCapability(Device::Capability::DEVICE_CAP_POINTER);
    inputDev = g_context->inputDevMgr_.Transform(g_device);
    EXPECT_NE(inputDev, nullptr);
}

/**
 * @tc.name: inputDevcieMgr_test062
 * @tc.desc: Test cooperate plugin GetRemoteDeviceById
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, inputDevcieMgr_test062, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<IDevice> g_device = std::make_shared<Device>(VREMOTE_NETWORKID);
    auto ret = g_context->inputDevMgr_.GetRemoteDeviceById(LOCAL_NETWORKID, VREMOTE_NETWORKID);
    EXPECT_EQ(ret, nullptr);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.RemoveVirtualInputDevice(LOCAL_NETWORKID, VREMOTE_NETWORKID));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.AddRemoteInputDevice(LOCAL_NETWORKID, g_device));
    ret = g_context->inputDevMgr_.GetRemoteDeviceById(LOCAL_NETWORKID, VREMOTE_NETWORKID);
    EXPECT_NE(ret, nullptr);
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.AddVirtualInputDevice(LOCAL_NETWORKID, VREMOTE_NETWORKID));
    ASSERT_NO_FATAL_FAILURE(g_context->inputDevMgr_.RemoveVirtualInputDevice(LOCAL_NETWORKID, VREMOTE_NETWORKID));
}