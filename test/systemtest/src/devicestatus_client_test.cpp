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

#undef LOG_TAG
#define LOG_TAG "DeviceStatusClientTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

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
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    ActivityEvent activityEvent = ActivityEvent::EVENT_INVALID;
    int32_t result = stationaryMgr->SubscribeCallback(Type::TYPE_VERTICAL_POSITION, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_VERTICAL_POSITION, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);

    result = stationaryMgr->SubscribeCallback(Type::TYPE_INVALID, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_INVALID, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);

    result = stationaryMgr->SubscribeCallback(Type::TYPE_ABSOLUTE_STILL, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_ABSOLUTE_STILL, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);

    result = stationaryMgr->SubscribeCallback(Type::TYPE_HORIZONTAL_POSITION, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_HORIZONTAL_POSITION, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);

    result = stationaryMgr->SubscribeCallback(Type::TYPE_LID_OPEN, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_LID_OPEN, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);

    result = stationaryMgr->SubscribeCallback(Type::TYPE_MAX, activityEvent, latency, cb);
    ASSERT_EQ(result, RET_OK);
    result = stationaryMgr->UnsubscribeCallback(Type::TYPE_MAX, activityEvent, cb);
    ASSERT_EQ(result, RET_OK);
}

/**
 * @tc.name: GetDeviceStatusDataTest001
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    GTEST_LOG_(INFO) << "INTENTION_FRAMEWORK, UNSUPPORTED ";
#else
    auto stationaryMgr = StationaryManager::GetInstance();
    OnChangedValue invalidValue = OnChangedValue::VALUE_INVALID;
    OnChangedValue exitValue = OnChangedValue::VALUE_EXIT;

    Type type = Type::TYPE_ABSOLUTE_STILL;
    Data data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);

    type = Type::TYPE_VERTICAL_POSITION;
    data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);

    type = Type::TYPE_HORIZONTAL_POSITION;
    data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);

    type = Type::TYPE_LID_OPEN;
    data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);

    type = Type::TYPE_INVALID;
    data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);

    type = static_cast<Type>(10);
    data = stationaryMgr->GetDeviceStatusData(type);
    EXPECT_TRUE(data.type == type && data.value >= invalidValue && data.value <= exitValue);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
