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

#include <future>
#include <utility>
#include <vector>

#include <unistd.h>

#include "i_event_listener.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#undef LOG_TAG
#define LOG_TAG "MouseLocationListenerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
uint64_t g_tokenID { 0 };
const char* g_cores[] = { "ohos.permission.INPUT_MONITORING" };
const char* g_basics[] = { "ohos.permission.COOPERATE_MANAGER" };
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };  
} // namespace

class MouseLocationListenerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};


void MouseLocationListenerTest::SetUpTestCase() {}

void MouseLocationListenerTest::SetUp() {}

void MouseLocationListenerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

class EventListener : public IEventListener {
    void OnEvent(const Event &event) override ;
};

void EventListener::OnEvent(const Event &event) {
    FI_HILOGI("DisplayX:%{public}d, displayY:%{public}d, displayWidth:%{public}d, displayHeight:%{public}d",
        event.displayX, event.displayY, event.displayWidth, event.displayHeight); 
}

void MouseLocationListenerTest::SetPermission(const std::string &level, const char** perms, size_t permAmount)
{
    CALL_DEBUG_ENTER;
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
        .processName = "MouseLocationListenerTest",
        .aplStr = level.c_str(),
    };
    g_tokenID = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(g_tokenID);
    OHOS::Security::AccessToken::AccessTokenKit::AccessTokenKit::ReloadNativeTokenInfo();
}

void MouseLocationListenerTest::RemovePermission()
{
    CALL_DEBUG_ENTER;
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::DeleteToken(g_tokenID);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to remove permission");
        return;
    }
}

/**
 * @tc.name: RegisterEventListener_00
 * @tc.desc: Event Listener, local networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, RegisterEventListener_00, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    int32_t ret = InteractionManager::GetInstance()->RegisterEventListener(networkId, nullptr);
    ASSERT_EQ(ret, COMMON_PARAMETER_ERROR);
}

/**
 * @tc.name: UnregisterEventListener_01
 * @tc.desc: Event Listener, local networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, UnregisterEventListener_01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    auto listener = std::make_shared<EventListener>();
    int32_t ret = InteractionManager::GetInstance()->UnregisterEventListener(networkId, listener);
    ASSERT_EQ(ret, COMMON_PARAMETER_ERROR);
}

/**
 * @tc.name: RegisterEventListener_02
 * @tc.desc: Event Listener, remote networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, RegisterEventListener_02, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    int32_t ret = InteractionManager::GetInstance()->UnregisterEventListener(networkId, nullptr);
    ASSERT_EQ(ret, COMMON_PARAMETER_ERROR);
}

/**
 * @tc.name: UnregisterEventListener_03
 * @tc.desc: Event Listener, remote networkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, UnregisterEventListener_03, TestSize.Level1)
{
    CALL_TEST_DEBUG;
}

/**
 * @tc.name: RegisterEventListener_04
 * @tc.desc: Event Listener, invalid listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, RegisterEventListener_04, TestSize.Level1)
{
    CALL_TEST_DEBUG;
}

/**
 * @tc.name: UnregisterEventListener_05
 * @tc.desc: Event Listener, invalid listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, UnregisterEventListener_05, TestSize.Level1)
{
    CALL_TEST_DEBUG;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
