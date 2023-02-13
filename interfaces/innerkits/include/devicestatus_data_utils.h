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

#ifndef DEVICESTATUS_DATA_UTILS_H
#define DEVICESTATUS_DATA_UTILS_H

#include<string>

namespace OHOS {
namespace Msdp {
class DevicestatusDataUtils {
public:
    enum DevicestatusType {
        TYPE_INVALID = -1,
        TYPE_STILL,
        TYPE_RELATIVE_STILL,
        TYPE_CAR_BLUETOOTH,
		TYPE_STAND,
        TYPE_LID_OPEN,
        TYPE_MAX
    };

    enum DevicestatusValue {
        VALUE_INVALID = -1,
        VALUE_ENTER = 1,
        VALUE_EXIT
    };

    enum Value {
        INVALID = 0,
        VALID
    };

    struct DevicestatusData {
        DevicestatusType type = TYPE_INVALID;
        DevicestatusValue value = VALUE_INVALID;
    };
    struct AlgoData {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        double resultantAcc = 0.0;
        double pitch = 0.0;
        double roll = 0.0;
    };
};


typedef struct DeviceStatusJsonData {
    int32_t type;
    std::string json;
}DeviceStatusJsonD;

static DeviceStatusJsonD DeviceStatusJson[] = {
    {DevicestatusDataUtils::DevicestatusType::TYPE_STILL, "TYPE_STILL"},
    {DevicestatusDataUtils::DevicestatusType::TYPE_RELATIVE_STILL, "TYPE_RELATIVE_STILL"},
    {DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH, "TYPE_CAR_BLUETOOTH"},
    {DevicestatusDataUtils::DevicestatusType::TYPE_STAND, "TYPE_STAND"},
    {DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN, "TYPE_LID_OPEN"}
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DATA_UTILS_H
