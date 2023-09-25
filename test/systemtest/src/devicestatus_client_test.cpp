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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest001 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_VERTICAL_POSITION;
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest001 end";
}

/**
 * @tc.name: DeviceStatusCallbackTest002
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest002 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_INVALID;
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest002 end";
}

/**
 * @tc.name: DeviceStatusCallbackTest003
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest003 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_ABSOLUTE_STILL;
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest003 end";
}

/**
 * @tc.name: DeviceStatusCallbackTest004
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest004 enter";
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest004 end";
}

/**
 * @tc.name: DeviceStatusCallbackTest005
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest005 enter";
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest005 end";
}

/**
 * @tc.name: DeviceStatusCallbackTest006
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest006 enter";
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
    GTEST_LOG_(INFO) << "DeviceStatusCallbackTest006 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest007
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest007 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_ABSOLUTE_STILL;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_ABSOLUTE_STILL &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest007 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest008
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest008 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_VERTICAL_POSITION;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_VERTICAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest008 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest009
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest009 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_HORIZONTAL_POSITION;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_HORIZONTAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest009 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest010
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 enter";
    CALL_TEST_DEBUG;
    Type type = Type::TYPE_LID_OPEN;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_LID_OPEN &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusDataTest004 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest011
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 enter";
    Type type = Type::TYPE_INVALID;
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_INVALID &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusDataTest005 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 end";
}

/**
 * @tc.name: GetDeviceStatusDataTest012
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest012 enter";
    Type type = static_cast<Type>(10);
    auto stationaryMgr = StationaryManager::GetInstance();
    Data data = stationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_INVALID &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusDataTest006 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest012 end";
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
