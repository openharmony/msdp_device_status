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

#ifndef JS_UTIL_H
#define JS_UTIL_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "napi/native_api.h"

#include "coordination_message.h"

#include "refbase.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
const std::unordered_map<int32_t, std::string> ERR_CODE_MSG_MAP {
    { static_cast<int32_t> (CoordinationMessage::PREPARE), "PREPARE" },
    { static_cast<int32_t> (CoordinationMessage::UNPREPARE), "UNPREPARE" },
    { static_cast<int32_t> (CoordinationMessage::ACTIVATE), "ACTIVATE" },
    { static_cast<int32_t> (CoordinationMessage::ACTIVATE_SUCCESS), "ACTIVATE_SUCCESS" },
    { static_cast<int32_t> (CoordinationMessage::ACTIVATE_FAIL), "ACTIVATE_FAIL" },
    { static_cast<int32_t> (CoordinationMessage::DEACTIVATE_SUCCESS), "DEACTIVATE_SUCCESS" },
    { static_cast<int32_t> (CoordinationMessage::DEACTIVATE_FAIL), "DEACTIVATE_FAIL" },
    { static_cast<int32_t> (CoordinationMessage::SESSION_CLOSED), "SESSION_CLOSED" }
};
} // namespace

class JsUtil {
public:
    struct UserData {
        int32_t userData { 0 };
        int32_t deviceId { 0 };
        napi_value handle { nullptr };
        std::vector<int32_t> keys;
    };
    struct CallbackData {
        bool prepareResult { false };
        bool activateResult { false };
        bool deactivateResult { false };
        bool coordinationOpened { false };
        std::string deviceDescriptor;
        int32_t errCode { 0 };
        CoordinationMessage msg = CoordinationMessage::PREPARE;
        std::string type;
    };
    struct CallbackInfo : RefBase {
        CallbackInfo() = default;
        ~CallbackInfo() = default;
        napi_env env { nullptr };
        napi_ref ref { nullptr };
        napi_deferred deferred { nullptr };
        int32_t errCode { 0 };
        CallbackData data;
    };
    struct MouseCallbackData {
        std::string networkId;
        int32_t displayX { -1 };
        int32_t displayY { -1 };
        int32_t displayWidth { -1 };
        int32_t displayHeight { -1 };
    };
    struct MouseCallbackInfo : RefBase {
        MouseCallbackInfo() = default;
        ~MouseCallbackInfo() = default;
        napi_env env { nullptr };
        napi_ref ref { nullptr };
        napi_deferred deferred { nullptr };
        int32_t errCode { 0 };
        MouseCallbackData data;
    };
    template <typename T>
    static void DeletePtr(T &ptr)
    {
        if (ptr != nullptr) {
            delete ptr;
            ptr = nullptr;
        }
    }

    static napi_value GetPrepareInfo(sptr<CallbackInfo> cb);
    static napi_value GetActivateInfo(sptr<CallbackInfo> cb);
    static napi_value GetDeactivateInfo(sptr<CallbackInfo> cb);
    static napi_value GetCrossingSwitchStateInfo(sptr<CallbackInfo> cb);
    static napi_value GetCrossingSwitchStateResult(napi_env env, bool result);
    static napi_value GetResult(napi_env env, bool result, int32_t errCode);
    static bool GetErrMsg(int32_t errCode, std::string &msg);
    static bool IsSameHandle(napi_env env, napi_value handle, napi_ref ref);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // JS_UTIL_H
