/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "i_stop_drag_listener.h"

#include <gtest/gtest.h>
#include <typeinfo>

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

class MockStopDragListener : public IStopDragListener {
public:
    MockStopDragListener() = default;
    ~MockStopDragListener() override = default;
    void OnDragEndMessage() override
    {
        callbackCalled = true;
    }
    bool callbackCalled { false };
};

class IStopDragListenerTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: IStopDragListenerTest001
 * @tc.desc: Test IStopDragListener interface - default constructor
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IStopDragListenerTest, IStopDragListenerTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    MockStopDragListener listener;
    EXPECT_FALSE(listener.callbackCalled);
}

/**
 * @tc.name: IStopDragListenerTest002
 * @tc.desc: Test IStopDragListener interface - OnDragEndMessage callback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IStopDragListenerTest, IStopDragListenerTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    MockStopDragListener listener;
    EXPECT_FALSE(listener.callbackCalled);
    listener.OnDragEndMessage();
    EXPECT_TRUE(listener.callbackCalled);
}

/**
 * @tc.name: IStopDragListenerTest003
 * @tc.desc: Test IStopDragListener interface - multiple callbacks
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IStopDragListenerTest, IStopDragListenerTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    MockStopDragListener listener;
    EXPECT_FALSE(listener.callbackCalled);
    listener.OnDragEndMessage();
    EXPECT_TRUE(listener.callbackCalled);
    listener.OnDragEndMessage();
    EXPECT_TRUE(listener.callbackCalled);
}

/**
 * @tc.name: IStopDragListenerTest004
 * @tc.desc: Test IStopDragListener interface - multiple instances
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IStopDragListenerTest, IStopDragListenerTest004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    MockStopDragListener listener1;
    MockStopDragListener listener2;
    EXPECT_FALSE(listener1.callbackCalled);
    EXPECT_FALSE(listener2.callbackCalled);
    listener1.OnDragEndMessage();
    EXPECT_TRUE(listener1.callbackCalled);
    EXPECT_FALSE(listener2.callbackCalled);
    listener2.OnDragEndMessage();
    EXPECT_TRUE(listener1.callbackCalled);
    EXPECT_TRUE(listener2.callbackCalled);
}

/**
 * @tc.name: IStopDragListenerTest005
 * @tc.desc: Test IStopDragListener interface - virtual destructor
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(IStopDragListenerTest, IStopDragListenerTest005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    IStopDragListener* listener = new MockStopDragListener();
    EXPECT_NE(listener, nullptr);
    delete listener;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
