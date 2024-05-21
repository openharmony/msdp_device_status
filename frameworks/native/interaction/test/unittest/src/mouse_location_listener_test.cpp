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

#include <vector>

#include <unistd.h>

#include "accesstoken_kit.h"
#include "device_manager.h"
#include "dm_device_info.h"
#include <gtest/gtest.h>
#include "nativetoken_kit.h"
#include "pointer_event.h"
#include "securec.h"
#include "token_setproc.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "i_event_listener.h"
#include "interaction_manager.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "MouseLocationListenerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
const std::string DM_SERVICE_ACCESS_PERMISSION { "ohos.permission.ACCESS_SERVICE_DM" };
const std::string DM_SERVICE_ACCESS_NEWPERMISSION { "ohos.permission.DISTRIBUTED_DATASYNC" };
const std::string PKG_NAME_PREFIX { "DBinderBus_Dms_" };
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
#define DSTB_HARDWARE DistributedHardware::DeviceManager::GetInstance()
} // namespace

class MouseLocationListenerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static std::string GetLocalNetworkId();
};


void MouseLocationListenerTest::SetUpTestCase() {}

void MouseLocationListenerTest::SetUp()
{
    const int32_t permsNum = 2;
    const char *perms[permsNum];
    perms[0] = DM_SERVICE_ACCESS_NEWPERMISSION.c_str();
    perms[1] = DM_SERVICE_ACCESS_PERMISSION.c_str();
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = permsNum,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "MouseLocationListenerTest",
        .aplStr = "system_core",
    };
    uint64_t tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void MouseLocationListenerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

std::string MouseLocationListenerTest::GetLocalNetworkId()
{
    auto packageName = PKG_NAME_PREFIX + std::to_string(getpid());
    OHOS::DistributedHardware::DmDeviceInfo dmDeviceInfo;
    if (int32_t errCode = DSTB_HARDWARE.GetLocalDeviceInfo(packageName, dmDeviceInfo); errCode != RET_OK) {
        FI_HILOGE("GetLocalBasicInfo failed, errCode:%{public}d", errCode);
        return {};
    }
    FI_HILOGD("LocalNetworkId:%{public}s", Utility::Anonymize(dmDeviceInfo.networkId).c_str());
    return dmDeviceInfo.networkId;
}

class EventListener : public IEventListener {
    void OnMouseLocationEvent(const std::string &networkId, const Event &event) override;
};

void EventListener::OnMouseLocationEvent(const std::string &networkId, const Event &event)
{
    FI_HILOGI("NetworkId:%{public}s, DisplayX:%{public}d, displayY:%{public}d,"
        "displayWidth:%{public}d, displayHeight:%{public}d", Utility::Anonymize(networkId).c_str(),
        event.displayX, event.displayY, event.displayWidth, event.displayHeight);
}

/**
 * @tc.name: RegisterEventListener_00
 * @tc.desc: Default NetworkId, Valid Listener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, RegisterEventListener_00, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    auto listener = std::make_shared<EventListener>();
    int32_t ret = InteractionManager::GetInstance()->RegisterEventListener(networkId, listener);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: RegisterEventListener_01
 * @tc.desc: Default NetworkId, NULL Listener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, RegisterEventListener_01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    int32_t ret = InteractionManager::GetInstance()->RegisterEventListener(networkId, nullptr);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, COMMON_PARAMETER_ERROR);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: UnregisterEventListener_00
 * @tc.desc: Local NetworkId, Valid Listener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, UnregisterEventListener_00, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    auto listener = std::make_shared<EventListener>();
    int32_t ret = InteractionManager::GetInstance()->RegisterEventListener(networkId, listener);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    ret = InteractionManager::GetInstance()->UnregisterEventListener(networkId, listener);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: UnregisterEventListener_01
 * @tc.desc: Local networkId, NULL Listener.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(MouseLocationListenerTest, UnregisterEventListener_01, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string networkId { "Default" };
    auto listener = std::make_shared<EventListener>();
    int32_t ret = InteractionManager::GetInstance()->RegisterEventListener(networkId, listener);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    ret = InteractionManager::GetInstance()->UnregisterEventListener(networkId, nullptr);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
