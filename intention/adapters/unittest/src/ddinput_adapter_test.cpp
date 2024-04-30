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

#include <future>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <unistd.h>

#include "accesstoken_kit.h"
#include <gtest/gtest.h>
#include "input_device.h"
#include "pointer_event.h"
#include "securec.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "i_dinput_adapter.h"
#include "dinput_adapter.h"
#include "drag_data.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#undef LOG_TAG
#define LOG_TAG "DDnputAdapterTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
const std::string SYSTEM_CORE { "system_core" };
uint64_t g_tokenID { 0 };
const char* g_cores[] = { "ohos.permission.INPUT_MONITORING" };
std::string defaultNetworkId { "Default NetworkId" };
} // namespace

class DDnputAdapterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void SetPermission(const std::string &level, const char** perms, size_t permAmount);
    static void RemovePermission();
};

void DDnputAdapterTest::SetPermission(const std::string &level, const char** perms, size_t permAmount)
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
        .processName = "DDnputAdapterTest",
        .aplStr = level.c_str(),
    };
    g_tokenID = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(g_tokenID);
    OHOS::Security::AccessToken::AccessTokenKit::AccessTokenKit::ReloadNativeTokenInfo();
}

void DDnputAdapterTest::RemovePermission()
{
    CALL_DEBUG_ENTER;
    int32_t ret = OHOS::Security::AccessToken::AccessTokenKit::DeleteToken(g_tokenID);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to remove permission");
        return;
    }
}

void DDnputAdapterTest::SetUpTestCase() {}

void DDnputAdapterTest::SetUp() {}

void DDnputAdapterTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

/**
 * @tc.name: TestNeedFilterOut
 * @tc.desc: Test NeedFilterOut
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, TestNeedFilterOut, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    bool ret = dinputAdapter->IsNeedFilterOut(defaultNetworkId, std::move(businessEvent));
    ASSERT_TRUE(ret);
    RemovePermission();
}

/**
 * @tc.name: TestStartRemoteInput
 * @tc.desc: Test StartRemoteInput
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, StartRemoteInput, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->StartRemoteInput(defaultNetworkId, defaultNetworkId, inputDeviceDhids, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestStopRemoteInput
 * @tc.desc: Test StopRemoteInput
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, StopRemoteInput, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->StopRemoteInput(defaultNetworkId, defaultNetworkId, inputDeviceDhids, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestStopRemoteInput_0
 * @tc.desc: Test StopRemoteInput_0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, StopRemoteInput_0, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->StopRemoteInput(defaultNetworkId, inputDeviceDhids, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestPrepareRemoteInput
 * @tc.desc: Test PrepareRemoteInput
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, PrepareRemoteInput, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->PrepareRemoteInput(defaultNetworkId, defaultNetworkId, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestUnPrepareRemoteInput
 * @tc.desc: Test UnPrepareRemoteInput
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, UnPrepareRemoteInput, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->UnPrepareRemoteInput(defaultNetworkId, defaultNetworkId, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestPrepareRemoteInput_0
 * @tc.desc: Test PrepareRemoteInput_0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, PrepareRemoteInput_0, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->PrepareRemoteInput(defaultNetworkId, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestUnPrepareRemoteInput_0
 * @tc.desc: Test UnPrepareRemoteInput_0
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, UnPrepareRemoteInput_0, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    IDInputAdapter::BusinessEvent businessEvent;
    DInputAdapter::DInputCallback callback = [] (bool param) {
        FI_HILOGI("On callback");
    };
    std::vector<std::string> inputDeviceDhids {};
    bool ret = dinputAdapter->UnPrepareRemoteInput(defaultNetworkId, callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestRegisterSessionStateCb
 * @tc.desc: Test RegisterSessionStateCb
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, RegisterSessionStateCb, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    auto callback = [] (uint32_t param) {
        FI_HILOGI("On callback");
    };
    bool ret = dinputAdapter->RegisterSessionStateCb(callback);
    ASSERT_EQ(ret, RET_OK);
    RemovePermission();
}

/**
 * @tc.name: TestUnPrepareRemoteInput_1
 * @tc.desc: Test UnPrepareRemoteInput_1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DDnputAdapterTest, UnPrepareRemoteInput_1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SetPermission(SYSTEM_CORE, g_cores, sizeof(g_cores) / sizeof(g_cores[0]));
    std::shared_ptr<IDInputAdapter> dinputAdapter = std::make_shared<DInputAdapter>(nullptr);
    dinputAdapter->UnregisterSessionStateCb();
    RemovePermission();
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
