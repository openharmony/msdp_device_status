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

#ifndef DEVICESTATUS_DATA_PARSE_H
#define DEVICESTATUS_DATA_PARSE_H

#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

#include "cJSON.h"

#include "devicestatus_data_define.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_errors.h"
#include "devicestatus_hilog_wrapper.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusDataParse {
public:
    DeviceStatusDataParse() = default;
    ~DeviceStatusDataParse() = default;
    bool ParseDeviceStatusData(DevicestatusDataUtils::DevicestatusData& data,
        DevicestatusDataUtils::DevicestatusType& type);
    bool DisableCount(const  DevicestatusDataUtils::DevicestatusType& type);
    bool DataInit(const std::string& fileData, bool logStatus, DevicestatusDataUtils::DevicestatusType& type,
        DevicestatusDataUtils::DevicestatusData& data);
    int32_t CreateJsonFile();

private:
    bool JudgeTypeData(const int32_t valueInt);
    int32_t GetFileSize(const std::string& filePath);
    bool CheckFileDir(const std::string& filePath, const std::string& dir);
    int32_t CheckFileSize(const std::string& filePath);
    bool CheckFileExtendName(const std::string& filePath, const std::string& checkExtension);
    bool IsFileExists(const std::string& fileName);
    std::string ReadFile(const std::string &filePath);
    std::string ReadJsonFile(const std::string &filePath);
    static std::vector<int32_t> tempcount_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DATA_PARSE_H
