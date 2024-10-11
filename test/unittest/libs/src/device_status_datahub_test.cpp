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

#include <cstdio>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"

#include "devicestatus_common.h"
#include "devicestatus_msdp_client_impl.h"
#include "devicestatus_msdp_mock.h"
#include "fi_log.h"
#include "sensor_data_callback.h"

#undef LOG_TAG
#define LOG_TAG "DeviceStatusDatahubTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

class DeviceStatusDatahubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DeviceStatusDatahubTest::SetUpTestCase()
{
    SENSOR_DATA_CB.Init();
}

void DeviceStatusDatahubTest::TearDownTestCase() {}

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
    CALL_TEST_DEBUG;
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 12;
    data.z = 163;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    bool ret = SENSOR_DATA_CB.SubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnsubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = SensorAccelCallbackData;
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    bool ret = SENSOR_DATA_CB.SubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnsubscribeSensorEvent(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR), callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    using SensorCallback = std::function<void(int32_t, AccelData*)>;
    SensorCallback callback = nullptr;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.SubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnsubscribeSensorEvent(sensorTypeId, callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest007, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest008, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t sensorTypeIdt = SENSOR_TYPE_ID_TEMPERATURE;
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeIdt);
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest009, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest010, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest011, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_TEMPERATURE;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest012, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t SENSOR_TYPE_ID_ERROR = 300;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest013, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t SENSOR_TYPE_ID_ERROR = -1;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(static_cast<SensorTypeId>(SENSOR_TYPE_ID_ERROR));
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest014, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest015, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 12;
    data.z = 15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest016, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -10;
    data.y = -12;
    data.z = -15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    ASSERT_TRUE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest017, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 163;
    data.y = 12;
    data.z = 15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest018, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -163;
    data.y = -12;
    data.z = -15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest019, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = 10;
    data.y = 163;
    data.z = 15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusDataCallbackTest
 * @tc.desc: test devicestatus Callback in Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusDatahubTest, DeviceStatusDatahubTest020, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = SENSOR_DATA_CB.RegisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
    AccelData data;
    data.x = -10;
    data.y = -163;
    data.z = -15;
    ret = SENSOR_DATA_CB.PushData(sensorTypeId, reinterpret_cast<uint8_t*>(&data));
    EXPECT_FALSE(ret);
    ret = SENSOR_DATA_CB.UnregisterCallbackSensor(sensorTypeId);
    ASSERT_TRUE(ret);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
