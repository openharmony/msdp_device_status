/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "boomerang_server.h"

#include "accesstoken_kit.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "parameters.h"
#include "tokenid_kit.h"


#include "devicestatus_define.h"
#include "devicestatus_dumper.h"
#include "devicestatus_hisysevent.h"

#undef LOG_TAG
#define LOG_TAG "BoomerangServer"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
const std::vector<std::string> SUPPORT_DEVICE_TYPE = { "phone", "tablet" };
}

BoomerangServer::BoomerangServer()
{
    manager_.Init();
}

void BoomerangServer::DumpDeviceStatusSubscriber(int32_t fd) const
{
    DS_DUMPER->DumpDeviceStatusSubscriber(fd);
}

void BoomerangServer::DumpDeviceStatusChanges(int32_t fd) const
{
    DS_DUMPER->DumpDeviceStatusChanges(fd);
}

void BoomerangServer::DumpCurrentDeviceStatus(int32_t fd)
{
    std::vector<Data> datas;

    for (auto type = TYPE_ABSOLUTE_STILL; type <= TYPE_LID_OPEN; type = static_cast<Type>(type + 1)) {
        Data data = manager_.GetLatestDeviceStatusData(type);
        if (data.value != OnChangedValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    DS_DUMPER->DumpDeviceStatusCurrentStatus(fd, datas);
}

int32_t BoomerangServer::SubscribeCallback(const CallingContext &context, int32_t type, const std::string& bundleName,
    const sptr<IRemoteBoomerangCallback>& subCallback)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    CHKPR(subCallback, RET_ERR);
    BoomerangType boomerangType = static_cast<BoomerangType>(type);
    auto appInfo = std::make_shared<BoomerangAppInfo>();
    appInfo->uid = context.uid;
    appInfo->pid = context.pid;
    appInfo->tokenId = context.tokenId;
    manager_.GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = boomerangType;
    appInfo->boomerangCallback = subCallback;
    DS_DUMPER->SaveBoomerangAppInfo(appInfo);
    int32_t ret = manager_.Subscribe(type, bundleName, subCallback);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang Subscribe failed");
        DS_DUMPER->RemoveBoomerangAppInfo(appInfo);
    }
    return ret;
}

int32_t BoomerangServer::UnsubscribeCallback(const CallingContext &context, int32_t type, const std::string& bundleName,
    const sptr<IRemoteBoomerangCallback>& unsubCallback)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    CHKPR(unsubCallback, RET_ERR);
    BoomerangType boomerangType = static_cast<BoomerangType>(type);
    auto appInfo = std::make_shared<BoomerangAppInfo>();
    appInfo->uid = context.uid;
    appInfo->pid = context.pid;
    appInfo->tokenId = context.tokenId;
    appInfo->packageName = DS_DUMPER->GetPackageName(appInfo->tokenId);
    appInfo->type = boomerangType;
    appInfo->boomerangCallback = unsubCallback;
    DS_DUMPER->RemoveBoomerangAppInfo(appInfo);
    int32_t ret = manager_.Unsubscribe(type, bundleName, unsubCallback);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang Unsubscribe failed");
        DS_DUMPER->SaveBoomerangAppInfo(appInfo);
    }
    return ret;
}

int32_t BoomerangServer::NotifyMetadataBindingEvent(const CallingContext &context, const std::string& bundleName,
    const sptr<IRemoteBoomerangCallback>& notifyCallback)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    if (!IsSystemHAPCalling(context)) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    CHKPR(notifyCallback, RET_ERR);
    auto appInfo = std::make_shared<BoomerangAppInfo>();
    appInfo->uid = context.uid;
    appInfo->pid = context.pid;
    appInfo->tokenId = context.tokenId;
    appInfo->packageName = DS_DUMPER->GetPackageName(appInfo->tokenId);
    appInfo->boomerangCallback = notifyCallback;
    DS_DUMPER->SetNotifyMetadatAppInfo(appInfo);
    int32_t ret = manager_.NotifyMetadata(bundleName, notifyCallback);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang NotifyMetadataBindingEvent failed");
    }
    return ret;
}

int32_t BoomerangServer::BoomerangEncodeImage(const CallingContext &context,
    const std::shared_ptr<Media::PixelMap>& pixelMap, const std::string& metaData,
    const sptr<IRemoteBoomerangCallback>& encodeCallback)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    if (!IsSystemHAPCalling(context)) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    CHKPR(encodeCallback, RET_ERR);
    auto appInfo = std::make_shared<BoomerangAppInfo>();
    appInfo->uid = context.uid;
    appInfo->pid = context.pid;
    appInfo->tokenId = context.tokenId;
    appInfo->packageName = DS_DUMPER->GetPackageName(appInfo->tokenId);
    appInfo->boomerangCallback = encodeCallback;
    DS_DUMPER->SetNotifyMetadatAppInfo(appInfo);
    int32_t ret = manager_.BoomerangEncodeImage(pixelMap, metaData, encodeCallback);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang EncodeImage failed");
    }
    return ret;
}

int32_t BoomerangServer::BoomerangDecodeImage(const CallingContext &context,
    const std::shared_ptr<Media::PixelMap>& pixelMap, const sptr<IRemoteBoomerangCallback>& decodeCallback)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    if (!IsSystemHAPCalling(context)) {
        FI_HILOGE("The caller is not system hap");
        return COMMON_NOT_SYSTEM_APP;
    }
    CHKPR(decodeCallback, RET_ERR);
    auto appInfo = std::make_shared<BoomerangAppInfo>();
    appInfo->uid = context.uid;
    appInfo->pid = context.pid;
    appInfo->tokenId = context.tokenId;
    appInfo->packageName = DS_DUMPER->GetPackageName(appInfo->tokenId);
    appInfo->boomerangCallback = decodeCallback;
    DS_DUMPER->SetNotifyMetadatAppInfo(appInfo);
    int32_t ret = manager_.BoomerangDecodeImage(pixelMap, decodeCallback);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang DecodeImage failed");
    }
    return ret;
}

int32_t BoomerangServer::SubmitMetadata(const CallingContext &context, const std::string& metaData)
{
    if (!IsDeviceSupport()) {
        FI_HILOGE("the device type is not support");
        return COMMON_CAPABILITY_NOT_SUPPORT;
    }
    int32_t ret = manager_.SubmitMetadata(metaData);
    if (ret != RET_OK) {
        FI_HILOGE("boomerang submit metada failed");
    }
    return ret;
}

Data BoomerangServer::GetCache(const CallingContext &context, const Type &type)
{
    return manager_.GetLatestDeviceStatusData(type);
}

void BoomerangServer::ReportSensorSysEvent(const CallingContext &context, int32_t type, bool enable)
{
    std::string packageName;
    manager_.GetPackageName(context.tokenId, packageName);
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MSDP,
        std::string(enable ? "Subscribe" : "Unsubscribe"),
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", context.uid,
        "PKGNAME", packageName,
        "TYPE", type);
    if (ret != 0) {
        FI_HILOGE("HiviewDFX write failed, error:%{public}d", ret);
    }
}

bool BoomerangServer::IsSystemServiceCalling(const CallingContext &context)
{
    auto flag = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(context.tokenId);
    if ((flag == Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) ||
        (flag == Security::AccessToken::ATokenTypeEnum::TOKEN_SHELL)) {
        FI_HILOGI("system service calling, flag:%{public}u", flag);
        return true;
    }
    return false;
}

bool BoomerangServer::IsSystemHAPCalling(const CallingContext &context)
{
    if (IsSystemServiceCalling(context)) {
        return true;
    }
    return Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(context.fullTokenId);
}

bool BoomerangServer::IsDeviceSupport()
{
#ifdef DEVICE_STATUS_PHONE_STANDARD_LITE
    return false;
#endif
    std::string deviceType = OHOS::system::GetParameter("const.product.devicetype", "");
    return std::find(SUPPORT_DEVICE_TYPE.begin(), SUPPORT_DEVICE_TYPE.end(), deviceType) != SUPPORT_DEVICE_TYPE.end();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

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