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

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include <fcntl.h>
#include "nocopyable.h"

#include "delegate_tasks.h"
#include "device_manager.h"
#include "devicestatus_define.h"
#include "devicestatus_delayed_sp_singleton.h"
#include "drag_manager.h"
#include "i_context.h"
#include "timer_manager.h"

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#include "intention_service.h"
#include "socket_session_manager.h"
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

#undef LOG_TAG
#define LOG_TAG "IntentionDeviceManagerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
class ContextService final : public IContext {
    ContextService();
    ~ContextService() = default;
    DISALLOW_COPY_AND_MOVE(ContextService);
public:
    IDelegateTasks& GetDelegateTasks() override;
    IDeviceManager& GetDeviceManager() override;
    ITimerManager& GetTimerManager() override;
    IDragManager& GetDragManager() override;

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    IPluginManager& GetPluginManager() override;
    ISocketSessionManager& GetSocketSessionManager() override;
    IInputAdapter& GetInput() override;
    IDSoftbusAdapter& GetDSoftbusAda() override;
    IDDPAdapter& GetDP() override;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
private:
    void OnStart();
    bool Init();
    int32_t InitDeviceMgr();
    int32_t InitDelegateTasks();
    static ContextService* GetInstance();
private:
    DelegateTasks delegateTasks_;
    TimerManager timerMgr_;
    DeviceManager devMgr_;
    DragManager dragMgr_;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    SocketSessionManager socketSessionMgr_;
    std::unique_ptr<IInputAdapter> input_;
    std::unique_ptr<IPluginManager> pluginMgr_;
    std::unique_ptr<IDSoftbusAdapter> dsoftbusAda_;
    std::unique_ptr<IDDPAdapter> ddp_;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
};
class IntentionDeviceManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEV_NODE { "/dev/input/TestDeviceNode" };
ContextService *g_instance = nullptr;
constexpr int32_t TIME_WAIT_FOR_OP_MS { 100 };
} // namespace

ContextService::ContextService()
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    FI_HILOGI("OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK is on");
    OnStart();
#else
    FI_HILOGI("OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK is off");
    OnStart();
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

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
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
ISocketSessionManager& ContextService::GetSocketSessionManager()
{
    return socketSessionMgr_;
}

IPluginManager& ContextService::GetPluginManager()
{
    return *pluginMgr_;
}

IInputAdapter& ContextService::GetInput()
{
    return *input_;
}

IDSoftbusAdapter& ContextService::GetDSoftbusAda()
{
    return *dsoftbusAda_;
}

IDDPAdapter& ContextService::GetDP()
{
    return *ddp_;
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

bool ContextService::Init()
{
    CALL_DEBUG_ENTER;
    if (InitDelegateTasks() != RET_OK) {
        FI_HILOGE("Delegate tasks init failed");
        goto INIT_FAIL;
    }

    if (InitDeviceMgr() != RET_OK) {
        FI_HILOGE("DeviceMgr init failed");
        goto INIT_FAIL;
    }

    return true;

INIT_FAIL:
    return false;
}

int32_t ContextService::InitDeviceMgr()
{
    CALL_DEBUG_ENTER;
    int32_t ret = devMgr_.Init(this);
    if (ret != RET_OK) {
        FI_HILOGE("DeviceMgr init failed");
        return ret;
    }
    return RET_OK;
}

int32_t ContextService::InitDelegateTasks()
{
    CALL_DEBUG_ENTER;
    if (!delegateTasks_.Init()) {
        FI_HILOGE("The delegate task init failed");
        return RET_ERR;
    }
    return RET_OK;
}

void ContextService::OnStart()
{
    CALL_DEBUG_ENTER;
    if (!Init()) {
        FI_HILOGE("On start call init failed");
        return;
    }
}

void IntentionDeviceManagerTest::SetUpTestCase() {}

void IntentionDeviceManagerTest::TearDownTestCase()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

void IntentionDeviceManagerTest::SetUp() {}

void IntentionDeviceManagerTest::TearDown() {}

/**
 * @tc.name: IntentionDeviceManagerTest01
 * @tc.desc: Test Init and Enable
 * @tc.type: FUNC
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    int32_t ret = env->devMgr_.Enable();
    EXPECT_NE(ret, RET_OK);
    ret = env->devMgr_.Disable();
    EXPECT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest02
 * @tc.desc: test ParseDeviceId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest02, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    std::string devNode = "event123";
    int32_t expectedDeviceId = 123;
    int32_t actualDeviceId = env->devMgr_.ParseDeviceId(devNode);
    EXPECT_EQ(actualDeviceId, expectedDeviceId);
    devNode = "invalid_device";
    actualDeviceId = env->devMgr_.ParseDeviceId(devNode);
    EXPECT_EQ(actualDeviceId, RET_ERR);
}

/**
 * @tc.name: IntentionDeviceManagerTest03
 * @tc.desc: test FindDevice
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest03, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    env->devMgr_.AddDevice(TEST_DEV_NODE);
    std::shared_ptr<IDevice> foundDevice = env->devMgr_.FindDevice(TEST_DEV_NODE);
    EXPECT_EQ(foundDevice, nullptr);
    std::shared_ptr<IDevice> removeDevice = env->devMgr_.RemoveDevice(TEST_DEV_NODE);
    EXPECT_EQ(removeDevice, nullptr);
}

/**
 * @tc.name: IntentionDeviceManagerTest04
 * @tc.desc: test Dispatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest04, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    uint32_t events = EPOLLIN | EPOLLOUT;
    struct epoll_event ev {};
    ev.events = events;
    ev.data.ptr = nullptr;
    ASSERT_NO_FATAL_FAILURE(env->devMgr_.Dispatch(ev));
    int32_t ret = env->devMgr_.OnEpollDispatch(events);
    EXPECT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest05
 * @tc.desc: test GetDevice
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest05, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    std::shared_ptr<IDevice> ret = env->devMgr_.GetDevice(1);
    EXPECT_EQ(ret, nullptr);
    std::shared_ptr<IDevice> result = env->devMgr_.OnGetDevice(1);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: IntentionDeviceManagerTest06
 * @tc.desc: test RetriggerHotplug
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest06, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    std::weak_ptr<IDeviceObserver> weakObserver = std::weak_ptr<IDeviceObserver>();
    ASSERT_NO_FATAL_FAILURE(env->devMgr_.RetriggerHotplug(weakObserver));
    int32_t ret = env->devMgr_.OnRetriggerHotplug(weakObserver);
    EXPECT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest07
 * @tc.desc: test AddDeviceObserver
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest07, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    std::weak_ptr<IDeviceObserver> weakObserver = std::weak_ptr<IDeviceObserver>();
    int32_t ret = env->devMgr_.AddDeviceObserver(weakObserver);
    EXPECT_NE(ret, RET_OK);
    int32_t tmp = env->devMgr_.OnAddDeviceObserver(weakObserver);
    EXPECT_NE(tmp, RET_OK);
    ASSERT_NO_FATAL_FAILURE(env->devMgr_.RemoveDeviceObserver(weakObserver));
    int32_t result = env->devMgr_.OnRemoveDeviceObserver(weakObserver);
    EXPECT_NE(result, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest08
 * @tc.desc: test AnyOf
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest08, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto env = ContextService::GetInstance();
    ASSERT_NE(env, nullptr);
    auto pred = [](std::shared_ptr<IDevice> device) {
        return device != nullptr;
    };
    EXPECT_FALSE(env->devMgr_.AnyOf(pred));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS