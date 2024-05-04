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

#include "delegate_tasks.h"
#include "timer_manager.h"
#include "drag_manager.h"
#include "device_manager.h"
#include <gtest/gtest.h>
#include "util.h"

#include "device.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "IntentionDeviceManagerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
const std::string TEST_DEV_NODE {"TestDeviceNode"};
} // namespace

class IntentionDeviceManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase(void);
};
void IntentionDeviceManagerTest::SetUpTestCase() {}

void IntentionDeviceManagerTest::TearDownTestCase() {}

void IntentionDeviceManagerTest::SetUp() {}

void IntentionDeviceManagerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

class IContextTest : public IContext {
public:
    IContextTest() = default;
    virtual ~IContextTest() = default;

    IDelegateTasks& GetDelegateTasks() override
    { 
        return delegateTasksTest_;
    }
    IDeviceManager& GetDeviceManager() override
    { 
        return deviceManagerTest_;
    }
    ITimerManager& GetTimerManager() override
    { 
        return timerManagerTest_;
    }
    IDragManager& GetDragManager() override
    { 
        return dragManagerTest_;
    }

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    ISocketSessionManager& GetSocketSessionManager() override
    { 
        return socketSessionManager_;
    }
    IPluginManager& GetPluginManager() override
    {
        return pluginManager_;
    }
    IInputAdapter& GetInput() override
    {
        return inputAdapter_;
    }
    IDInputAdapter& GetDInput() override
    { 
        return dInputAdapter_;
    }
    IDSoftbusAdapter& GetDSoftbus() override
    { 
        return dSoftbusAdapter_;
    }
    IDDPAdapter& GetDP() override
    { 
        return ddpAdapter_;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

private:
    DelegateTasks delegateTasksTest_;
    DeviceManager deviceManagerTest_;
    TimerManager timerManagerTest_;
    DragManager dragManagerTest_;

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    SocketSessionManager socketSessionManager_;
    PluginManager pluginManager_;
    InputAdapter inputAdapter_;
    DInputAdapter dInputAdapter_;
    DSoftbusAdapter dSoftbusAdapter_;
    DDPAdapter ddpAdapter_;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
};


/**
 * @tc.name: IntentionDeviceManagerTest01
 * @tc.desc: test Init and OnInit Parameter is empty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest01, TestSize.Level1)
{
    DeviceManager deviceManager;
    int32_t ret = deviceManager.Init(nullptr);
    EXPECT_EQ(ret, RET_ERR);
    ret = deviceManager.OnInit(nullptr);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: IntentionDeviceManagerTest02
 * @tc.desc: test Init
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest02, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    int32_t initret = deviceManager.Init(contextTest);
    EXPECT_NE(initret, RET_OK);
}


/**
 * @tc.name: IntentionDeviceManagerTest03
 * @tc.desc: test OnInit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest03, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    int32_t onInitret = deviceManager.OnInit(contextTest);
    EXPECT_EQ(onInitret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest04
 * @tc.desc: test Enable and Disable
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest04, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    deviceManager.OnInit(contextTest);
    int32_t ret = deviceManager.Enable();
    EXPECT_NE(ret, RET_OK);
    ret = deviceManager.Disable();
    EXPECT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest05
 * @tc.desc: test ParseDeviceId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest05, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::string devNode = "event123";
    int32_t expectedDeviceId = 123;
    int32_t actualDeviceId = deviceManager.ParseDeviceId(devNode);
    EXPECT_EQ(actualDeviceId, expectedDeviceId);
    devNode = "invalid_device";
    actualDeviceId = deviceManager.ParseDeviceId(devNode);
    EXPECT_EQ(actualDeviceId, RET_ERR);
}

/**
 * @tc.name: IntentionDeviceManagerTest06
 * @tc.desc: test FindDevice
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest06, TestSize.Level1)
{
    DeviceManager deviceManager;
    deviceManager.AddDevice(TEST_DEV_NODE);
    std::shared_ptr<IDevice> foundDevice = deviceManager.FindDevice(TEST_DEV_NODE);
    EXPECT_EQ(foundDevice, nullptr);
    std::shared_ptr<IDevice> removeDevice = deviceManager.RemoveDevice(TEST_DEV_NODE);
    EXPECT_EQ(removeDevice, nullptr);
}

/**
 * @tc.name: IntentionDeviceManagerTest07
 * @tc.desc: test Dispatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest07, TestSize.Level1)
{
    std::shared_ptr<DeviceManager> deviceManager = std::make_shared<DeviceManager>();
    uint32_t events = EPOLLIN | EPOLLOUT;
    struct epoll_event ev {};
    ev.events = events;
    ev.data.ptr = nullptr; 
    ASSERT_NO_FATAL_FAILURE(deviceManager->Dispatch(ev));
}

/**
 * @tc.name: IntentionDeviceManagerTest08
 * @tc.desc: test GetDevice
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest08, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    deviceManager.Init(contextTest);
    std::shared_ptr<IDevice> result = deviceManager.GetDevice(1);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: IntentionDeviceManagerTest09
 * @tc.desc: test RetriggerHotplug
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest09, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    deviceManager.Init(contextTest);
    std::weak_ptr<IDeviceObserver> weakObserver = std::weak_ptr<IDeviceObserver>();
    ASSERT_NO_FATAL_FAILURE(deviceManager.RetriggerHotplug(weakObserver));
    int32_t ret = deviceManager.OnRetriggerHotplug(weakObserver);
    EXPECT_NE(ret, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest010
 * @tc.desc: test AddDeviceObserver
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest010, TestSize.Level1)
{
    DeviceManager deviceManager;
    std::shared_ptr<IContextTest> context = std::make_shared<IContextTest>();
    IContext *contextTest = context.get();
    deviceManager.Init(contextTest);
    std::weak_ptr<IDeviceObserver> weakObserver = std::weak_ptr<IDeviceObserver>();
    int32_t ret = deviceManager.AddDeviceObserver(weakObserver);
    EXPECT_NE(ret, RET_OK);
    int32_t tmp = deviceManager.OnAddDeviceObserver(weakObserver);
    EXPECT_NE(tmp, RET_OK);
    ASSERT_NO_FATAL_FAILURE(deviceManager.RemoveDeviceObserver(weakObserver));
    int32_t result = deviceManager.OnRemoveDeviceObserver(weakObserver);
    EXPECT_NE(result, RET_OK);
}

/**
 * @tc.name: IntentionDeviceManagerTest011
 * @tc.desc: test AnyOf
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IntentionDeviceManagerTest, IntentionDeviceManagerTest011, TestSize.Level1)
{
    DeviceManager deviceManager;
    auto pred = [](std::shared_ptr<IDevice> device) {
        return device != nullptr;
    };
    EXPECT_FALSE(deviceManager.AnyOf(pred));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS