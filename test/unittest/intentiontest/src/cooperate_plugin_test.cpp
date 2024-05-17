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
#include "cooperate_plugin_test.h"

#include "cooperate_events.h"
#include "cooperate_context.h"
#include "coordination_event_manager.h"
#include "ddp_adapter.h"
#include "dsoftbus_adapter.h"
#include "ipc_skeleton.h"
#include "mouse_location.h"
#include "socket_session.h"
#include "state_machine.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
using namespace Cooperate;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
std::shared_ptr<Context> g_context { nullptr };
std::shared_ptr<Context> g_contextOne { nullptr };
ContextService *g_instance = nullptr;
IContext *g_icontext { nullptr };
std::shared_ptr<SocketSession> g_session { nullptr };
DelegateTasks g_delegateTasks;
DeviceManager g_devMgr;
TimerManager g_timerMgr;
DragManager g_dragMgr;
SocketSessionManager g_socketSessionMgr;
std::unique_ptr<IInputAdapter> g_input { nullptr };
std::unique_ptr<IPluginManager> g_pluginMgr { nullptr };
std::unique_ptr<IDSoftbusAdapter> g_dsoftbus { nullptr };
std::unique_ptr<IDDPAdapter> g_ddp { nullptr };
std::shared_ptr<Cooperate::StateMachine> g_stateMachine { nullptr };
const std::string LOCAL_NETWORKID { "testLocalNetworkId" };
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

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
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

IDDPAdapter& ContextService::GetDP()
{
    return *g_ddp;
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

MMI::PointerEvent::PointerItem CooperatePluginTest::CreatePointerItem(int32_t pointerId, int32_t deviceId,
    const std::pair<int32_t, int32_t> &displayLocation, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    item.SetDisplayX(displayLocation.first);
    item.SetDisplayY(displayLocation.second);
    item.SetPressed(isPressed);
    return item;
}

void CooperatePluginTest::SetUpTestCase() {}

void CooperatePluginTest::SetUp()
{
    g_ddp = std::make_unique<DDPAdapter>();
    g_dsoftbus = std::make_unique<DSoftbusAdapter>();
    g_contextOne = std::make_shared<Context>(g_icontext);
    auto env = ContextService::GetInstance();
    g_context = std::make_shared<Context>(env);
    int32_t moduleType = 1;
    int32_t tokenType = 1;
    int32_t uid = IPCSkeleton::GetCallingUid();
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t sockFds[2] { 0, -1 };
    g_session = std::make_shared<SocketSession>("test", moduleType, tokenType, sockFds[0], uid, pid);
}

void CooperatePluginTest::TearDown()
{
    g_context = nullptr;
    g_contextOne = nullptr;
    g_session = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

/**
 * @tc.name: CooperatePluginTest1
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, CooperatePluginTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Cooperate::RegisterEventListenerEvent registerEventListenerEvent {};
    g_contextOne->mouseLocation_.AddListener(registerEventListenerEvent);
    g_contextOne->mouseLocation_.RemoveListener(registerEventListenerEvent);
    DSoftbusSubscribeMouseLocation dSoftbusSubscribeMouseLocation {};
    g_contextOne->mouseLocation_.OnSubscribeMouseLocation(dSoftbusSubscribeMouseLocation);
    g_contextOne->mouseLocation_.OnUnSubscribeMouseLocation(dSoftbusSubscribeMouseLocation);

    g_context->mouseLocation_.AddListener(registerEventListenerEvent);
    g_context->mouseLocation_.RemoveListener(registerEventListenerEvent);
    
    Cooperate::RegisterEventListenerEvent registerEventListenerEvent1 {IPCSkeleton::GetCallingPid(), "test"};
    g_context->mouseLocation_.AddListener(registerEventListenerEvent1);
    g_context->mouseLocation_.ProcessData(MMI::PointerEvent::Create());
    g_context->mouseLocation_.RemoveListener(registerEventListenerEvent1);
    Cooperate::LocationInfo locationInfo {1, 1, 1, 1};
    
    DSoftbusSyncMouseLocation dSoftbusSyncMouseLocation{"test", "test", locationInfo};
    g_context->mouseLocation_.OnRemoteMouseLocation(dSoftbusSyncMouseLocation);
    
    g_context->mouseLocation_.listeners_.clear();
    g_context->mouseLocation_.RemoveListener(registerEventListenerEvent1);
    g_context->mouseLocation_.OnRemoteMouseLocation(dSoftbusSyncMouseLocation);

    DSoftbusSubscribeMouseLocation dSoftbusSubscribeMouseLocation1 {"test", "test1"};
    g_context->mouseLocation_.OnSubscribeMouseLocation(dSoftbusSubscribeMouseLocation1);
    g_context->mouseLocation_.ProcessData(MMI::PointerEvent::Create());
    g_context->mouseLocation_.OnUnSubscribeMouseLocation(dSoftbusSubscribeMouseLocation1);
    
    g_context->mouseLocation_.remoteSubscribers_.clear();
    g_context->mouseLocation_.ProcessData(MMI::PointerEvent::Create());
    g_context->mouseLocation_.OnUnSubscribeMouseLocation(dSoftbusSubscribeMouseLocation1);

    DSoftbusReplySubscribeMouseLocation dSoftbusReplySubscribeMouseLocation {"test", "test1", true};
    g_context->mouseLocation_.OnReplySubscribeMouseLocation(dSoftbusReplySubscribeMouseLocation);
    g_context->mouseLocation_.OnReplyUnSubscribeMouseLocation(dSoftbusReplySubscribeMouseLocation);
    DSoftbusReplySubscribeMouseLocation dSoftbusReplySubscribeMouseLocation1 {"test", "test1", false};
    g_context->mouseLocation_.OnReplySubscribeMouseLocation(dSoftbusReplySubscribeMouseLocation1);
    g_context->mouseLocation_.OnReplyUnSubscribeMouseLocation(dSoftbusReplySubscribeMouseLocation1);

    int32_t ret = g_context->mouseLocation_.ReplySubscribeMouseLocation(dSoftbusReplySubscribeMouseLocation);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: CooperatePluginTest2
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, CooperatePluginTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Cooperate::RegisterEventListenerEvent registerEventListenerEvent1 {IPCSkeleton::GetCallingPid(), "test"};
    g_context->mouseLocation_.AddListener(registerEventListenerEvent1);
    g_socketSessionMgr.Init();
    g_socketSessionMgr.AddSession(g_session);
    g_contextOne->mouseLocation_.ReportMouseLocationToListener("test", {}, IPCSkeleton::GetCallingPid());
    g_context->mouseLocation_.ReportMouseLocationToListener("test", {}, IPCSkeleton::GetCallingPid());
    g_context->mouseLocation_.localListeners_.clear();
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: CooperatePluginTest3
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, CooperatePluginTest3, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    g_context->mouseLocation_.ProcessData(nullptr);
    auto pointerEvent = MMI::PointerEvent::Create();
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->SetPointerId(1);
    MMI::PointerEvent::PointerItem curPointerItem = CreatePointerItem(1, 1, { 0, 0 }, true);
    pointerEvent->AddPointerItem(curPointerItem);
    g_context->mouseLocation_.ProcessData(pointerEvent);
    NetPacket pkt(MessageId::COORDINATION_MESSAGE);
    int32_t ret = g_context->mouseLocation_.SendPacket("test", pkt);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event;
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(CooperateEventType::QUIT);
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(
        CooperateEventType::DDM_BOARD_OFFLINE,
        DDMBoardOfflineEvent {
            .networkId = LOCAL_NETWORKID
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    bool switchStatus = false;
    CooperateEvent event(
        CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        DDPCooperateSwitchChanged {
            .networkId = LOCAL_NETWORKID,
            .normal = switchStatus,
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(
        CooperateEventType::INPUT_HOTPLUG_EVENT,
        InputHotplugEvent {
            .deviceId = -1,
            .type = InputHotplugType::PLUG,
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent006, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(
        CooperateEventType::INPUT_HOTPLUG_EVENT,
        InputHotplugEvent {
            .deviceId = -1,
            .type = InputHotplugType::UNPLUG,
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent007, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);

    CooperateEvent event(
        CooperateEventType::INPUT_POINTER_EVENT,
        InputPointerEvent {
            .deviceId = pointerEvent->GetDeviceId(),
            .pointerAction = pointerEvent->GetPointerAction(),
            .sourceType = pointerEvent->GetSourceType(),
            .position = Coordinate {
                .x = pointerItem.GetDisplayX(),
                .y = pointerItem.GetDisplayY(),
            }
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent008, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    MMI::PointerEvent::PointerItem pointerItem = CreatePointerItem(1, 1, { 0, 0 }, true);

    CooperateEvent event(
        CooperateEventType::INPUT_POINTER_EVENT,
        InputPointerEvent {
            .deviceId = pointerEvent->GetDeviceId(),
            .pointerAction = pointerEvent->GetPointerAction(),
            .sourceType = pointerEvent->GetSourceType(),
            .position = Coordinate {
                .x = pointerItem.GetDisplayX(),
                .y = pointerItem.GetDisplayY(),
            }
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent009, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t fd = -1;
    CooperateEvent event(
        CooperateEventType::DUMP,
        DumpEvent {
            .fd = fd
        });
    
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent010, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t pid = IPCSkeleton::GetCallingPid();
    CooperateEvent registerEvent(
        CooperateEventType::REGISTER_EVENT_LISTENER,
        RegisterEventListenerEvent {
            .pid = pid,
            .networkId = LOCAL_NETWORKID,
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, registerEvent);

    CooperateEvent unregisterEvent(
        CooperateEventType::UNREGISTER_EVENT_LISTENER,
        UnregisterEventListenerEvent {
            .pid = pid,
            .networkId = LOCAL_NETWORKID,
        });
    g_stateMachine->OnEvent(cooperateContext, unregisterEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent011, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(CooperateEventType::NOOP);
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent012, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t userData { 0 };
    int32_t pid = IPCSkeleton::GetCallingPid();
    CooperateEvent event(
        CooperateEventType::GET_COOPERATE_STATE,
        GetCooperateStateEvent {
            .pid = pid,
            .userData = userData,
            .networkId = LOCAL_NETWORKID,
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent013, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<ICooperateObserver> observer { nullptr };
    CooperateEvent addEvent(
        CooperateEventType::ADD_OBSERVER,
        AddObserverEvent {
            .observer = observer
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, addEvent);
    CooperateEvent removeEvent(
       CooperateEventType::REMOVE_OBSERVER,
        RemoveObserverEvent {
            .observer = observer
        });
    g_stateMachine->OnEvent(cooperateContext, removeEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent014, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent onlineEvent(
        CooperateEventType::DDM_BOARD_ONLINE,
        DDMBoardOnlineEvent {
            .networkId = LOCAL_NETWORKID
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, onlineEvent);
    CooperateEvent offlineEvent(
        CooperateEventType::DDM_BOARD_OFFLINE,
        DDMBoardOfflineEvent {
            .networkId = LOCAL_NETWORKID
        });
    g_stateMachine->OnEvent(cooperateContext, offlineEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent_015, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CooperateEvent event(CooperateEventType::APP_CLOSED);
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, event);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent016, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t pid = IPCSkeleton::GetCallingPid();
    CooperateEvent registerEvent(
       CooperateEventType::REGISTER_LISTENER,
        UnregisterListenerEvent {
            .pid = pid
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, registerEvent);
    CooperateEvent unregisterEvent(
       CooperateEventType::UNREGISTER_LISTENER,
        UnregisterListenerEvent {
            .pid = pid
        });
    g_stateMachine->OnEvent(cooperateContext, unregisterEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent017, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t pid = IPCSkeleton::GetCallingPid();
    CooperateEvent registerEvent(
       CooperateEventType::REGISTER_HOTAREA_LISTENER,
        RegisterHotareaListenerEvent {
            .pid = pid
        });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, registerEvent);
    CooperateEvent unregisterEvent(
       CooperateEventType::UNREGISTER_HOTAREA_LISTENER,
        UnregisterHotareaListenerEvent {
            .pid = pid
        });
    g_stateMachine->OnEvent(cooperateContext, unregisterEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: StateMachineTest_OnEvent
 * @tc.desc: cooperate plugin
 * @tc.type: FUNC
 * @tc.require:
*/
HWTEST_F(CooperatePluginTest, StateMachineTest_OnEvent018, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t pid = IPCSkeleton::GetCallingPid();
    int32_t userData = 0;
    std::string remoteNetworkId("");
    int32_t startDeviceId = 1;
    bool isUnchained = true;
    CooperateEvent startEvent(
        CooperateEventType::START,
        StartCooperateEvent{
        .pid = pid,
        .userData = userData,
        .remoteNetworkId = remoteNetworkId,
        .startDeviceId = startDeviceId,
        .errCode = std::make_shared<std::promise<int32_t>>(),
    });
    auto env = ContextService::GetInstance();
    Context cooperateContext(env);
    g_stateMachine = std::make_shared<Cooperate::StateMachine>(env);
    g_stateMachine->OnEvent(cooperateContext, startEvent);
    CooperateEvent stopEvent(
        CooperateEventType::STOP,
        StopCooperateEvent {
            .pid = pid,
            .userData = userData,
            .isUnchained = isUnchained,
        });
    g_stateMachine->OnEvent(cooperateContext, stopEvent);
    bool ret = g_context->mouseLocation_.HasLocalListener();
    EXPECT_FALSE(ret);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
