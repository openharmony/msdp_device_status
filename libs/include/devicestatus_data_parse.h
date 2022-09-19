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
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

#include "devicestatus_data_define.h"
#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class DeviceStatusDataParse {
public:
    DeviceStatusDataParse() = default;
    ~DeviceStatusDataParse() = default;
    bool ParseDeviceStatusData(DevicestatusDataUtils::DevicestatusData& data, int type);
    bool DeviceStatusDataInit(const std::string& fileData, bool logStatus, int type, DevicestatusDataUtils::DevicestatusData& data);
    int32_t CreateJsonFile();
};
}
}
#endif // DEVICESTATUS_DATA_PARSE_H
