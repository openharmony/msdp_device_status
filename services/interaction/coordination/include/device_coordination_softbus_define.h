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

#ifndef DEVICE_COORDINATION_SOFTBUS_DEFINE_H
#define DEVICE_COORDINATION_SOFTBUS_DEFINE_H

#include <string>
#include <unistd.h>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
constexpr int32_t FILTER_WAIT_TIMEOUT_SECOND { 1 };
constexpr int32_t SESSION_SIDE_CLIENT { 1 };
constexpr int32_t SESSION_SIDE_SERVER { 0 };
constexpr int32_t SESSION_WAIT_TIMEOUT_SECOND { 5 };
constexpr uint32_t DEVICE_NAME_SIZE_MAX { 256 };
constexpr uint32_t PKG_NAME_SIZE_MAX { 65 };
constexpr uint32_t BIND_STRING_LENGTH { 10 };
constexpr uint32_t INTERCEPT_STRING_LENGTH { 20 };
constexpr uint32_t SESSION_NAME_SIZE_MAX { 256 };
constexpr size_t MSG_MAX_SIZE { 45 * 1024 };
constexpr int32_t ENCRYPT_TAG_LEN { 32 };

#define FI_SOFTBUS_KEY_CMD_TYPE "fi_softbus_key_cmd_type"
#define FI_SOFTBUS_KEY_LOCAL_DEVICE_ID "fi_softbus_key_local_device_id"
#define FI_SOFTBUS_KEY_START_DHID "fi_softbus_key_start_dhid"
#define FI_SOFTBUS_KEY_POINTER_X "fi_softbus_key_pointer_x"
#define FI_SOFTBUS_KEY_POINTER_Y "fi_softbus_key_pointer_y"
#define FI_SOFTBUS_KEY_RESULT "fi_softbus_key_result"
#define FI_SOFTBUS_KEY_SESSION_ID "fi_softbus_key_session_id"
#define FI_SOFTBUS_KEY_OTHER_DEVICE_ID "fi_softbus_key_other_device_id"
#define FI_SOFTBUS_POINTER_BUTTON_IS_PRESS "fi_softbus_pointer_button_is_press"

enum {
    REMOTE_COORDINATION_START = 1,
    REMOTE_COORDINATION_START_RES = 2,
    REMOTE_COORDINATION_STOP = 3,
    REMOTE_COORDINATION_STOP_RES = 4,
    REMOTE_COORDINATION_STOP_OTHER_RES = 5,
    NOTIFY_UNCHAINED_RES = 6,
    NOTIFY_FILTER_ADDED = 7
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_COORDINATION_SOFTBUS_DEFINE_H
