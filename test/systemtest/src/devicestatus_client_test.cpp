/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "stationary_manager.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
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
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest001 Enter");
    Type type = Type::TYPE_VERTICAL_POSITION;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest001 end");
}

/**
 * @tc.name: DeviceStatusCallbackTest002
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest002, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest002 Enter");
    Type type = Type::TYPE_INVALID;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest002 end");
}

/**
 * @tc.name: DeviceStatusCallbackTest003
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest003, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest003 Enter");
    Type type = Type::TYPE_ABSOLUTE_STILL;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest003 end");
}

/**
 * @tc.name: DeviceStatusCallbackTest004
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest004, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest004 Enter");
    Type type = Type::TYPE_HORIZONTAL_POSITION;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest004 end");
}

/**
 * @tc.name: DeviceStatusCallbackTest005
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest005, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest005 Enter");
    Type type = Type::TYPE_LID_OPEN;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest005 end");
}

/**
 * @tc.name: DeviceStatusCallbackTest006
 * @tc.desc: test devicestatus callback in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, DeviceStatusCallbackTest006, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest006 Enter");
    Type type = Type::TYPE_MAX;
    ActivityEvent event = ActivityEvent::EVENT_INVALID;
    ReportLatencyNs latency = ReportLatencyNs::Latency_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    sptr<IRemoteDevStaCallback> cb = new DeviceStatusClientTestCallback();
    GTEST_LOG_(INFO) << "Start register";
    StationaryMgr->SubscribeCallback(type, event, latency, cb);
    GTEST_LOG_(INFO) << "Cancell register";
    StationaryMgr->UnsubscribeCallback(type, event, cb);
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest006 end");
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest007, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest007 Enter");
    Type type = Type::TYPE_ABSOLUTE_STILL;
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_ABSOLUTE_STILL &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest007 end");
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest008, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest008 Enter");
    Type type = Type::TYPE_VERTICAL_POSITION;
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_VERTICAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest008 end");
}

/**
 * @tc.name: GetDeviceStatusDataTest
 * @tc.desc: test get devicestatus data in proxy
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest009, TestSize.Level0)
{
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest009 Enter");
    Type type = Type::TYPE_HORIZONTAL_POSITION;
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_HORIZONTAL_POSITION &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusData failed";
    DEV_HILOGD(SERVICE, "GetDeviceStatusDataTest009 end");
}

HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 Enter";
    Type type = Type::TYPE_LID_OPEN;
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_LID_OPEN &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusDataTest004 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest010 end";
}

HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 Enter";
    Type type = Type::TYPE_INVALID;
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
    GTEST_LOG_(INFO) << "type: " << data.type;
    GTEST_LOG_(INFO) << "value: " << data.value;
    EXPECT_TRUE(data.type == Type::TYPE_INVALID &&
        data.value >= OnChangedValue::VALUE_INVALID && data.value <= OnChangedValue::VALUE_EXIT)
        << "GetDeviceStatusDataTest005 failed";
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest011 end";
}

HWTEST_F(DeviceStatusClientTest, GetDeviceStatusDataTest012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetDeviceStatusDataTest012 Enter";
    Type type = static_cast<Type>(10);
    auto StationaryMgr = StationaryManager::GetInstance();
    Data data = StationaryMgr->GetDeviceStatusData(type);
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