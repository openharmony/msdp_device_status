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

#ifndef COOPERATE_MESSAGE_H
#define COOPERATE_MESSAGE_H

namespace OHOS {
namespace Msdp {
enum class CooperateMessage {
    PREPARE = 0,
    UNPREPARE = 1,
    ACTIVATE = 2,
    ACTIVATE_SUCCESS = 3,
    ACTIVATE_FAIL = 4,
    DEACTIVATE_SUCCESS = 5,
    DEACTIVATE_FAIL = 6,
    SESSION_CLOSED = 7,
    PARAMETER_ERROR = 401,
    COOPERATE_FAIL = 20900001
};
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_MESSAGE_H