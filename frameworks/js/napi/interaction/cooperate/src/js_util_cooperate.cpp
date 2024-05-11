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

#include "js_util_cooperate.h"

#include "devicestatus_define.h"
#include "napi_constants.h"
#include "util_napi_error.h"

#undef LOG_TAG
#define LOG_TAG "JsUtilCooperate"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
inline constexpr std::string_view GET_BOOLEAN { "napi_get_boolean" };
inline constexpr std::string_view COERCE_TO_BOOL { "napi_coerce_to_bool" };
inline constexpr std::string_view CREATE_ERROR { "napi_create_error" };
} // namespace

napi_value JsUtilCooperate::GetEnableInfo(sptr<CallbackInfo> cb)
{
    CHKPP(cb);
    CHKPP(cb->env);
    return GetResult(cb->env, cb->data.enableResult, cb->data.errCode);
}

napi_value JsUtilCooperate::GetStartInfo(sptr<CallbackInfo> cb)
{
    CHKPP(cb);
    CHKPP(cb->env);
    return GetResult(cb->env, cb->data.startResult, cb->data.errCode);
}

napi_value JsUtilCooperate::GetStopInfo(sptr<CallbackInfo> cb)
{
    CHKPP(cb);
    CHKPP(cb->env);
    return GetResult(cb->env, cb->data.stopResult, cb->data.errCode);
}

napi_value JsUtilCooperate::GetStateInfo(sptr<CallbackInfo> cb)
{
    CHKPP(cb);
    CHKPP(cb->env);
    napi_value ret = nullptr;
    napi_value state = nullptr;
    CHKRP(napi_create_int32(cb->env, cb->data.coordinationOpened ? 1 : 0, &ret),
        CREATE_INT32);
    CHKRP(napi_coerce_to_bool(cb->env, ret, &state), COERCE_TO_BOOL);
    return state;
}

napi_value JsUtilCooperate::GetResult(napi_env env, bool result, int32_t errorCode)
{
    CHKPP(env);
    napi_value object = nullptr;
    if (result) {
        napi_get_undefined(env, &object);
        return object;
    }
    std::string errMsg;
    if (!GetErrMsg(errorCode, errMsg)) {
        FI_HILOGE("This errCode:%{public}d could not be found", errorCode);
        return nullptr;
    }
    napi_value resultCode = nullptr;
    CHKRP(napi_create_int32(env, errorCode, &resultCode), CREATE_INT32);
    napi_value resultMessage = nullptr;
    CHKRP(napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &resultMessage),
        CREATE_STRING_UTF8);
    CHKRP(napi_create_error(env, nullptr, resultMessage, &object), CREATE_ERROR);
    CHKRP(napi_set_named_property(env, object, ERR_CODE.c_str(), resultCode), SET_NAMED_PROPERTY);
    return object;
}

bool JsUtilCooperate::GetErrMsg(int32_t errCode, std::string &msg)
{
    auto iter = COOPERATE_MSG_MAP.find(errCode);
    if (iter == COOPERATE_MSG_MAP.end()) {
        FI_HILOGE("Error code:%{public}d not found", errCode);
        return false;
    }
    msg = iter->second;
    return true;
}

napi_value JsUtilCooperate::GetStateResult(napi_env env, bool result)
{
    CHKPP(env);
    napi_value state = nullptr;
    CHKRP(napi_get_boolean(env, result, &state), GET_BOOLEAN);
    napi_value object = nullptr;
    CHKRP(napi_create_object(env, &object), CREATE_OBJECT);
    CHKRP(napi_set_named_property(env, object, "state", state), SET_NAMED_PROPERTY);
    return object;
}

bool JsUtilCooperate::IsSameHandle(napi_env env, napi_value handle, napi_ref ref)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHKPF(scope);
    napi_value handlerTemp = nullptr;
    CHKRF_SCOPE(env, napi_get_reference_value(env, ref, &handlerTemp), GET_REFERENCE_VALUE, scope);
    bool isSame = false;
    CHKRF_SCOPE(env, napi_strict_equals(env, handle, handlerTemp, &isSame), STRICT_EQUALS, scope);
    napi_close_handle_scope(env, scope);
    return isSame;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
