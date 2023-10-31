/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "devicestatus_callback_stub.h"
#include "devicestatus_define.h"
#include "stationary_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, ::OHOS::Msdp::MSDP_DOMAIN_ID, "DeviceStatusClientTest" };
} // namespace

class DeviceStatusClientTest : public testing::Test {
public:
    class DeviceStatusClientTestCallback : public DeviceStatusCallbackStub {
    public:
        DeviceStatusClientTestCallback() {};
        virtual ~DeviceStatusClientTestCallback() {};
        virtual void OnDeviceStatusChanged(const Data& devicestatusData);
    };
};

void DeviceStatusClientTest::DeviceStatusClientTestCallback::OnDeviceStatusChanged(const
    Data& devicestatusData)
{
    GTEST_LOG_(INFO) << "DeviceStatusClientTestCallback type: " << devicestatusData.type;
    GTEST_LOG_(INFO) << "DeviceStatusClientTestCallback value: " << devicestatusData.value;
    EXPECT_TRUE(devicestatusData.type == Type::TYPE_VERTICAL_POSITION &&
        devicestatusData.value >= OnChangedValue::VALUE_INVALID &&
        devicestatusData.value <= OnChangedValue::VALUE_EXIT) << "DeviceStatusClientTestCallback failed";
}

/**
 * @tc.name: DeviceStatusCallbackTest001
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    Type type = Type::TYPE_VERTICAL_POSITION;
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusCallbackTest002
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    Type type = Type::TYPE_INVALID;
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusCallbackTest003
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    Type type = Type::TYPE_ABSOLUTE_STILL;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusCallbackTest004
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest004, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_HORIZONTAL_POSITION;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusCallbackTest005
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest005, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_LID_OPEN;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusCallbackTest006
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest006, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_MAX;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    sptr<IRemoteDevStaCallback> cb = new (std::nothrow) DeviceStatusClientTestCallback();
    ASSERT_NE(cb, nullptr);
    auto stationaryMgr = StationaryManager::GetInstance();
    GTEST_LOG_(INFO) << "Start register";
    int32_t ret = RET_OK;
    ret = stationaryMgr->SubscribeCallback(type, event, latency, cb);
    ASSERT_EQ(ret, RET_OK);
    GTEST_LOG_(INFO) << "Cancel register";
    ret = stationaryMgr->UnsubscribeCallback(type, event, cb);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: GetDeviceStatusDataTest007
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest007, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_ABSOLUTE_STILL;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_ABSOLUTE_STILL &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusData failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest008
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest008, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_VERTICAL_POSITION;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_VERTICAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusData failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest009
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest009, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_HORIZONTAL_POSITION;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_HORIZONTAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusData failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest010
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest010, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_LID_OPEN;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_LID_OPEN &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest004 failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest011
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest011, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_INVALID;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_INVALID &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest005 failed";
}

/**
 * @tc.name: GetDeviceStatusDataTest012
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest012, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    Type type = static_cast<Type>(10);
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_INVALID &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT);
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest006 failed";
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
