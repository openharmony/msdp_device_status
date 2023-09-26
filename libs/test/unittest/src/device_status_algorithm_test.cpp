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
#include <dlfcn.h>

#include <gtest/gtest.h>

#include "accesstoken_kit.h"

#define private public
#include "devicestatus_algorithm_manager.h"
#include "sensor_agent_type.h"
#include "sensor_data_callback.h"
#undef private
#include "devicestatus_dumper.h"
#include "devicestatus_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
std::shared_ptr<AlgoMgr> g_manager { nullptr };
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusAlgorithmTest" };
#ifdef __aarch64__
const std::string DEVICESTATUS_ALGO_LIB_PATH { "/system/lib64/libdevicestatus_algo.z.so" };
#else
const std::string DEVICESTATUS_ALGO_LIB_PATH { "/system/lib/libdevicestatus_algo.z.so" };
#endif
} // namespace

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
    int32_t LoadAlgoLibrary(const std::shared_ptr<MsdpAlgoHandle>& algoHandler);
    int32_t UnloadAlgoLibrary(const std::shared_ptr<MsdpAlgoHandle>& algoHandler);
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

int32_t DeviceStatusAlgorithmTest::LoadAlgoLibrary(const std::shared_ptr<MsdpAlgoHandle>& algoHandler)
{
    FI_HILOGI("Enter");
    if (algoHandler == nullptr) {
        FI_HILOGE("algoHandler is nullptr");
        return RET_ERR;
    }
    if (algoHandler->handle != nullptr) {
        FI_HILOGE("handle already exists");
        return RET_OK;
    }

    std::string dlName = DEVICESTATUS_ALGO_LIB_PATH;
    char libRealPath[PATH_MAX] = {};
    if (realpath(dlName.c_str(), libRealPath) == nullptr) {
        FI_HILOGE("Get absolute algoPath is error, errno:%{public}d", errno);
        return RET_ERR;
    }

    algoHandler->handle = dlopen(libRealPath, RTLD_LAZY);
    if (algoHandler->handle == nullptr) {
        FI_HILOGE("Cannot load library error:%{public}s", dlerror());
        return RET_ERR;
    }

    algoHandler->create = reinterpret_cast<IMsdp* (*)()>(dlsym(algoHandler->handle, "Create"));
    algoHandler->destroy = reinterpret_cast<void *(*)(IMsdp*)>(dlsym(algoHandler->handle, "Destroy"));
    if (algoHandler->create == nullptr || algoHandler->destroy == nullptr) {
        FI_HILOGE("%{public}s dlsym create or destroy failed", dlName.c_str());
        dlclose(algoHandler->handle);
        algoHandler->Clear();
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceStatusAlgorithmTest::UnloadAlgoLibrary(const std::shared_ptr<MsdpAlgoHandle>& algoHandler)
{
    FI_HILOGI("Enter");
    if (algoHandler == nullptr) {
        FI_HILOGE("algoHandler is nullptr");
        return RET_ERR;
    }
    if (algoHandler->handle == nullptr) {
        FI_HILOGE("handle is nullptr");
        return RET_ERR;
    }

    if (algoHandler->pAlgorithm != nullptr) {
        algoHandler->destroy(algoHandler->pAlgorithm);
        algoHandler->pAlgorithm = nullptr;
    }
    dlclose(algoHandler->handle);
    algoHandler->Clear();
    return RET_OK;
}

/**
 * @tc.name: DeviceStatusAlgoAbsoluteStillTest
 * @tc.desc: test devicestatus Absolute Still Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest001, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest001 start");
    AlgoAbsoluteStill still;
    bool ret = still.Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    still.Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
    still.RegisterCallback(callback);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgoHorizontalTest
 * @tc.desc: test devicestatus AlgoHorizontal Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest002, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest002 start");
    AlgoHorizontal horizontal;
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    bool ret = horizontal.Init(TYPE_INVALID);
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal.RegisterCallback(callback);
    ASSERT_TRUE(ret);
    horizontal.Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusVeriticalTest
 * @tc.desc: test devicestatus Veritical Algorithm
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest003, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest003 start");
    AlgoVertical vertical;
    bool ret = vertical.Init(TYPE_INVALID);
    int32_t sensorTypeId = SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
    ASSERT_TRUE(ret);
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
    vertical.RegisterCallback(callback);
    ASSERT_TRUE(ret);
    vertical.Unsubscribe(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test devicestatus Algorithm g_manager
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest004, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest004 start");
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
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest005, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest005 start");
    int32_t ret = g_manager->Enable(Type::TYPE_LID_OPEN);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest006, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest006 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoAbsoluteStill still;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
    still.RegisterCallback(callback);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest007, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest007 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->Enable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
    AlgoHorizontal horizontal;
    std::shared_ptr<DeviceStatusMsdpClientImpl> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
    horizontal.RegisterCallback(callback);
    ASSERT_TRUE(ret);
    ret = g_manager->Disable(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest008, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest008 start");
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
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test DisableCount
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest009, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest009 start");
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
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest010, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest010 start");
    int32_t typeError = 5;
    int32_t ret = g_manager->Enable(static_cast<Type>(typeError));
    ASSERT_TRUE(ret);
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    ret = g_manager->Enable(static_cast<Type>(typeError));
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest011, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest011 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest012, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest012 start");
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest013, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest013 start");
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest014, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest014 start");
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test StartSensor and UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest015, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest015 start");
    int32_t typeError = 5;
    bool result = g_manager->StartSensor(static_cast<Type>(typeError));
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest016, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest016 start");
    bool result = g_manager->StartSensor(Type::TYPE_INVALID);
    EXPECT_FALSE(result);
    g_manager->GetSensorTypeId(Type::TYPE_INVALID);
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest017, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest017 start");
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
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest018, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest018 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_MAX;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId004
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest019, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest019 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t sensorTypeId = SENSOR_TYPE_ID_MAX;
    int32_t ret = g_manager->CheckSensorTypeId(sensorTypeId);
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test CheckSensorTypeId005
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest020, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest020 start");
    bool result = g_manager->StartSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_ABSOLUTE_STILL);
    int32_t typeError = -100;
    int32_t ret = g_manager->CheckSensorTypeId(static_cast<SensorTypeId>(typeError));
    EXPECT_FALSE(ret);
    ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId001
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest021, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest021 start");
    bool result = g_manager->StartSensor(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_HORIZONTAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_HORIZONTAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId002
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest022, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest022 start");
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_VERTICAL_POSITION);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test GetSensorTypeId003
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest023, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest023 start");
    bool result = g_manager->StartSensor(Type::TYPE_VERTICAL_POSITION);
    EXPECT_TRUE(result);
    g_manager->GetSensorTypeId(Type::TYPE_MAX);
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_VERTICAL_POSITION);
    ret += 1;
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest024, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest024 start");
    int32_t ret = g_manager->Enable(Type::TYPE_MAX);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest025, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest025 start");
    g_manager->callAlgoNums_[Type::TYPE_ABSOLUTE_STILL] = 2;
    int32_t ret = g_manager->Disable(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Disable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest026, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest026 start");
    int32_t ret = g_manager->Enable(Type::TYPE_MAX);
    ASSERT_TRUE(ret);
    g_manager->callAlgoNums_[Type::TYPE_MAX] = 1;
    ret = g_manager->Disable(Type::TYPE_MAX);
    EXPECT_TRUE(ret == RET_OK);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest027, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest027 start");
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_MAX);
    EXPECT_TRUE(ret == false);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test UnregisterSensor
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest028, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest028 start");
    SENSOR_DATA_CB.user_.callback = nullptr;
    int32_t ret = g_manager->UnregisterSensor(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_TRUE(ret == RET_ERR);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test Enable
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest029, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest029 start.");
    std::shared_ptr<MsdpAlgoHandle> algo = std::make_shared<MsdpAlgoHandle>();
    int32_t ret = LoadAlgoLibrary(algo);
    ASSERT_EQ(ret, RET_OK);
    ASSERT_NE(algo->handle, nullptr);
    algo->pAlgorithm = algo->create();

    ret = algo->pAlgorithm->Enable(Type::TYPE_LID_OPEN);
    ASSERT_TRUE(ret);

    ret = UnloadAlgoLibrary(algo);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test RegisterCallback
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest030, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest030 start.");
    int32_t ret = g_manager->Enable(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_EQ(ret, RET_OK);

    std::shared_ptr<DeviceStatusMock> callback = std::make_shared<DeviceStatusMock>();
    ret = g_manager->RegisterCallback(callback);
    EXPECT_EQ(ret, RET_OK);

    ret = g_manager->Disable(Type::TYPE_ABSOLUTE_STILL);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test RegisterCallback
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest031, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest031 start.");
    int32_t ret = g_manager->Enable(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_EQ(ret, RET_OK);

    std::shared_ptr<DeviceStatusMock> callback = std::make_shared<DeviceStatusMock>();
    ret = g_manager->RegisterCallback(callback);
    EXPECT_EQ(ret, RET_OK);

    ret = g_manager->Disable(Type::TYPE_HORIZONTAL_POSITION);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DeviceStatusAlgorithmManagerTest
 * @tc.desc: test RegisterCallback
 * @tc.type: FUNC
 */
HWTEST_F(DeviceStatusAlgorithmTest, DeviceStatusAlgorithmTest032, TestSize.Level1)
{
    FI_HILOGI("DeviceStatusAlgorithmTest032 start.");
    int32_t ret = g_manager->Enable(Type::TYPE_VERTICAL_POSITION);
    EXPECT_EQ(ret, RET_OK);

    std::shared_ptr<DeviceStatusMock> callback = std::make_shared<DeviceStatusMock>();
    ret = g_manager->RegisterCallback(callback);
    EXPECT_EQ(ret, RET_OK);

    ret = g_manager->Disable(Type::TYPE_VERTICAL_POSITION);
    EXPECT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
