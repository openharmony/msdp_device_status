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

#ifndef DP_DEFINE_H
#define DP_DEFINE_H

#include <string>
#include <variant>
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class ValueType {
    INT_TYPE,
    BOOL_TYPE,
    STRING_TYPE,
};

const std::string SERVICE_ID { "deviceStatus" };
const std::string SERVICE_TYPE { "deviceStatus" };
const std::string JSON_NULL = "null";
using DP_VALUE = std::variant<int32_t, bool, std::string>;
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DP_DEFINE_H