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

#ifndef DEVICESTATUS_DATA_PARSE_H
#define DEVICESTATUS_DATA_PARSE_H

#include <string>
#include <vector>

#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusDataParse {
public:
    DeviceStatusDataParse() = default;
    ~DeviceStatusDataParse() = default;
    bool ParseDeviceStatusData(Type type, Data &data);
    bool DisableCount(const Type type);
    bool DeviceStatusDataInit(const std::string &fileData, bool logStatus, Type &type, Data &data);
    int32_t CreateJsonFile();

private:
    bool CheckFileDir(const std::string &filePath, const std::string &dir);
    bool CheckFileSize(const std::string &filePath);
    bool CheckFileExtendName(const std::string &filePath, const std::string &checkExtension);
    std::string ReadFile(const std::string &filePath);
    std::string ReadJsonFile(const std::string &filePath);
    static std::vector<int32_t> tempcount_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DATA_PARSE_H
