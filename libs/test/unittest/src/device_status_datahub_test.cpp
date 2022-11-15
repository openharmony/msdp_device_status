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

#include <cstdio>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"

#include "devicestatus_common.h"
#include "devicestatus_msdp_client_impl.h"
#include "devicestatus_msdp_mock.h"
#include "sensor_data_callback.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;

namespace {
    std::shared_ptr<SensorDataCallback> g_datahub;
}

class DeviceStatusDatahubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DeviceStatusDatahubTest::SetUpTestCase()
{
    g_datahub = SensorDataCallback::GetInstance();
    g_datahub->Init();
}

void DeviceStatusDatahubTest::TearDownTestCase()
{
    g_datahub = nullptr;
}

void DeviceStatusDatahubTest::SetUp() {}

void DeviceStatusDatahubTest::TearDown() {}

void SensorAccelCallbackData(int32_t sensorTypeId, AccelData* data)
{
    GTEST_LOG_(INFO) << sensorTypeId;
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest001 start";
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = g_datahub->SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest001 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest002 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 12;
    data.z = 163;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest002 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest003 start";
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    bool ret = g_datahub->SubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnsubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest003 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest004 start";
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    bool ret = g_datahub->SubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnsubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest004 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest005 start";
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = g_datahub->SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest005 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest006 start";
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = nullptr;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = g_datahub->SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest006 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest007 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest007 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest008 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t sensorTypeIdt = SENSOR_TYPE_ID_TEMPERATURE;
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeIdt);
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest008 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest009 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    ret = g_datahub->UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest009 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest010 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    ret = g_datahub->UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest010 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest011 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_TEMPERATURE;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest011 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest012 start";
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    bool ret =  g_datahub->RegisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest012 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest013 start";
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    bool ret =  g_datahub->RegisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest013 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest014 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest014 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest015 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 12;
    data.z = 15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    ASSERT_TRUE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest015 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest016 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -10;
    data.y = -12;
    data.z = -15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    ASSERT_TRUE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest016 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest017 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 163;
    data.y = 12;
    data.z = 15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest017 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest018 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -163;
    data.y = -12;
    data.z = -15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest018 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest019 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 163;
    data.z = 15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest019 end";
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest020 start";
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret =  g_datahub->RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -10;
    data.y = -163;
    data.z = -15;
    ret = g_datahub->PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = g_datahub->UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusDatahubTest020 end";
}
