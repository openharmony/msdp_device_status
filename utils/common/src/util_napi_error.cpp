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

#include "util_napi_error.h"

#undef LOG_TAG
#define LOG_TAG "UtilNapiError"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace UtilNapiError {

bool GetErrorMsg(int32_t code, std::string &codeMsg)
{
    auto iter = NAPI_ERRORS.find(code);
    if (iter == NAPI_ERRORS.end()) {
        FI_HILOGE("Error code %{public}d not found", code);
        return false;
    }
    codeMsg = iter->second;
    return true;
}

void HandleExecuteResult(napi_env env, int32_t errCode, const std::string param1, const std::string param2)
{
    switch (errCode) {
        case COMMON_PERMISSION_CHECK_ERROR: {
            THROWERR(env, errCode, param1.c_str(), param2.c_str());
            break;
        }
        case COMMON_PARAMETER_ERROR: {
            THROWERR_CUSTOM(env, errCode, "param is invalid");
            break;
        }
        case COMMON_NOT_SYSTEM_APP: {
            std::string errMsg;
            if (!UtilNapiError::GetErrorMsg(errCode, errMsg)) {
                FI_HILOGE("GetErrorMsg failed");
                return;
            }
            THROWERR_CUSTOM(env, errCode, errMsg.c_str());
            break;
        }
        case COMMON_NOT_ALLOWED_DISTRIBUTED: {
            std::string errMsg;
            if (!UtilNapiError::GetErrorMsg(errCode, errMsg)) {
                FI_HILOGE("GetErrorMsg failed");
                return;
            }
            THROWERR_CUSTOM(env, COMMON_PARAMETER_ERROR, errMsg.c_str());
            break;
        }
        default: {
            FI_HILOGW("This error code does not require a synchronous exception throw");
            break;
        }
    }
}
} // namespace UtilNapiError
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS