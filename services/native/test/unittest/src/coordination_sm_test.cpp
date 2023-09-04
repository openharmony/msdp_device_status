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

#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "fi_log.h"
#include "nativetoken_kit.h"
#include "nocopyable.h"
#include "token_setproc.h"

using namespace ::OHOS;
using namespace ::OHOS::Security::AccessToken;

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationSMTest" };
} // namespace

class CoordinationSMTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp();
    void TearDown() {}
    void AddPermission();
    void SetAceessTokenPermission(const std::string &processName, const char** perms, size_t permCount);
};

void CoordinationSMTest::SetUp() {
    AddPermission();
}

void CoordinationSMTest::AddPermission()
{
    const char** perms = new (std::nothrow) const char* [1];
    CHKPV(perms);
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    SetAceessTokenPermission("CoordinationSMTest", perms, 1);
    delete []perms;
}

void CoordinationSMTest::SetAceessTokenPermission(const std::string &processName, const char** perms, size_t permCount)
{
    if (perms == nullptr || permCount == 0) {
        return;
    }
    uint64_t tokenId;
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permCount,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .processName = processName.c_str(),
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/**
 * @tc.name: CoordinationSMTest
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
}

/**
 * @tc.name: CoordinationSMTest
 * @tc.desc: test abnormal GetCoordinationState when local NetworkId is empty
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = COOR_SM->GetCoordinationState("");
    ASSERT_TRUE(ret == static_cast<int32_t>(CoordinationMessage::PARAMETER_ERROR));
}

/**
 * @tc.name: CoordinationSMTest
 * @tc.desc: test normal GetCoordinationState when local NetworkId is correct
 * @tc.type: FUNC
 */
HWTEST_F(CoordinationSMTest, CoordinationSMTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    int32_t ret = COOR_SM->GetCoordinationState(localNetworkId);
    ASSERT_TRUE(ret == 0);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS