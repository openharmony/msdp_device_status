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
#include "util.h"
#include "devicestatus_common.h"
#include "devicestatus_msdp_rdb.h"

namespace OHOS {
namespace Msdp {
int32_t DeviceStatusDataParse::CreateJsonFile()
{
    int32_t fd = open(MSDP_DATA_PATH.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        DEV_HILOGE(SERVICE, "open failed.");
        return false;
    }
    close(fd);
    fd = -1;

    struct stat buf;
    if (stat(MSDP_DATA_DIR.c_str(), &buf) != 0) {
        DEV_HILOGE(SERVICE, "stat folder path is invalid %{public}d.", errno);
        return false;
    }
    if (chown(MSDP_DATA_PATH.c_str(), buf.st_uid, buf.st_gid) != 0) {
        DEV_HILOGE(SERVICE, "chown failed, errno is %{public}d.", errno);
        return false;
    }

    return true;
}

bool DeviceStatusDataParse::ParseDeviceStatusData(DevicestatusDataUtils::DevicestatusData& data, int type)
{
    std::string jsonBuf = MMI::ReadJsonFile(MSDP_DATA_PATH.c_str());
    if (jsonBuf.empty()) {
        DEV_HILOGE(SERVICE, "read json failed, errno is %{public}d.", errno);
        return false;
    }
    return DeviceStatusDataInit(jsonBuf, true, type, data);
}

bool DeviceStatusDataParse::DeviceStatusDataInit(const std::string& fileData, bool logStatus,
    int type, DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGE(SERVICE, "enter");
    JsonParser parser;
    parser.json_ = cJSON_Parse(fileData.c_str());
    if (cJSON_IsArray(parser.json_)) {
        DEV_HILOGE(SERVICE, "parser is array");
        return false;
    }

    if (type >= DevicestatusDataUtils::DevicestatusType::TYPE_STILL && type <= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN)
    {
        cJSON* in_vector = cJSON_GetObjectItem(parser.json_, DeviceStatusJson[type].Json);
        int32_t in_vector_size = cJSON_GetArraySize(in_vector);
        in_vector_count[type] = in_vector_count[type]%in_vector_size;
        cJSON * cJsonValue = cJSON_GetArrayItem(in_vector, in_vector_count[type]);
        in_vector_count[type]++;
        if (cJSON_IsNumber(cJsonValue)) {
        data.type = static_cast<DevicestatusDataUtils::DevicestatusType>(type);
        data.value = static_cast<DevicestatusDataUtils::DevicestatusValue>(cJsonValue->valueint);
        DEV_HILOGE(SERVICE, "type: %{public}d. status: %{public}d", data.type, data.value);
        return true;
        }
    }
    return false;
}

} // namespace MSDP
} // namespace OHOS
