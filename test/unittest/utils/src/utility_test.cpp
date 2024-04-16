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

#include <future>
#include <optional>
#include <unistd.h>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "UtilityTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
} // namespace

class UtilityTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase(void);
};

void UtilityTest::SetUpTestCase() {}

void UtilityTest::TearDownTestCase() {}

void UtilityTest::SetUp() {}

void UtilityTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

/**
 * @tc.name: UtityTest_IsInteger_001
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "0";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_002
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "123";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_003
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = " 0";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_004
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "-0";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_005
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "-1";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_006
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "-10";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_007
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_007, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "123";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_008
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_008, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "-123";
    bool ret = Utility::IsInteger(target);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_009
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_009, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "0123";
    bool ret = Utility::IsInteger(target);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_010
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_010, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "A01";
    bool ret = Utility::IsInteger(target);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_011
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest,  UtityTest_IsInteger_011, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = "A-10";
    bool ret = Utility::IsInteger(target);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_012
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_012, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = " 123A";
    bool ret = Utility::IsInteger(target);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: UtityTest_IsInteger_013
 * @tc.desc: Checks whether a string is an integer.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UtilityTest, UtityTest_IsInteger_013, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string target = " 123 A";
    bool ret = Utility::IsInteger(target);
    ASSERT_FALSE(ret);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
