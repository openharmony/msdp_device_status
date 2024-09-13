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
#include "input_event_builder_test.h"
#include "ddm_adapter.h"

#undef LOG_TAG
#define LOG_TAG "InputEventBuilderTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
using namespace testing::ext;

namespace {
ContextService *g_instance = nullptr;
std::unique_ptr<IInputAdapter> input_;
std::unique_ptr<IPluginManager> pluginMgr_;
std::unique_ptr<IDSoftbusAdapter> dsoftbus_;
SocketSessionManager socketSessionMgr_;
InputEventBuilder *builder_ = {nullptr};
auto env_ = ContextService::GetInstance();
const std::string networkId_ = "1234";
} // namespace

ContextService::ContextService()
{
    ddm_ = std::make_unique<DDMAdapter>();
}

ContextService::~ContextService() {}

IDelegateTasks& ContextService::GetDelegateTasks()
{
    return delegateTasks_;
}

IDeviceManager& ContextService::GetDeviceManager()
{
    return devMgr_;
}

ITimerManager& ContextService::GetTimerManager()
{
    return timerMgr_;
}

IDragManager& ContextService::GetDragManager()
{
    return dragMgr_;
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
    return socketSessionMgr_;
}

IDDMAdapter& ContextService::GetDDM()
{
    return *ddm_;
}

IPluginManager& ContextService::GetPluginManager()
{
    return *pluginMgr_;
}

IInputAdapter& ContextService::GetInput()
{
    return *input_;
}

IDSoftbusAdapter& ContextService::GetDSoftbus()
{
    return *dsoftbus_;
}

void InputEventBuilderTest::SetUpTestCase()
{
    ASSERT_NE(env_, nullptr);
    builder_ = new InputEventBuilder(env_);
    ASSERT_NE(builder_, nullptr);
    dsoftbus_ = std::make_unique<DSoftbusAdapter>();
    input_ = std::make_unique<InputAdapter>();
}

void InputEventBuilderTest::TearDownTestCase()
{
    delete builder_;
    builder_ = nullptr;
}

void InputEventBuilderTest::SetUp() {}
void InputEventBuilderTest::TearDown() {}

/**
 * @tc.name: EnableTest001
 * @tc.desc: Test EnableTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, EnableTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    Context context(env_);
    ASSERT_NO_FATAL_FAILURE(builder_->Enable(context));
}

/**
 * @tc.name: UpdateTest001
 * @tc.desc: Test UpdateTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, UpdateTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    Context context(env_);
    ASSERT_NO_FATAL_FAILURE(builder_->Update(context));
}

/**
 * @tc.name: OnPacketTest001
 * @tc.desc: Test OnPacketTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, OnPacketTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    builder_->remoteNetworkId_ = "12345";
    NetPacket packet(MessageId::DSOFTBUS_INPUT_POINTER_EVENT);
    bool ret = builder_->OnPacket(networkId_, packet);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: OnPacketTest002
 * @tc.desc: Test OnPacketTest002
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, OnPacketTest002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    builder_->remoteNetworkId_ = "1234";
    NetPacket packet(MessageId::DSOFTBUS_INPUT_POINTER_EVENT);
    bool ret = builder_->OnPacket(networkId_, packet);
    ASSERT_EQ(ret, true);
    NetPacket packet1(MessageId::DSOFTBUS_INPUT_KEY_EVENT);
    ret = builder_->OnPacket(networkId_, packet1);
    ASSERT_EQ(ret, true);
    NetPacket packet2(MessageId::DSOFTBUS_RELAY_COOPERATE);
    ret = builder_->OnPacket(networkId_, packet2);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: OnPacketTest003
 * @tc.desc: Test OnPacketTest003
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, OnPacketTest003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    builder_->remoteNetworkId_ = "1234";
    NetPacket packet(MessageId::DSOFTBUS_INPUT_POINTER_EVENT);
    int32_t ret1 = InputEventSerialization::Marshalling(builder_->pointerEvent_, packet);
    ASSERT_EQ(ret1, RET_OK);
    bool ret = builder_->OnPacket(networkId_, packet);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: InputEventBuilderTest_Freeze_001
 * @tc.desc: Test the funcation Freeze
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_Freeze_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    builder_->enable_ = false;
    ASSERT_NO_FATAL_FAILURE(builder_->Freeze());
    builder_->enable_ = true;
    ASSERT_NO_FATAL_FAILURE(builder_->Freeze());
}

/**
 * @tc.name: InputEventBuilderTest_Thaw_001
 * @tc.desc: Test the funcation Thaw
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_Thaw_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    builder_->enable_ = false;
    ASSERT_NO_FATAL_FAILURE(builder_->Thaw());
    builder_->enable_ = true;
    ASSERT_NO_FATAL_FAILURE(builder_->Thaw());
}

/**
 * @tc.name: InputEventBuilderTest_Enable_001
 * @tc.desc: Test the funcation Enable
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_Enable_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    Context context(env_);
    builder_->enable_ = true;
    ASSERT_NO_FATAL_FAILURE(builder_->Enable(context));
    builder_->enable_ = false;
    ASSERT_NO_FATAL_FAILURE(builder_->Enable(context));
}

/**
 * @tc.name: InputEventBuilderTest_UpdatePointerEvent_001
 * @tc.desc: Test the funcation UpdatePointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_UpdatePointerEvent_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    bool ret = builder_->UpdatePointerEvent(pointerEvent);
    ASSERT_FALSE(ret);
}


/**
 * @tc.name: InputEventBuilderTest_IsActive_001
 * @tc.desc: Test the funcation IsActive
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_IsActive_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    builder_->freezing_ = true;
    builder_->IsActive(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_MOVE);
    builder_->IsActive(pointerEvent);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    bool ret = builder_->IsActive(pointerEvent);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: InputEventBuilderTest_IsActive_002
 * @tc.desc: Test the funcation IsActive
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_IsActive_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    builder_->freezing_ = true;
    bool ret = builder_->IsActive(pointerEvent);
    ASSERT_TRUE(ret);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_PULL_MOVE);
    ret = builder_->IsActive(pointerEvent);
    ASSERT_TRUE(ret);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    ret = builder_->IsActive(pointerEvent);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: InputEventBuilderTest_IsActive_003
 * @tc.desc: Test the funcation IsActive
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_IsActive_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerEvent->AddPointerItem(pointerItem);
    builder_->freezing_ = true;
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_MOVE);
    bool ret = builder_->IsActive(pointerEvent);
    ASSERT_FALSE(ret);
    builder_->xDir_ = 1;
    builder_->IsActive(pointerEvent);
    builder_->movement_ = -1;
    ret = builder_->IsActive(pointerEvent);
    ASSERT_FALSE(ret);
}


/**
 * @tc.name: InputEventBuilderTest_SetDamplingCoefficient
 * @tc.desc: Test the funcation SetDamplingCoefficient
 * @tc.type: FUNC
 */
