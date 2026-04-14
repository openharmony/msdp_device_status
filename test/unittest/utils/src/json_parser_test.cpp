/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cJSON.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "json_parser.h"

#undef LOG_TAG
#define LOG_TAG "JsonParserTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

class JsonParserTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase(void);
};

void JsonParserTest::SetUpTestCase() {}

void JsonParserTest::TearDownTestCase() {}

void JsonParserTest::SetUp() {}

void JsonParserTest::TearDown() {}

/**
 * @tc.name: JsonParserTest_Constructor_001
 * @tc.desc: Test JsonParser constructor with nullptr jsonStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_Constructor_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    ASSERT_EQ(parser.Get(), nullptr);
}

/**
 * @tc.name: JsonParserTest_Constructor_002
 * @tc.desc: Test JsonParser constructor with valid jsonStr
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_Constructor_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": \"value\"}";
    JsonParser parser(jsonStr);
    ASSERT_NE(parser.Get(), nullptr);
}

/**
 * @tc.name: JsonParserTest_IsInteger_001
 * @tc.desc: Test IsInteger with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_IsInteger_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    ASSERT_FALSE(parser.IsInteger(nullptr));
}

/**
 * @tc.name: JsonParserTest_ParseInt32_001
 * @tc.desc: Test ParseInt32 with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(nullptr, "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseInt32_002
 * @tc.desc: Test ParseInt32 with non-object json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "[1, 2, 3]";
    JsonParser parser(jsonStr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseInt32_003
 * @tc.desc: Test ParseInt32 with non-number jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": \"value\"}";
    JsonParser parser(jsonStr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseInt32_004
 * @tc.desc: Test ParseInt32 with non-integer jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123.45}";
    JsonParser parser(jsonStr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseInt32_005
 * @tc.desc: Test ParseInt32 with out-of-bounds integer
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 2147483648}";
    JsonParser parser(jsonStr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseInt32_006
 * @tc.desc: Test ParseInt32 with valid integer
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseInt32_006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123}";
    JsonParser parser(jsonStr);
    int32_t value = 0;
    ASSERT_EQ(parser.ParseInt32(parser.Get(), "key", value), RET_OK);
    ASSERT_EQ(value, 123);
}

/**
 * @tc.name: JsonParserTest_ParseString_001
 * @tc.desc: Test ParseString with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseString_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    std::string value;
    ASSERT_EQ(parser.ParseString(nullptr, "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseString_002
 * @tc.desc: Test ParseString with non-object json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseString_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "[1, 2, 3]";
    JsonParser parser(jsonStr);
    std::string value;
    ASSERT_EQ(parser.ParseString(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseString_003
 * @tc.desc: Test ParseString with non-string jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseString_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123}";
    JsonParser parser(jsonStr);
    std::string value;
    ASSERT_EQ(parser.ParseString(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseString_004
 * @tc.desc: Test ParseString with valid string
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseString_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": \"value\"}";
    JsonParser parser(jsonStr);
    std::string value;
    ASSERT_EQ(parser.ParseString(parser.Get(), "key", value), RET_OK);
    ASSERT_EQ(value, "value");
}

/**
 * @tc.name: JsonParserTest_ParseBool_001
 * @tc.desc: Test ParseBool with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseBool_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    bool value = false;
    ASSERT_EQ(parser.ParseBool(nullptr, "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseBool_002
 * @tc.desc: Test ParseBool with non-object json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseBool_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "[1, 2, 3]";
    JsonParser parser(jsonStr);
    bool value = false;
    ASSERT_EQ(parser.ParseBool(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseBool_003
 * @tc.desc: Test ParseBool with non-bool jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseBool_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123}";
    JsonParser parser(jsonStr);
    bool value = false;
    ASSERT_EQ(parser.ParseBool(parser.Get(), "key", value), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseBool_004
 * @tc.desc: Test ParseBool with valid bool
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseBool_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": true}";
    JsonParser parser(jsonStr);
    bool value = false;
    ASSERT_EQ(parser.ParseBool(parser.Get(), "key", value), RET_OK);
    ASSERT_TRUE(value);
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_001
 * @tc.desc: Test ParseStringArray with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(nullptr, "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_002
 * @tc.desc: Test ParseStringArray with non-object json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "[1, 2, 3]";
    JsonParser parser(jsonStr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_003
 * @tc.desc: Test ParseStringArray with non-array jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123}";
    JsonParser parser(jsonStr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_004
 * @tc.desc: Test ParseStringArray with non-string arrayItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1, 2, 3]}";
    JsonParser parser(jsonStr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_005
 * @tc.desc: Test ParseStringArray with valid array
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [\"a\", \"b\", \"c\"]}";
    JsonParser parser(jsonStr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
    ASSERT_EQ(value[0], "a");
    ASSERT_EQ(value[1], "b");
    ASSERT_EQ(value[2], "c");
}

/**
 * @tc.name: JsonParserTest_ParseStringArray_006
 * @tc.desc: Test ParseStringArray with array size larger than maxSize
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseStringArray_006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [\"a\", \"b\", \"c\", \"d\", \"e\"]}";
    JsonParser parser(jsonStr);
    std::vector<std::string> value;
    ASSERT_EQ(parser.ParseStringArray(parser.Get(), "key", value, 3), RET_OK);
    ASSERT_EQ(value.size(), 3);
    ASSERT_EQ(value[0], "a");
    ASSERT_EQ(value[1], "b");
    ASSERT_EQ(value[2], "c");
}
}  // namespace DeviceStatus
}  // namespace Msdp
}  // namespace OHOS

/**
 * @tc.name: JsonParserTest_ParseFloatArray_001
 * @tc.desc: Test ParseFloatArray with nullptr json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    JsonParser parser(nullptr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(nullptr, "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_002
 * @tc.desc: Test ParseFloatArray with non-object json
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "[1.0, 2.0, 3.0]";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_003
 * @tc.desc: Test ParseFloatArray with non-existent key
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"otherKey\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_004
 * @tc.desc: Test ParseFloatArray with non-array jsonNode
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": 123.45}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_005
 * @tc.desc: Test ParseFloatArray with non-number arrayItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [\"a\", \"b\", \"c\"]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_006
 * @tc.desc: Test ParseFloatArray with empty array
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": []}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 0);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_007
 * @tc.desc: Test ParseFloatArray with valid float array
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_007, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.5, 3.14]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
    EXPECT_FLOAT_EQ(value[0], 1.0f);
    EXPECT_FLOAT_EQ(value[1], 2.5f);
    EXPECT_FLOAT_EQ(value[2], 3.14f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_008
 * @tc.desc: Test ParseFloatArray with negative values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_008, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [-1.5, -2.0, -3.14]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
    EXPECT_FLOAT_EQ(value[0], -1.5f);
    EXPECT_FLOAT_EQ(value[1], -2.0f);
    EXPECT_FLOAT_EQ(value[2], -3.14f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_009
 * @tc.desc: Test ParseFloatArray with zero value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_009, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [0.0, 0.0, 0.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
    EXPECT_FLOAT_EQ(value[0], 0.0f);
    EXPECT_FLOAT_EQ(value[1], 0.0f);
    EXPECT_FLOAT_EQ(value[2], 0.0f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_010
 * @tc.desc: Test ParseFloatArray with array size equal to maxSize
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_010, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 3), RET_OK);
    ASSERT_EQ(value.size(), 3);
    EXPECT_FLOAT_EQ(value[0], 1.0f);
    EXPECT_FLOAT_EQ(value[1], 2.0f);
    EXPECT_FLOAT_EQ(value[2], 3.0f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_011
 * @tc.desc: Test ParseFloatArray with array size larger than maxSize
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_011, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0, 4.0, 5.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 3), RET_OK);
    ASSERT_EQ(value.size(), 3);
    EXPECT_FLOAT_EQ(value[0], 1.0f);
    EXPECT_FLOAT_EQ(value[1], 2.0f);
    EXPECT_FLOAT_EQ(value[2], 3.0f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_012
 * @tc.desc: Test ParseFloatArray with mixed positive and negative values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_012, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.5, -2.5, 0.0, 3.14]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 4);
    EXPECT_FLOAT_EQ(value[0], 1.5f);
    EXPECT_FLOAT_EQ(value[1], -2.5f);
    EXPECT_FLOAT_EQ(value[2], 0.0f);
    EXPECT_FLOAT_EQ(value[3], 3.14f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_013
 * @tc.desc: Test ParseFloatArray with single element
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_013, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [3.14159]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 1);
    EXPECT_FLOAT_EQ(value[0], 3.14159f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_014
 * @tc.desc: Test ParseFloatArray with very large array
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_014, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 10);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_015
 * @tc.desc: Test ParseFloatArray with decimal precision
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_015, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [0.1, 0.01, 0.001, 0.0001]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 4);
    EXPECT_FLOAT_EQ(value[0], 0.1f);
    EXPECT_FLOAT_EQ(value[1], 0.01f);
    EXPECT_FLOAT_EQ(value[2], 0.001f);
    EXPECT_FLOAT_EQ(value[3], 0.0001f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_016
 * @tc.desc: Test ParseFloatArray with scientific notation
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_016, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0e5, 2.0e-3, 3.14]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_017
 * @tc.desc: Test ParseFloatArray with mixed integer and float
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_017, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1, 2.5, 3, 4.75, 5]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 5);
    EXPECT_FLOAT_EQ(value[0], 1.0f);
    EXPECT_FLOAT_EQ(value[1], 2.5f);
    EXPECT_FLOAT_EQ(value[2], 3.0f);
    EXPECT_FLOAT_EQ(value[3], 4.75f);
    EXPECT_FLOAT_EQ(value[4], 5.0f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_018
 * @tc.desc: Test ParseFloatArray with maxSize zero
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_018, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 0), RET_OK);
    ASSERT_EQ(value.size(), 0);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_019
 * @tc.desc: Test ParseFloatArray clears existing value
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_019, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.5, 2.5]}";
    JsonParser parser(jsonStr);
    std::vector<float> value = {99.0f, 99.0f, 99.0f};
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 2);
    EXPECT_FLOAT_EQ(value[0], 1.5f);
    EXPECT_FLOAT_EQ(value[1], 2.5f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_020
 * @tc.desc: Test ParseFloatArray with very small float values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_020, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [0.00001, 0.00002, 0.00003]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_021
 * @tc.desc: Test ParseFloatArray with very large float values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_021, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1000000.0, 2000000.0, 3000000.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_022
 * @tc.desc: Test ParseFloatArray with boolean arrayItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_022, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [true, false, true]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_023
 * @tc.desc: Test ParseFloatArray with null arrayItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_023, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, null, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_024
 * @tc.desc: Test ParseFloatArray with object arrayItem
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_024, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [{\"a\":1}, {\"b\":2}, {\"c\":3}]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_025
 * @tc.desc: Test ParseFloatArray with infinity and special values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_025, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_026
 * @tc.desc: Test ParseFloatArray with maxSize negative
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_026, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, -1), RET_OK);
    ASSERT_EQ(value.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_027
 * @tc.desc: Test ParseFloatArray with complex key names
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_027, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"dropAnimationCurve\": [0.2, 0.0, 0.2, 1.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "dropAnimationCurve", value, 10), RET_OK);
    ASSERT_EQ(value.size(), 4);
    EXPECT_FLOAT_EQ(value[0], 0.2f);
    EXPECT_FLOAT_EQ(value[1], 0.0f);
    EXPECT_FLOAT_EQ(value[2], 0.2f);
    EXPECT_FLOAT_EQ(value[3], 1.0f);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_028
 * @tc.desc: Test ParseFloatArray repeated calls
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_028, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [1.0, 2.0, 3.0]}";
    JsonParser parser(jsonStr);
    std::vector<float> value1;
    std::vector<float> value2;
    std::vector<float> value3;
    
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value1, 10), RET_OK);
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value2, 10), RET_OK);
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value3, 10), RET_OK);
    
    ASSERT_EQ(value1.size(), 3);
    ASSERT_EQ(value2.size(), 3);
    ASSERT_EQ(value3.size(), 3);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_029
 * @tc.desc: Test ParseFloatArray with float string values
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_029, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [\"1.0\", \"2.0\", \"3.0\"]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}

/**
 * @tc.name: JsonParserTest_ParseFloatArray_030
 * @tc.desc: Test ParseFloatArray with nested arrays
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsonParserTest, JsonParserTest_ParseFloatArray_030, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const char *jsonStr = "{\"key\": [[1.0, 2.0], [3.0, 4.0]]}";
    JsonParser parser(jsonStr);
    std::vector<float> value;
    ASSERT_EQ(parser.ParseFloatArray(parser.Get(), "key", value, 10), RET_ERR);
}
