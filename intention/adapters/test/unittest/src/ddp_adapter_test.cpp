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


#define private public
#define protected public

#include <future>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include "pointer_event.h"
#include "securec.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "distributed_device_profile_client.h"
#include "ddp_adapter_impl.h"
#include "ddp_adapter.h"
#include "dp_change_listener.h"


#undef LOG_TAG
#define LOG_TAG "DdpAdapterTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
    std::string networkId { "networkId" };
} // namespace

class DdpAdapterTest : public testing::Test {
public:
    void SetUp();
    static void SetUpTestCase();
};

void DdpAdapterTest::SetUpTestCase() {}

void DdpAdapterTest::SetUp() {}

/**
 * @tc.name: TestUpdateCrossingSwitchState
 * @tc.desc: Test UpdateCrossingSwitchState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestUpdateCrossingSwitchState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    bool state = false;
    int32_t ret = dDPAdapterImpl.UpdateCrossingSwitchState(state);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestGetCrossingSwitchState
 * @tc.desc: Test GetCrossingSwitchState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetCrossingSwitchState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    auto udId = dDPAdapterImpl.GetLocalUdId();
    bool state = true;
    int32_t ret = dDPAdapterImpl.GetCrossingSwitchState(udId, state);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: TestSetProperty1
 * @tc.desc: Test SetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestSetProperty1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const std::string propertyName = "int_property";
    const int32_t propertyValue = 12345;
    int32_t ret = dDPAdapterImpl.SetProperty(propertyName, propertyValue);
    ASSERT_EQ(ret, RET_OK);
}


/**
 * @tc.name: TestSetProperty
 * @tc.desc: Test SetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestSetProperty2, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const std::string propertyName = "int_property";
    const bool propertyValue = true;
    int32_t ret = dDPAdapterImpl.SetProperty(propertyName, propertyValue);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: TestSetProperty
 * @tc.desc: Test SetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestSetProperty3, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const std::string propertyName = "int_property";
    const std::string propertyValue = "12345";
    int32_t ret = dDPAdapterImpl.SetProperty(propertyName, propertyValue);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: TestPutProfile
 * @tc.desc: Test PutProfile
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestPutProfile, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    int32_t ret = dDPAdapterImpl.PutProfile();
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestPutServiceProfile
 * @tc.desc: Test PutProfile
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestPutServiceProfile, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    dDPAdapterImpl.PutServiceProfile();
}

/**
 * @tc.name: TestGetProperty_1
 * @tc.desc: Test GetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetProperty_1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const auto localUdId = dDPAdapterImpl.GetLocalUdId();
    const std::string name = "name";
    bool value = true;
    int32_t ret = dDPAdapterImpl.GetProperty(localUdId, name, value);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestGetProperty_2
 * @tc.desc: Test GetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetProperty_2, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const auto localUdId = dDPAdapterImpl.GetLocalUdId();
    const std::string name = "name";
    int32_t value = 123;
    int32_t ret = dDPAdapterImpl.GetProperty(localUdId, name, value);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestGetProperty_3
 * @tc.desc: Test GetProperty
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetProperty_3, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const auto localUdId = dDPAdapterImpl.GetLocalUdId();
    const std::string name = "name";
    std::string value = "value";
    int32_t ret = dDPAdapterImpl.GetProperty(localUdId, name, value);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestGetNetworkIdByUdId
 * @tc.desc: Test GetNetworkIdByUdId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetNetworkIdByUdId, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const auto localUdId = dDPAdapterImpl.GetLocalUdId();
    dDPAdapterImpl.GetNetworkIdByUdId(localUdId);
}

/**
 * @tc.name: TestGetUdIdByNetworkId
 * @tc.desc: Test GetUdIdByNetworkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGetUdIdByNetworkId, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    dDPAdapterImpl.GetUdIdByNetworkId(networkId);
}

/**
 * @tc.name: TestUnregisterProfileListener
 * @tc.desc: Test UnregisterProfileListener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestUnregisterProfileListener, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    int32_t ret = dDPAdapterImpl.UnregisterProfileListener(networkId);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestUpdateCrossingSwitchState_1
 * @tc.desc: Test UpdateCrossingSwitchState_1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestUpdateCrossingSwitchState_1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    bool state = true;
    int32_t ret = dDPAdapterImpl.UpdateCrossingSwitchState(state);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestPutCharacteristicProfile
 * @tc.desc: Test PutCharacteristicProfile
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestPutCharacteristicProfile, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    const std::string profileStr = "profileStr";
    int32_t ret = dDPAdapterImpl.PutCharacteristicProfile(profileStr);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestDDPAdapterAddWatch
 * @tc.desc: Test AddWatch
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterAddWatch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    DDPAdapter dDPAdapter;
    auto networkId = dDPAdapter.GetLocalNetworkId();
    dDPAdapter.AddWatch(networkId);
    dDPAdapter.RemoveWatch(networkId);
}

/**
 * @tc.name: TestDDPAdapterGetLocalNetworkId
 * @tc.desc: Test GetLocalNetworkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterGetLocalNetworkId, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    DDPAdapter dDPAdapter;
    auto networkId = dDPAdapter.GetLocalNetworkId();
    dDPAdapter.OnProfileChanged(networkId);
}

/**
 * @tc.name: TestDDPAdapterGetLocalUdId
 * @tc.desc: Test GetLocalUdId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterGetLocalUdId, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    auto udId = dDPAdapterImpl.GetLocalUdId();
    DDPAdapter dDPAdapter;
    dDPAdapter.GetNetworkIdByUdId(udId);
}

/**
 * @tc.name: TestDDPAdapterOnProfileChanged_1
 * @tc.desc: Test OnProfileChanged_1
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterOnProfileChanged_1, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    DDPAdapter dDPAdapter;
    auto networkId = dDPAdapter.GetLocalNetworkId();
    dDPAdapter.OnProfileChanged(networkId);
}

/**
 * @tc.name: TestDDPAdapterOnProfileChanged_2
 * @tc.desc: Test OnProfileChanged_2
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterOnProfileChanged_2, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    auto udId = dDPAdapterImpl.GetLocalUdId();
    DDPAdapter dDPAdapter;
    dDPAdapter.GetNetworkIdByUdId(udId);
}

/**
 * @tc.name: TestDDPAdapterGetUdIdByNetworkId
 * @tc.desc: Test GetUdIdByNetworkId
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterGetUdIdByNetworkId, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    DDPAdapter dDPAdapter;
    auto networkId = dDPAdapter.GetLocalNetworkId();
    dDPAdapter.GetUdIdByNetworkId(networkId);
}

/**
 * @tc.name: TestDDPAdapterUpdateCrossingSwitchState
 * @tc.desc: Test UpdateCrossingSwitchState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterUpdateCrossingSwitchState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapter dDPAdapter;
    bool state = false;
    int32_t ret = dDPAdapter.UpdateCrossingSwitchState(state);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: TestDDPAdapterGetCrossingSwitchState
 * @tc.desc: Test GetCrossingSwitchState
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestDDPAdapterGetCrossingSwitchState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapter dDPAdapter;
    bool state = true;
    DDPAdapterImpl dDPAdapterImpl;
    auto udId = dDPAdapterImpl.GetLocalUdId();
    int32_t ret = dDPAdapter.GetCrossingSwitchState(udId, state);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: TestGenerateProfileStr
 * @tc.desc: Test GenerateProfileStr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DdpAdapterTest, TestGenerateProfileStr, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DDPAdapterImpl dDPAdapterImpl;
    std::string state;
    int32_t ret = dDPAdapterImpl.GenerateProfileStr(state);
    ASSERT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS