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

#include "js_cooperate_manager.h"

#include "devicestatus_define.h"
#include "interaction_manager.h"
#include "util_napi_error.h"

#undef LOG_TAG
#define LOG_TAG "JsCooperateManager"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

napi_value JsCooperateManager::Enable(napi_env env, bool enable, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtilCooperate::CallbackInfo> cb = new (std::nothrow) JsUtilCooperate::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsEnable, cb, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = 0;
    if (enable) {
        errCode = INTERACTION_MGR->PrepareCoordination(callback);
    } else {
        errCode = INTERACTION_MGR->UnprepareCoordination(callback);
    }
    if (errCode != RET_OK) {
        UtilNapiError::HandleExecuteResult(env, errCode, "enable", COOPERATE_PERMISSION);
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    return result;
}

napi_value JsCooperateManager::Start(napi_env env, const std::string &remoteNetworkDescriptor,
    int32_t startDeviceId, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtilCooperate::CallbackInfo> cb = new (std::nothrow) JsUtilCooperate::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsStart, cb, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = INTERACTION_MGR->ActivateCoordination(remoteNetworkDescriptor, startDeviceId, callback);
    if (errCode != RET_OK) {
        UtilNapiError::HandleExecuteResult(env, errCode, "start", COOPERATE_PERMISSION);
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    return result;
}

napi_value JsCooperateManager::Stop(napi_env env, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtilCooperate::CallbackInfo> cb = new (std::nothrow) JsUtilCooperate::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsStop, cb, std::placeholders::_1, std::placeholders::_2);
    bool isUnchained = false;
    int32_t errCode = INTERACTION_MGR->DeactivateCoordination(isUnchained, callback);
    if (errCode != RET_OK) {
        UtilNapiError::HandleExecuteResult(env, errCode, "stop", COOPERATE_PERMISSION);
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    return result;
}

napi_value JsCooperateManager::GetState(napi_env env, const std::string &deviceDescriptor, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtilCooperate::CallbackInfo> cb = new (std::nothrow) JsUtilCooperate::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsGetState, cb, std::placeholders::_1);
    int32_t errCode = INTERACTION_MGR->GetCoordinationState(deviceDescriptor, callback);
    if (errCode != RET_OK) {
        UtilNapiError::HandleExecuteResult(env, errCode, "getState", COOPERATE_PERMISSION);
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    return result;
}

void JsCooperateManager::RegisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    AddListener(env, type, handle);
}

void JsCooperateManager::UnregisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    RemoveListener(env, type, handle);
}

void JsCooperateManager::ResetEnv()
{
    CALL_INFO_TRACE;
    JsEventCooperateTarget::ResetEnv();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
