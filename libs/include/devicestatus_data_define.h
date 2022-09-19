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

#ifndef DEVICESTATUS_DATA_DEFINE_H
#define DEVICESTATUS_DATA_DEFINE_H

#include <functional>
#include "devicestatus_data_utils.h"
#include "sensor_agent.h"

#include "cJSON.h"

namespace OHOS {
namespace Msdp {
const std::string MSDP_DATA_PATH = "/data/msdp/device_status_data.json";
const std::string MSDP_DATA_DIR = "/data/msdp";

constexpr double PI = 3.141592653589793;

// universal
constexpr int32_t VECTEOR_MODULE_LOW_THRESHOLD = 5;
constexpr int32_t VECTEOR_MODULE_HIGH_THRESHOLD = 12;
constexpr int32_t ACCELERATION_VALID_THRESHOLD = 100;
constexpr int32_t ANGLE_ONE_HUNDRED_AND_EIGHTY_DEGREE = 180;
constexpr int32_t ANGLE_TWENTY_DEGREE = 20;
constexpr int32_t ANGLE_EIGHTY_DEGREE = 80;
constexpr int32_t ANGLE_ONE_HUNDRED_AND_TEN_DEGREE = 110;

// absoluteStill, horizontal, vertical
constexpr int32_t VALID_TIME_THRESHOLD = 500;
constexpr int32_t ACC_SAMPLE_PERIOD = 100;
constexpr int32_t COUNTER_THRESHOLD = VALID_TIME_THRESHOLD / ACC_SAMPLE_PERIOD;

struct JsonParser {
    JsonParser() = default;
    ~JsonParser()
    {
        if (json_ != nullptr) {
            cJSON_Delete(json_);
        }
    }
    operator cJSON *()
    {
        return json_;
    }
    cJSON *json_ = nullptr;
};

using SensorCallback = std::function<void(int32_t, void*)>;
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_DATA_DEFINE_H
