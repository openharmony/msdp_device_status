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

#include "js_coordination_manager.h"

#include "coordination_manager_impl.h"
#include "devicestatus_define.h"
#include "interaction_manager.h"
#include "util_napi.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "JsCoordinationManager" };
} // namespace

napi_value JsCoordinationManager::Enable(napi_env env, bool enable, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtil::CallbackInfo> cb = new (std::nothrow) JsUtil::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsEnable, cb, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->EnableCoordination(enable, callback);
    if (errCode != RET_OK) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    HandleExecuteResult(env, errCode);
    return result;
}

napi_value JsCoordinationManager::Start(napi_env env, const std::string &remoteNetworkDescriptor,
    int32_t startDeviceId, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtil::CallbackInfo> cb = new (std::nothrow) JsUtil::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsStart, cb, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->StartCoordination(remoteNetworkDescriptor, startDeviceId, callback);
    if (errCode != RET_OK) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    HandleExecuteResult(env, errCode);
    return result;
}

napi_value JsCoordinationManager::Stop(napi_env env, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtil::CallbackInfo> cb = new (std::nothrow) JsUtil::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsStop, cb, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->StopCoordination(callback);
    if (errCode != RET_OK) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    HandleExecuteResult(env, errCode);
    return result;
}

napi_value JsCoordinationManager::GetState(napi_env env, const std::string &deviceDescriptor, napi_value handle)
{
    CALL_INFO_TRACE;
    sptr<JsUtil::CallbackInfo> cb = new (std::nothrow) JsUtil::CallbackInfo();
    CHKPP(cb);
    napi_value result = CreateCallbackInfo(env, handle, cb);
    auto callback = std::bind(EmitJsGetState, cb, std::placeholders::_1);
    int32_t errCode = InteractionMgr->GetCoordinationState(deviceDescriptor, callback);
    if (errCode != RET_OK) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
    }
    HandleExecuteResult(env, errCode);
    return result;
}

void JsCoordinationManager::RegisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    AddListener(env, type, handle);
}

void JsCoordinationManager::UnregisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    RemoveListener(env, type, handle);
}

void JsCoordinationManager::ResetEnv()
{
    CALL_INFO_TRACE;
    JsEventTarget::ResetEnv();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