HWTEST_F(InputEventBuilderTest, InputEventBuilderTest_SetDamplingCoefficient, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    uint32_t direction = COORDINATION_DAMPLING_UP;
    double coefficient = 0.5;
    ASSERT_NO_FATAL_FAILURE(builder_->SetDamplingCoefficient(direction, coefficient));
    direction = COORDINATION_DAMPLING_DOWN;
    ASSERT_NO_FATAL_FAILURE(builder_->SetDamplingCoefficient(direction, coefficient));
    direction = COORDINATION_DAMPLING_RIGHT;
    ASSERT_NO_FATAL_FAILURE(builder_->SetDamplingCoefficient(direction, coefficient));
    direction = COORDINATION_DAMPLING_UP;
    ASSERT_NO_FATAL_FAILURE(builder_->SetDamplingCoefficient(direction, coefficient));
    direction = COORDINATION_DAMPLING_UP;
    coefficient = 1.5;
    ASSERT_NO_FATAL_FAILURE(builder_->SetDamplingCoefficient(direction, coefficient));
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS


/**
 * @tc.name: InteractionManagerTest_UpdatePointerAction_01
 * @tc.desc: Check non-text drag shadow
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_UpdatePointerAction_01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_coresInject, sizeof(g_coresInject) / sizeof(g_coresInject[0]));
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("Param displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData({ TEST_PIXEL_MAP_WIDTH, TEST_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    dragData->filterInfo = "{ \"dip_scale\": 3.5, \"drag_shadow_offsetX\": 50, \"drag_shadow_offsetY\": 50, "
        "\"drag_shadow_argb\": 872415231, "
		"\"shadow_color_strategy\": 0, \"shadow_is_hardwareacceleration\": true, \"shadow_elevation\": 120, "
		"\"drag_type\": \"non-text\", \"shadow_enable\": true }";
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<UnitTestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_OK);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = SetupPointerEvent({ DRAG_SRC_X, DRAG_SRC_Y },
        MMI::PointerEvent::POINTER_ACTION_MOVE, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, true);
    MMI::InputManager::GetInstance()->SimulateInputEvent(pointerEvent);
    CHKPV(pointerEvent);
#ifdef OHOS_BUILD_ENABLE_ARKUI_X
    ret = InteractionManager::GetInstance()->UpdatePointerAction(pointerEvent);
    EXPECT_EQ(ret, RET_OK);
#endif // OHOS_BUILD_ENABLE_ARKUI_X
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    InteractionManager::GetInstance()->StopDrag(dropResult);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) !=
        std::future_status::timeout);
    RemovePermission();
}

/**
 * @tc.name: InteractionManagerTest_UpdatePointerAction_02
 * @tc.desc: Check non-text drag shadow
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_UpdatePointerAction_02, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_coresInject, sizeof(g_coresInject) / sizeof(g_coresInject[0]));
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("Param displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData({ TEST_PIXEL_MAP_WIDTH, TEST_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    dragData->filterInfo = "{ \"dip_scale\": 3.5, \"drag_shadow_offsetX\": 50, \"drag_shadow_offsetY\": 50, "
        "\"drag_shadow_argb\": 872415231, "
		"\"shadow_color_strategy\": 0, \"shadow_is_hardwareacceleration\": true, \"shadow_elevation\": 120, "
		"\"drag_type\": \"non-text\", \"shadow_enable\": true }";
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<UnitTestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_OK);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = SetupPointerEvent({ DRAG_SRC_X, DRAG_SRC_Y },
        MMI::PointerEvent::POINTER_ACTION_BUTTON_UP, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, true);
    MMI::InputManager::GetInstance()->SimulateInputEvent(pointerEvent);
    CHKPV(pointerEvent);
#ifdef OHOS_BUILD_ENABLE_ARKUI_X
    ret = InteractionManager::GetInstance()->UpdatePointerAction(pointerEvent);
    EXPECT_EQ(ret, RET_OK);
#endif // OHOS_BUILD_ENABLE_ARKUI_X
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    InteractionManager::GetInstance()->StopDrag(dropResult);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) !=
        std::future_status::timeout);
    RemovePermission();
}

/**
 * @tc.name: InteractionManagerTest_SetDragWindow
 * @tc.desc: Check non-text drag shadow
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_SetDragWindow, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_coresInject, sizeof(g_coresInject) / sizeof(g_coresInject[0]));
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("Param displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData({ TEST_PIXEL_MAP_WIDTH, TEST_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    dragData->filterInfo = "{ \"dip_scale\": 3.5, \"drag_shadow_offsetX\": 50, \"drag_shadow_offsetY\": 50, "
        "\"drag_shadow_argb\": 872415231, "
		"\"shadow_color_strategy\": 0, \"shadow_is_hardwareacceleration\": true, \"shadow_elevation\": 120, "
		"\"drag_type\": \"non-text\", \"shadow_enable\": true }";
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<UnitTestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_OK);
#ifdef OHOS_BUILD_ENABLE_ARKUI_X
    std::shared_ptr<OHOS::Rosen::Window> window { nullptr };
    InteractionManager::GetInstance()->SetDragWindow(window);
#endif // OHOS_BUILD_ENABLE_ARKUI_X
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    InteractionManager::GetInstance()->StopDrag(dropResult);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) !=
        std::future_status::timeout);
    RemovePermission();
}

/**
 * @tc.name: InteractionManagerTest_RegisterDragWindow
 * @tc.desc: Check non-text drag shadow
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_RegisterDragWindow, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_coresInject, sizeof(g_coresInject) / sizeof(g_coresInject[0]));
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("Param displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData({ TEST_PIXEL_MAP_WIDTH, TEST_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    dragData->filterInfo = "{ \"dip_scale\": 3.5, \"drag_shadow_offsetX\": 50, \"drag_shadow_offsetY\": 50, "
        "\"drag_shadow_argb\": 872415231, "
		"\"shadow_color_strategy\": 0, \"shadow_is_hardwareacceleration\": true, \"shadow_elevation\": 120, "
		"\"drag_type\": \"non-text\", \"shadow_enable\": true }";
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<UnitTestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_OK);
#ifdef OHOS_BUILD_ENABLE_ARKUI_X
    std::function<void()> callback = nullptr;
    InteractionManager::GetInstance()->RegisterDragWindow(callback);
#endif // OHOS_BUILD_ENABLE_ARKUI_X
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    InteractionManager::GetInstance()->StopDrag(dropResult);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) !=
        std::future_status::timeout);
    RemovePermission();
}

/**
 * @tc.name: InteractionManagerTest_SetSVGFilePath
 * @tc.desc: Check non-text drag shadow
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_SetSVGFilePath, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_coresInject, sizeof(g_coresInject) / sizeof(g_coresInject[0]));
    std::promise<bool> promiseFlag;
    std::future<bool> futureFlag = promiseFlag.get_future();
    auto callback = [&promiseFlag](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("Param displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData({ TEST_PIXEL_MAP_WIDTH, TEST_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    ASSERT_TRUE(dragData);
    dragData->filterInfo = "{ \"dip_scale\": 3.5, \"drag_shadow_offsetX\": 50, \"drag_shadow_offsetY\": 50, "
        "\"drag_shadow_argb\": 872415231, "
		"\"shadow_color_strategy\": 0, \"shadow_is_hardwareacceleration\": true, \"shadow_elevation\": 120, "
		"\"drag_type\": \"non-text\", \"shadow_enable\": true }";
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<UnitTestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_OK);
#ifdef OHOS_BUILD_ENABLE_ARKUI_X
    std::string filePath = "xxx/not_exist_file.txt";
    InteractionManager::GetInstance()->SetSVGFilePath(filePath);
#endif // OHOS_BUILD_ENABLE_ARKUI_X
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    InteractionManager::GetInstance()->StopDrag(dropResult);
    ASSERT_TRUE(futureFlag.wait_for(std::chrono::milliseconds(PROMISE_WAIT_SPAN_MS)) !=
        std::future_status::timeout);
    RemovePermission();
}