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
#include "input_event_interceptor_test.h"
#include "ddm_adapter.h"

#undef LOG_TAG
#define LOG_TAG "InputEventInterceptorTest"

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
InputEventInterceptor *interceptor_ = {nullptr};
auto env_ = ContextService::GetInstance();
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

void InputEventInterceptorTest::SetUpTestCase()
{
    ASSERT_NE(env_, nullptr);
    interceptor_ = new InputEventInterceptor(env_);
    ASSERT_NE(interceptor_, nullptr);
    dsoftbus_ = std::make_unique<DSoftbusAdapter>();
    input_ = std::make_unique<InputAdapter>();
}

void InputEventInterceptorTest::TearDownTestCase()
{
    delete interceptor_;
    interceptor_ = nullptr;
}

void InputEventInterceptorTest::SetUp() {}
void InputEventInterceptorTest::TearDown() {}

/**
 * @tc.name: EnableTest001
 * @tc.desc: Test EnableTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, EnableTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    Context context(env_);
    ASSERT_NO_FATAL_FAILURE(interceptor_->Enable(context));
    ASSERT_NO_FATAL_FAILURE(interceptor_->Disable());
    interceptor_->interceptorId_ = 1;
    ASSERT_NO_FATAL_FAILURE(interceptor_->Enable(context));
    interceptor_->pointerEventTimer_  = 1;
    ASSERT_NO_FATAL_FAILURE(interceptor_->Disable());
}

/**
 * @tc.name: UpdateTest001
 * @tc.desc: Test UpdateTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, UpdateTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    Context context(env_);
    ASSERT_NO_FATAL_FAILURE(interceptor_->Update(context));
}

/**
 * @tc.name: OnPointerEventTest01
 * @tc.desc: Test OnPointerEventTest01
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnPointerEventTest01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    interceptor_->scanState_ = false;
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
}

/**
 * @tc.name: OnPointerEventTest02
 * @tc.desc: Test OnPointerEventTest02
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnPointerEventTest02, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    interceptor_->pointerEventTimer_  = 1;
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
}

/**
 * @tc.name: OnPointerEventTest03
 * @tc.desc: Test OnPointerEventTest03
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnPointerEventTest03, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_ENTER_WINDOW);
}

/**
 * @tc.name: OnPointerEventTest04
 * @tc.desc: Test OnPointerEventTest04
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnPointerEventTest04, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_CANCEL);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
}

/**
 * @tc.name: OnPointerEventTest05
 * @tc.desc: Test OnPointerEventTest05
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnPointerEventTest05, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_BUTTON_UP);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
    pointerEvent->SetSourceType(MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    pointerEvent->SetPointerAction(MMI::PointerEvent::POINTER_ACTION_DOWN);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnPointerEvent(pointerEvent));
}

/**
 * @tc.name: OnKeyEventTest001
 * @tc.desc: Test OnKeyEventTest001
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, OnKeyEventTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(keyEvent, nullptr);
    interceptor_->OnKeyEvent(keyEvent);
    keyEvent->SetKeyCode(MMI::KeyEvent::KEYCODE_BACK);
    ASSERT_NO_FATAL_FAILURE(interceptor_->OnKeyEvent(keyEvent));
}

/**
 * @tc.name: InputEventInterceptorTest_ReportPointerEvent
 * @tc.desc: Test Test the funcation Enable
 * @tc.type: FUNC
 */
HWTEST_F(InputEventInterceptorTest, InputEventInterceptorTest_ReportPointerEvent, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(pointerEvent, nullptr);
    ASSERT_NO_FATAL_FAILURE(interceptor_->ReportPointerEvent(pointerEvent));
    pointerEvent->SetPointerId(1);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerItem.SetPointerId(1);
    pointerItem.SetDeviceId(1);
    pointerItem.SetDisplayX(0);
    pointerItem.SetDisplayY(0);
    pointerEvent->AddPointerItem(pointerItem);
    ASSERT_NO_FATAL_FAILURE(interceptor_->ReportPointerEvent(pointerEvent));
}
} //namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS