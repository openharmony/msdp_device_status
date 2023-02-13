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

#ifndef DEVICESTATUS_DATA_DEFINE_H
#define DEVICESTATUS_DATA_DEFINE_H

#include <functional>

#include "cJSON.h"
#include "sensor_agent.h"

#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
constexpr double PI = 3.141592653589793;
constexpr double RESULTANT_ACC_LOW_THRHD = 9;
constexpr double RESULTANT_ACC_UP_THRHD = 10.5;
constexpr double RELATIVE_ACC_MIN_THRHD = 7.5;
constexpr double RELATIVE_ACC_MAX_THRHD = 13;
constexpr double ACC_VALID_THRHD = 160.0;
constexpr double ANGLE_180_DEGREE = 180.0;
constexpr double ANGLE_HOR_UP_THRHD = 180.1;
constexpr double ANGLE_HOR_LOW_THRHD = 160.0;
constexpr double ANGLE_VER_UP_THRHD = 110.0;
constexpr double ANGLE_VER_LOW_THRHD = 80.0;

constexpr int32_t VALID_TIME_THRESHOLD = 500;
constexpr int32_t ACC_SAMPLE_PERIOD = 100;
constexpr int32_t COUNTER_THRESHOLD = VALID_TIME_THRESHOLD / ACC_SAMPLE_PERIOD;

using SensorCallback = std::function<void(int32_t, AccelData*)>;

struct JsonParser {
    JsonParser() = default;
    ~JsonParser()
    {
        if (json_ != nullptr) {
            cJSON_Delete(json_);
            json_ = nullptr;
        }
    }
    operator cJSON *()
    {
        return json_;
    }
    cJSON *json_ = nullptr;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DATA_DEFINE_H
