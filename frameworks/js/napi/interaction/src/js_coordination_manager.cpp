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
#include "define_interaction.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "JsCoordinationManager" };
} // namespace

napi_value JsCoordinationManager::Enable(napi_env env, bool enable, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    int32_t userData = CoordinationMgrImpl.GetUserData();
    napi_value result = CreateCallbackInfo(env, handle, userData);
    auto callback = std::bind(EmitJsEnable, userData, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->EnableInputDeviceCoordination(enable, callback);
    HandleExecuteResult(env, errCode);
    if (errCode != RET_OK) {
        RemoveCallbackInfo(userData);
    }
    return result;
}

napi_value JsCoordinationManager::Start(napi_env env, const std::string &sinkDeviceDescriptor,
    int32_t srcInputDeviceId, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    int32_t userData = CoordinationMgrImpl.GetUserData();
    napi_value result = CreateCallbackInfo(env, handle, userData);
    auto callback = std::bind(EmitJsStart, userData, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->StartInputDeviceCoordination(sinkDeviceDescriptor, srcInputDeviceId, callback);
    HandleExecuteResult(env, errCode);
    if (errCode != RET_OK) {
        RemoveCallbackInfo(userData);
    }
    return result;
}

napi_value JsCoordinationManager::Stop(napi_env env, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    int32_t userData = CoordinationMgrImpl.GetUserData();
    napi_value result = CreateCallbackInfo(env, handle, userData);
    auto callback = std::bind(EmitJsStop, userData, std::placeholders::_1, std::placeholders::_2);
    int32_t errCode = InteractionMgr->StopDeviceCoordination(callback);
    HandleExecuteResult(env, errCode);
    if (errCode != RET_OK) {
        RemoveCallbackInfo(userData);
    }
    return result;
}

napi_value JsCoordinationManager::GetState(napi_env env, const std::string &deviceDescriptor, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    int32_t userData = CoordinationMgrImpl.GetUserData();
    napi_value result = CreateCallbackInfo(env, handle, userData);
    auto callback = std::bind(EmitJsGetState, userData, std::placeholders::_1);
    int32_t errCode = InteractionMgr->GetInputDeviceCoordinationState(deviceDescriptor, callback);
    HandleExecuteResult(env, errCode);
    if (errCode != RET_OK) {
        RemoveCallbackInfo(userData);
    }
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
