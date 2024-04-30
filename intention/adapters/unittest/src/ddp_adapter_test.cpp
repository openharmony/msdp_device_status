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
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS