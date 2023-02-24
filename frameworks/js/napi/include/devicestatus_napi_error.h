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

#ifndef MSDP_DEVICE_STATUS_DEVICESTATUS_NAPI_ERROR_H
#define MSDP_DEVICE_STATUS_DEVICESTATUS_NAPI_ERROR_H

#include <map>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "devicestatus_hilog_wrapper.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
const int32_t SERVICE_EXCEPTION = 201;
const int32_t PARAM_ERROR = 202;
const std::map <int32_t, std::string> ERROR_MESSAGES = {
    {SERVICE_EXCEPTION, "Service exception."},
    {PARAM_ERROR, "Param error."},
};

napi_value CreateNapiError(const napi_env &env, const int32_t errCode, const std::string &errMessage);
std::optional <std::string> GetErrMsg(int32_t errorCode);
void ThrowErr(const napi_env &env, const int32_t errCode, const std::string &printMsg);
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif //MSDP_DEVICE_STATUS_DEVICESTATUS_NAPI_ERROR_H
