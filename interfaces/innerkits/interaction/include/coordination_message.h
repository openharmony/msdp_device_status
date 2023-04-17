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
    PREPARE = 0,  //OPEN_SUCCESS
    UNPREPARE = 1,   //OPEN_FAIL
    ACTIVATE = 2,   // INFO_START
    ACTIVATE_SUCCESS = 3, // INFO_SUCCESS
    ACTIVATE_FAIL = 4,   //  INFO_FAIL
    DEACTIVATE_SUCCESS = 5,  // STOP_SUCCESS
    DEACTIVATE_FAIL  = 6,  // STOP_FAIL
    PARAMETER_ERROR = 401,
    COORDINATION_FAIL = 20900001,
};
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_MESSAGE_H