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

#ifndef COORDINATION_MESSAGE_H
#define COORDINATION_MESSAGE_H

namespace OHOS {
namespace Msdp {
enum class CoordinationMessage {
    OPEN_SUCCESS = 0,
    OPEN_FAIL = 1,
    INFO_START = 2,
    INFO_SUCCESS = 3,
    INFO_FAIL = 4,
    CLOSE = 5,
    CLOSE_SUCCESS = 6,
    STOP = 7,
    STOP_SUCCESS = 8,
    STOP_FAIL = 9,
    STATE_ON = 10,
    STATE_OFF = 11,
    PARAMETER_ERROR = 401,
    COORDINATION_FAIL = 20900001,
};
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_MESSAGE_H