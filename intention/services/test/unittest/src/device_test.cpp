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

#include <gtest/gtest.h>

#include "device.h"
#include "device_manager.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "DeviceTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

DeviceManager devmg_;
const std::string devNode_ = { "event0" };
const std::string devPath_ = { "/dev/input/event0" };

class DeviceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void DeviceTest::SetUpTestCase() {}
void DeviceTest::TearDownTestCase() {}
void DeviceTest::SetUp() {}
void DeviceTest::TearDown() {}

/**
 * @tc.name: OpenTest001
 * @tc.desc: Test func named open device
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, OpenTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    std::shared_ptr<IDevice> dev = devmg_.FindDevice(devPath_);
    dev = std::make_shared<Device>(deviceId);
    CHKPV(dev);
    dev->SetDevPath(devPath_);
    int32_t ret = dev->Open();
    EXPECT_EQ(ret, RET_OK);
    dev->Close();
}

/**
 * @tc.name: OpenTest002
 * @tc.desc: Test func named open device
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, OpenTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    std::shared_ptr<IDevice> dev = devmg_.FindDevice(devPath_);
    dev = std::make_shared<Device>(deviceId);
    CHKPV(dev);
    int32_t ret = dev->Open();
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: CloseTest001
 * @tc.desc: Test func named close device
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, CloseTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    std::shared_ptr<IDevice> dev = devmg_.FindDevice(devPath_);
    dev = std::make_shared<Device>(deviceId);
    CHKPV(dev);
    dev->Close();
}

/**
 * @tc.name: QueryDeviceInfoTest001
 * @tc.desc: Test func named QueryDeviceInfo
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, QueryDeviceInfoTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    dev->QueryDeviceInfo();
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: CheckAbsTest001
 * @tc.desc: Test func named CheckAbs
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, CheckAbsTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    dev->CheckAbs();
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: CheckMtTest001
 * @tc.desc: Test func named CheckMt
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, CheckMtTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    dev->CheckMt();
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ReadConfigFileTest001
 * @tc.desc: Test func named ReadConfigFile
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ReadConfigFileTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    const std::string filePath = { "/system/etc/device_status/drag_icon/Copy_Drag.svg" };
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ReadConfigFile(filePath);
    EXPECT_EQ(ret, RET_ERR);
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ReadConfigFileTest002
 * @tc.desc: Test func named ReadConfigFile
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ReadConfigFileTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    const std::string filePath = "";
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ReadConfigFile(filePath);
    EXPECT_EQ(ret, RET_ERR);
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ConfigItemSwitchTest001
 * @tc.desc: Test func named ConfigItemSwitch
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ConfigItemSwitchTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string configItem = "123456";
    std::string value = "123456";
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ConfigItemSwitch(configItem, value);
    EXPECT_EQ(ret, RET_OK);
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ConfigItemSwitchTest002
 * @tc.desc: Test func named ConfigItemSwitch
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ConfigItemSwitchTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string configItem = "";
    std::string value = "123456";
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ConfigItemSwitch(configItem, value);
    EXPECT_EQ(ret, RET_ERR);
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ConfigItemSwitchTest003
 * @tc.desc: Test func named ConfigItemSwitch
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ConfigItemSwitchTest003, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::string configItem = "1234567";
    std::string value = "";
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ConfigItemSwitch(configItem, value);
    EXPECT_EQ(ret, RET_ERR);
    delete dev;
    dev = nullptr;
}

/**
 * @tc.name: ReadTomlFileTest001
 * @tc.desc: Test func named ReadTomlFile
 * @tc.type: FUNC
 */
HWTEST_F(DeviceTest, ReadTomlFileTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    const std::string filePath = { "/system/etc/device_status/drag_icon/Copy_Drag.svg" };
    int32_t deviceId = devmg_.ParseDeviceId(devNode_);
    Device *dev = new Device(deviceId);
    CHKPV(dev);
    int32_t ret = dev->ReadTomlFile(filePath);
    EXPECT_EQ(ret, RET_ERR);
    delete dev;
    dev = nullptr;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS