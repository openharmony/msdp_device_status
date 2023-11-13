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

#ifndef UTIL_NAPI_ERROR_H
#define UTIL_NAPI_ERROR_H

#include <map>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "securec.h"

#include "util_napi.h"

namespace OHOS {
namespace Msdp {
const std::string ERR_CODE { "code" };
struct NapiError {
    int32_t errorCode { 0 };
    std::string msg;
};

enum NapiErrorCode : int32_t {
    OTHER_ERROR = -1,
    COMMON_PERMISSION_CHECK_ERROR = 201,
    COMMON_PARAMETER_ERROR = 401,
    COMMON_NOT_ALLOWED_DISTRIBUTED = 40101,
    COOPERATOR_FAIL = 20900001
};

const std::map<int32_t, NapiError> NAPI_ERRORS = {
    {COMMON_PERMISSION_CHECK_ERROR,
        {COMMON_PERMISSION_CHECK_ERROR, "Permission denied. An attempt was made to %s forbidden by permission:%s."}},
    {COMMON_PARAMETER_ERROR, {COMMON_PARAMETER_ERROR, "Parameter error. The type of %s must be %s."}},
    {COMMON_NOT_ALLOWED_DISTRIBUTED,
        {COMMON_NOT_ALLOWED_DISTRIBUTED, "Cross-device dragging is not allowed"}},
    {COOPERATOR_FAIL, {COOPERATOR_FAIL, "Input device operation failed."}}
};

#define THROWERR_CUSTOM(env, code, msg) \
    do { \
        napi_value businessError = nullptr; \
        napi_value errorMsg = nullptr; \
        napi_value errorCode = nullptr; \
        napi_create_int32(env, code, &errorCode); \
        napi_create_string_utf8(env, std::string(msg).c_str(), NAPI_AUTO_LENGTH, &errorMsg); \
        napi_create_error(env, nullptr, errorMsg, &businessError); \
        napi_set_named_property(env, businessError, ERR_CODE.c_str(), errorCode); \
        napi_throw(env, businessError); \
        FI_HILOGE("Raising exceptions, errorCode:%{public}s, errorMsg:%{public}s", \
            (#code), std::string(msg).c_str()); \
    } while (0)

#define THROWERR(env, code, param1, param2) \
    do { \
        NapiError codeMsg; \
        if (UtilNapiError::GetApiError(code, codeMsg)) { \
            char buf[300]; \
            int32_t ret = sprintf_s(buf, sizeof(buf), codeMsg.msg.c_str(), param1, param2); \
            if (ret > 0) { \
                THROWERR_CUSTOM(env, code, buf); \
            } else { \
                FI_HILOGE("Failed to convert string type to char type, error code:%{public}s", (#code)); \
            } \
        } \
    } while (0)
namespace UtilNapiError {
bool GetApiError(int32_t code, NapiError &codeMsg);
} // namespace UtilNapiError
} // namespace Msdp
} // namespace OHOS
#endif // UTIL_NAPI_ERROR_H
