/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef VIRTUAL_DEVICE_DEFINES_H
#define VIRTUAL_DEVICE_DEFINES_H

#include <cstdint>
#include <string>

#ifndef REL_WHEEL_HI_RES
#define REL_WHEEL_HI_RES 0x0b
#endif

#ifndef REL_HWHEEL_HI_RES
#define REL_HWHEEL_HI_RES 0x0c
#endif

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
constexpr double HALF_VALUE { 2.0 };
constexpr double DEFAULT_PRECISION { 0.1 };
constexpr int32_t SLEEP_TIME { 500 };
constexpr int32_t DOWN_VALUE { 1 };
constexpr int32_t UP_VALUE { 0 };
constexpr int32_t SYNC_VALUE { 0 };
constexpr int32_t OBFUSCATED { 30 };
constexpr int32_t MOVE_VALUE_X { 1 };
constexpr int32_t MOVE_VALUE_Y { -1 };

struct ResolutionInfo {
    int16_t axisCode { 0 };
    int32_t absResolution { 0 };
};

struct AbsInfo {
    int32_t code { 0 };
    int32_t minValue { 0 };
    int32_t maxValue { 0 };
    int32_t fuzz { 0 };
    int32_t flat { 0 };
};

struct Coordinate {
    int32_t x { 0 };
    int32_t y { 0 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_DEVICE_DEFINES_H