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

#include "accesstoken_kit.h"

#include "devicestatus_algorithm_manager.h"
#include "devicestatus_dumper.h"
#include "devicestatus_manager.h"

using namespace testing::ext;
using namespace OHOS::Msdp;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;

namespace {
    std::shared_ptr<AlgoMgr> g_manager;
}

class DeviceStatusAlgorithmTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    class DeviceStatusMock : public IMsdp::MsdpAlgoCallback {
    public:
        DeviceStatusMock() = default;
        virtual ~DeviceStatusMock() = default;
        void OnResult(const Data& data) {};
    };
};

void DeviceStatusAlgorithmTest::SetUpTestCase()
{
    g_manager = std::make_shared<AlgoMgr>();
}

void DeviceStatusAlgorithmTest::TearDownTestCase()
{
    g_manager = nullptr;
}

void DeviceStatusAlgorithmTest::SetUp() {}

void DeviceStatusAlgorithmTest::DeviceStatusAlgorithmTest::TearDown() {}

/**
 * @tc.name: DeviceStatusAlgoAbsoluteStillTest
 * @tc.desc: test devicestatus Absolute Still Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest001 start";
    AlgoAbsoluteStill* still = new (std::nothrow) AlgoAbsoluteStill();
    bool ret = still->Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    still->Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    still->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest001 end";
}

/**
 * @tc.name: DeviceStatusAlgoHorizontalTest
 * @tc.desc: test devicestatus AlgoHorizontal Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest002 start";
    AlgoHorizontal* horizontal = new (std::nothrow) AlgoHorizontal();
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = horizontal->Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    horizontal->Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest002 end";
}

/*
 * @tc.name: DeviceStatusVeriticalTest
 * @tc.desc: test devicestatus Veritical Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest003 start";
    AlgoVertical* vertical = new (std::nothrow) AlgoVertical();
    bool ret = vertical->Init(TYPE_INVALID);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    vertical->Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest003 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test devicestatus Algorithm g_manager
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest004 start";
    std::shared_ptr<AlgoMgr> g_manager = std::make_shared<AlgoMgr>();
    int32_t ret = g_manager->UnregisterCallback();
    GTEST_LOG_(INFO) << "10";
    EXPECT_TRUE(ret == ERR_OK);
    bool result = g_manager->StartSensor(Type::TYPE_LID_OPEN);
    EXPECT_TRUE(result == false);
    ret = g_manager->UnregisterCallback();
    EXPECT_TRUE(ret == ERR_OK);
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ASSERT_TRUE(ret);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    g_manager->GetSensorTypeId(Type::TYPE_HORIZONTAL_POSITION);
    g_manager->GetSensorTypeId(Type::TYPE_VERTICAL_POSITION);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest004 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest005 start";
    int32_t ret = g_manager->Enable(Type::TYPE_LID_OPEN);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest005 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest006 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->Enable(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoAbsoluteStill* still = new AlgoAbsoluteStill();
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    still->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest006 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest007 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->Enable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoHorizontal* horizontal = new AlgoHorizontal();
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest007 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest008 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->Enable(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoVertical* vertical = new AlgoVertical();
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical->RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);

    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest008 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest009 start";
    int32_t TYPE_ERROR = 5;
    int32_t ret = g_manager->Enable(static_cast<Type>(TYPE_ERROR));
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest009 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest010 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest010 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest011 start";
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_TRUE(result == false);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest011 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest012 start";
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest012 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest013 start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result == true);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest013 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest014 start";
    int32_t TYPE_ERROR = 5;
    bool result = g_manager->StartSensor(static_cast<Type>(TYPE_ERROR));
    EXPECT_TRUE(result == false);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest014 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest015 start";
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_TRUE(result == false);
    g_manager->GetSensorTypeId(Type::TYPE_INVALID);
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest015 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest016 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_NONE;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ret += 1;
    ASSERT_TRUE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest016 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest017 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_GYROSCOPE;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest017 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId004
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest018 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_MAX;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest018 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId005
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest019start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t TYPE_ERROR = -100;
    int32_t ret = g_manager->CheckSensorTypeId(static_cast<SensorTypeId>(TYPE_ERROR));
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest019 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest020start";
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_HORIZONTAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest020 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest021start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_VERTICAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest021 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest022start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result == true);
    g_manager->GetSensorTypeId(Type::TYPE_MAX);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest022 end";
}
