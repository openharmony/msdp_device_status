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

#include "common_event_adapter.h"
#include "common_event_observer.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "CommonEventAdapterTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
std::shared_ptr<ICommonEventObserver> g_observer { nullptr };
} // namespace

class CommonEventAdapterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};

void CommonEventAdapterTest::SetUpTestCase() {}

void CommonEventAdapterTest::SetUp() {}

void CommonEventAdapterTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

/**
 * @tc.name: CommonEventAdapterTest
 * @tc.desc: CommonEventAdapterTest001
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CommonEventAdapterTest, CommonEventAdapterTest001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    CommonEventAdapter commonEvent;
    int32_t ret = commonEvent.AddObserver(g_observer);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: CommonEventAdapterTest
 * @tc.desc: CommonEventAdapterTest002
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(CommonEventAdapterTest, CommonEventAdapterTest002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string commonevent = EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON;
    g_observer = CommonEventObserver::CreateCommonEventObserver(
        [this] (const std::string &event) {
            return ;
        }
    );
    ASSERT_NE(g_observer, nullptr);
    OHOS::AAFwk::Want want;
    EventFwk::CommonEventData event;
    ASSERT_NO_FATAL_FAILURE(g_observer->OnReceiveEvent(event));
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    event.SetWant(want);
    ASSERT_NO_FATAL_FAILURE(g_observer->OnReceiveEvent(event));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS