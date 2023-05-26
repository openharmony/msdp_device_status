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

#include <memory>

#include <gtest/gtest.h>

#include "accesstoken_kit.h"

#define private public
#include "devicestatus_algorithm_manager.h"
#include "sensor_agent_type.h"
#include "sensor_data_callback.h"
#undef private
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
    AlgoAbsoluteStill still;
    bool ret = still.Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    still.Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    still.RegisterCallback(callback_);
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
    AlgoHorizontal horizontal;
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = horizontal.Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal.RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    horizontal.Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest002 end";
}

/**
 * @tc.name: DeviceStatusVeriticalTest
 * @tc.desc: test devicestatus Veritical Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AbsolutstillTest003 start";
    AlgoVertical vertical;
    bool ret = vertical.Init(TYPE_INVALID);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical.RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    vertical.Unsubscribe(sensorTypeId);
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest004 start";
    std::shared_ptr<AlgoMgr> g_manager = std::make_shared<AlgoMgr>();
    int32_t ret = g_manager->UnregisterCallback();
    GTEST_LOG_(INFO) << "10";
    EXPECT_TRUE(ret == ERR_OK);
    bool result = g_manager->StartSensor(Type::TYPE_LID_OPEN);
    EXPECT_FALSE(result);
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest005 start";
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest006 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoAbsoluteStill still;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    still.RegisterCallback(callback_);
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest007 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoHorizontal horizontal;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal.RegisterCallback(callback_);
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest008 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoVertical vertical;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical.RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest008 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test DisableCount
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest009 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoVertical vertical;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical.RegisterCallback(callback_);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    ret = g_manager->DisableCount(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest009 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest010 start";
    int32_t typeError = 5;
    int32_t ret = g_manager->Enable(static_cast<Type>(typeError));
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest011 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest011 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest012 start";
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest012 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest013 start";
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
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
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest014 start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest014 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest015 start";
    int32_t typeError = 5;
    bool result = g_manager->StartSensor(static_cast<Type>(typeError));
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest015 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest016 start";
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_FALSE(result);
    g_manager->GetSensorTypeId(Type::TYPE_INVALID);
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest016 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest017 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_NONE;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ret += 1;
    ASSERT_TRUE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest017 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest018 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_GYROSCOPE;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest018 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId004
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest019 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_MAX;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest019 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId005
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest020 start";
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t typeError = -100;
    int32_t ret = g_manager->CheckSensorTypeId(static_cast<SensorTypeId>(typeError));
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest020 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest021 start";
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_HORIZONTAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest021 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest022 start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_VERTICAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest022 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest023, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest023 start";
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_MAX);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest023 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest024, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest024 start";
    int32_t ret = g_manager->Enable(Type::TYPE_MAX);
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest024 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest025, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest025 start";
    g_manager->callAlgoNum_[Type::TYPE_ABSOLUTE_STILL] = 2;
    int32_t ret = g_manager->Disable(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest025 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest026, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest026 start";
    int32_t ret = g_manager->Enable(Type::TYPE_MAX);
    ASSERT_TRUE(ret);
    g_manager->callAlgoNum_[Type::TYPE_MAX] = 1;
    ret = g_manager->Disable(Type::TYPE_MAX);
    EXPECT_TRUE(ret == RET_OK);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest026 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest027, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest027 start";
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_MAX);
    EXPECT_TRUE(ret == false);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest027 end";
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest028, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest028 start";
    SensorDataCallback::GetInstance().user_.callback = nullptr;
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(ret == RET_ERR);
    GTEST_LOG_(INFO) << "DeviceStatusAlgorithmTest028 end";
}