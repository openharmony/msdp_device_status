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

#include "js_event_target.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"
#include "napi_constants.h"
#include "util_napi_error.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "JsEventTarget"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
std::mutex mutex_;
inline constexpr std::string_view CREATE_PROMISE { "napi_create_promise" };
inline constexpr std::string_view GET_UNDEFINED { "napi_get_undefined" };
inline constexpr std::string_view RESOLVE_DEFERRED { "napi_resolve_deferred" };
inline constexpr std::string_view REJECT_DEFERRED { "napi_reject_deferred" };
} // namespace

JsEventTarget::JsEventTarget()
{
    CALL_DEBUG_ENTER;
    auto runner = AppExecFwk::EventRunner::GetMainEventRunner();
    if (runner != nullptr) {
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    auto ret = coordinationListeners_.insert({ COOPERATE_NAME, std::vector<sptr<JsUtil::CallbackInfo>>() });
    if (!ret.second) {
        FI_HILOGW("Failed to insert, errCode:%{public}d", static_cast<int32_t>(DeviceStatus::VAL_NOT_EXP));
    }
}

void JsEventTarget::EmitJsPrepare(sptr<JsUtil::CallbackInfo> cb, const std::string &networkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.prepareResult = (msg == CoordinationMessage::PREPARE || msg == CoordinationMessage::UNPREPARE);
    cb->data.errCode = static_cast<int32_t>(msg);
    auto task = [cb]() {
        FI_HILOGI("Execute lambda");
        if (cb->ref == nullptr) {
            CallPreparePromiseWork(cb);
        } else {
            CallPrepareAsyncWork(cb);
        }
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::EmitJsActivate(sptr<JsUtil::CallbackInfo> cb, const std::string &remoteNetworkId,
    CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.activateResult = (msg == CoordinationMessage::ACTIVATE_SUCCESS);
    cb->data.errCode = static_cast<int32_t>(msg);
    auto task = [cb]() {
        FI_HILOGI("Execute lambda");
        if (cb->ref == nullptr) {
            CallActivatePromiseWork(cb);
        } else {
            CallActivateAsyncWork(cb);
        }
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::EmitJsDeactivate(sptr<JsUtil::CallbackInfo> cb, const std::string &networkId,
    CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.deactivateResult = (msg == CoordinationMessage::DEACTIVATE_SUCCESS);
    cb->data.errCode = static_cast<int32_t>(msg);
        auto task = [cb]() {
        FI_HILOGI("Execute lambda");
        if (cb->ref == nullptr) {
            CallDeactivatePromiseWork(cb);
        } else {
            CallDeactivateAsyncWork(cb);
        }
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::EmitJsGetCrossingSwitchState(sptr<JsUtil::CallbackInfo> cb, bool state)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.coordinationOpened = state;
    auto task = [cb]() {
        FI_HILOGI("Execute lambda");
        if (cb->ref == nullptr) {
            CallGetCrossingSwitchStatePromiseWork(cb);
        } else {
            CallGetCrossingSwitchStateAsyncWork(cb);
        }
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::AddListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::string listenerType = type;
    bool isCompatible = false;
    if (type == COOPERATE_MESSAGE_NAME) {
        isCompatible = true;
        listenerType = COOPERATE_NAME;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListeners_.find(listenerType);
    if (iter == coordinationListeners_.end()) {
        FI_HILOGE("Not exist %{public}s", listenerType.c_str());
        return;
    }

    for (const auto &item : iter->second) {
        CHKPC(item);
        if (JsUtil::IsSameHandle(env, handle, item->ref)) {
            FI_HILOGE("The handle already exists");
            return;
        }
    }
    napi_ref ref = nullptr;
    CHKRV(napi_create_reference(env, handle, 1, &ref), CREATE_REFERENCE);
    sptr<JsUtil::CallbackInfo> monitor = new (std::nothrow) JsUtil::CallbackInfo();
    if (monitor == nullptr) {
        FI_HILOGE("Null monitor, release callback");
        RELEASE_CALLBACKINFO(env, ref);
        return;
    }
    monitor->env = env;
    monitor->ref = ref;
    monitor->data.type = type;
    iter->second.push_back(monitor);
    if (!isListeningProcess_) {
        int32_t errCode = INTERACTION_MGR->RegisterCoordinationListener(shared_from_this(), isCompatible);
        if (errCode != RET_OK) {
            UtilNapiError::HandleExecuteResult(env, errCode, "on", COOPERATE_PERMISSION);
            RELEASE_CALLBACKINFO(env, ref);
        } else {
            isListeningProcess_ = true;
        }
    }
}

void JsEventTarget::RemoveListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::string listenerType = type;
    bool isCompatible = false;
    if (type == COOPERATE_MESSAGE_NAME) {
        isCompatible = true;
        listenerType = COOPERATE_NAME;
    }
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListeners_.find(listenerType);
    if (iter == coordinationListeners_.end()) {
        FI_HILOGE("Not exist %{public}s", listenerType.c_str());
        return;
    }
    if (handle == nullptr) {
        iter->second.clear();
        goto MONITOR_LABEL;
    }
    for (auto it = iter->second.begin(); it != iter->second.end(); ++it) {
        if (JsUtil::IsSameHandle(env, handle, (*it)->ref)) {
            FI_HILOGE("Success in removing monitor");
            iter->second.erase(it);
            goto MONITOR_LABEL;
        }
    }

MONITOR_LABEL:
    if (iter->second.empty() && isListeningProcess_) {
        int32_t errCode = INTERACTION_MGR->UnregisterCoordinationListener(shared_from_this(), isCompatible);
        if (errCode == RET_OK) {
            isListeningProcess_ = false;
        } else {
            UtilNapiError::HandleExecuteResult(env, errCode, "off", COOPERATE_PERMISSION);
        }
    }
}

void JsEventTarget::AddListener(napi_env env, const std::string &type, const std::string &networkId, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (IsHandleExist(env, networkId, handle)) {
        FI_HILOGE("Current handle for networkId:%{public}s already exists", Utility::Anonymize(networkId));
        return;
    }
    napi_ref ref = nullptr;
    CHKRV(napi_create_reference(env, handle, 1, &ref), CREATE_REFERENCE);
    sptr<JsUtil::MouseCallbackInfo> monitor = new (std::nothrow) JsUtil::MouseCallbackInfo();
    if (monitor == nullptr) {
        FI_HILOGE("Null monitor, release callback");
        RELEASE_CALLBACKINFO(env, ref);
        return;
    }
    monitor->env = env;
    monitor->ref = ref;
    mouseLocationListeners_[networkId].push_back(monitor);
    if (int32_t errCode = INTERACTION_MGR->RegisterEventListener(networkId, shared_from_this());
        errCode != RET_OK) {
        FI_HILOGE("RegisterEventListener for networkId:%{public}s failed, ret:%{public}d",
            Utility::Anonymize(networkId), errCode);
        UtilNapiError::HandleExecuteResult(env, errCode, "on", COOPERATE_PERMISSION);
        RELEASE_CALLBACKINFO(env, ref);
    }
}

void JsEventTarget::RemoveListener(napi_env env, const std::string &type, const std::string &networkId,
    napi_value handle)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (mouseLocationListeners_.find(networkId) == mouseLocationListeners_.end()) {
        FI_HILOGE("Not listener for networkId:%{public}s exists", Utility::Anonymize(networkId));
        return;
    }
    if (handle == nullptr) {
        FI_HILOGI("Remove all listener for networkId:%{public}s", Utility::Anonymize(networkId));
        mouseLocationListeners_.erase(networkId);
    } else {
        for (auto iter = mouseLocationListeners_[networkId].begin();
            iter != mouseLocationListeners_[networkId].end();) {
            if (JsUtil::IsSameHandle(env, handle, (*iter)->ref)) {
                iter = mouseLocationListeners_[networkId].erase(iter);
                break;
            } else {
                ++iter;
            }
        }
    }
    if (mouseLocationListeners_.find(networkId) != mouseLocationListeners_.end() &&
        mouseLocationListeners_[networkId].empty()) {
        mouseLocationListeners_.erase(networkId);
    }
    if (int32_t errCode = INTERACTION_MGR->UnregisterEventListener(networkId, shared_from_this());
        errCode != RET_OK) {
        FI_HILOGE("UnregisterEventListener for networkId:%{public}s failed, ret:%{public}d",
            Utility::Anonymize(networkId), errCode);
        UtilNapiError::HandleExecuteResult(env, errCode, "off", COOPERATE_PERMISSION);
    }
}

napi_value JsEventTarget::CreateCallbackInfo(napi_env env, napi_value handle, sptr<JsUtil::CallbackInfo> callback)
{
    CALL_INFO_TRACE;
    CHKPP(callback);
    callback->env = env;
    napi_value napiPromise = nullptr;
    if (handle == nullptr) {
        CHKRP(napi_create_promise(env, &callback->deferred, &napiPromise), CREATE_PROMISE);
    } else {
        CHKRP(napi_create_reference(env, handle, 1, &callback->ref), CREATE_REFERENCE);
    }
    return napiPromise;
}

napi_value JsEventTarget::CreateMouseCallbackInfo(napi_env env, napi_value handle,
    sptr<JsUtil::MouseCallbackInfo> callback)
{
    CALL_INFO_TRACE;
    CHKPP(callback);
    callback->env = env;
    napi_value napiPromise = nullptr;
    if (handle == nullptr) {
        CHKRP(napi_create_promise(env, &callback->deferred, &napiPromise), CREATE_PROMISE);
    } else {
        CHKRP(napi_create_reference(env, handle, 1, &callback->ref), CREATE_REFERENCE);
    }
    return napiPromise;
}

void JsEventTarget::ResetEnv()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    INTERACTION_MGR->UnregisterCoordinationListener(shared_from_this());
}

void JsEventTarget::OnCoordinationMessage(const std::string &networkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto changeEvent = coordinationListeners_.find(COOPERATE_NAME);
    if (changeEvent == coordinationListeners_.end()) {
        FI_HILOGE("Find %{public}s failed", std::string(COOPERATE_NAME).c_str());
        return;
    }
    JsUtil::CallbackInfo cooMessageEvent;
    cooMessageEvent.data = {
        .deviceDescriptor = networkId,
        .msg = msg,
    };
    auto task = [cooMessageEvent]() {
        FI_HILOGI("Execute lamdba");
        EmitCoordinationMessageEvent(cooMessageEvent);
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::OnMouseLocationEvent(const std::string &networkId, const Event &event)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    if (mouseLocationListeners_.find(networkId) == mouseLocationListeners_.end()) {
        FI_HILOGE("Find listener for %{public}s failed", Utility::Anonymize(networkId));
        return;
    }
    JsUtil::MouseCallbackData mouseEvent = {
        .networkId = networkId,
        .displayX = event.displayX,
        .displayY = event.displayY,
        .displayWidth = event.displayWidth,
        .displayHeight = event.displayHeight,
    };
    auto task = [mouseEvent]() {
        FI_HILOGI("Execute lamdba");
        EmitMouseLocationEvent(mouseEvent);
    };
    CHKPV(eventHandler_);
    eventHandler_->PostTask(task);
}

void JsEventTarget::CallPreparePromiseWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope handleScope = nullptr;
    napi_open_handle_scope(cb->env, &handleScope);
    if (handleScope == nullptr) {
        FI_HILOGE("Prepare promise, handleScope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtil::GetPrepareInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("Prepare promise, object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }
    napi_valuetype napiValueType = napi_undefined;
    if (napi_typeof(cb->env, object, &napiValueType) != napi_ok) {
        FI_HILOGE("Prepare promise, napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }
    if (napiValueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, handleScope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, handleScope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, handleScope);
}

void JsEventTarget::CallPrepareAsyncWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("Prepare async, scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtil::GetPrepareInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("Prepare async, object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value processor = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &processor), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, processor, 1, &object, &result), CALL_FUNCTION, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallActivatePromiseWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope handleScope = nullptr;
    napi_open_handle_scope(cb->env, &handleScope);
    if (handleScope == nullptr) {
        FI_HILOGE("Activate promise, handleScope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value napiObject = JsUtil::GetActivateInfo(cb);
    if (napiObject == nullptr) {
        FI_HILOGE("Activate promise, napiObject is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, napiObject, &valueType) != napi_ok) {
        FI_HILOGE("Activate promise, napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, napiObject), REJECT_DEFERRED, handleScope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, napiObject), RESOLVE_DEFERRED, handleScope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, handleScope);
}

void JsEventTarget::CallActivateAsyncWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("Activate async, scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtil::GetActivateInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("Activate async, object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value ret = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 1, &object, &ret), CALL_FUNCTION, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallDeactivatePromiseWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope handleScope = nullptr;
    napi_open_handle_scope(cb->env, &handleScope);
    if (handleScope == nullptr) {
        FI_HILOGE("Deactivate promise, handleScope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtil::GetDeactivateInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("Deactivate promise, object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("Deactivate promise, napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, handleScope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, handleScope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, handleScope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, handleScope);
}

void JsEventTarget::CallDeactivateAsyncWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("Deactivate async, scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtil::GetDeactivateInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("Deactivate async, object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 1, &object, &result), CALL_FUNCTION, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallGetCrossingSwitchStatePromiseWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("Switch state, scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value state = JsUtil::GetCrossingSwitchStateInfo(cb);
    if (state == nullptr) {
        FI_HILOGE("Switch state, state is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, state), RESOLVE_DEFERRED, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallGetCrossingSwitchStateAsyncWork(sptr<JsUtil::CallbackInfo>cb)
{
    CALL_INFO_TRACE;
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("The scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value resultObj[2];
    CHKRV_SCOPE(cb->env, napi_get_undefined(cb->env, &resultObj[0]), GET_UNDEFINED, scope);
    resultObj[1] = JsUtil::GetCrossingSwitchStateInfo(cb);
    if (resultObj[1] == nullptr) {
        FI_HILOGE("The object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    size_t argc = TWO_PARAM;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, argc, resultObj, &result),
        CALL_FUNCTION, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::EmitCoordinationMessageEvent(const JsUtil::CallbackInfo &cooMessageEvent)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto messageEvent = coordinationListeners_.find(COOPERATE_NAME);
    if (messageEvent == coordinationListeners_.end()) {
        FI_HILOGE("Not exit messageEvent");
        return;
    }

    for (const auto &item : messageEvent->second) {
        CHKPC(item->env);
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env, &scope);
        napi_value deviceDescriptor = nullptr;
        CHKRV_SCOPE(item->env, napi_create_string_utf8(item->env, cooMessageEvent.data.deviceDescriptor.c_str(),
            NAPI_AUTO_LENGTH, &deviceDescriptor), CREATE_STRING_UTF8, scope);
        napi_value eventMsg = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(cooMessageEvent.data.msg), &eventMsg),
            CREATE_INT32, scope);
        napi_value object = nullptr;
        CHKRV_SCOPE(item->env, napi_create_object(item->env, &object), CREATE_OBJECT, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "networkId", deviceDescriptor),
            SET_NAMED_PROPERTY, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object,
            ((cooMessageEvent.data.type == COOPERATE_MESSAGE_NAME) ? "CooperateState" : "msg"), eventMsg),
            SET_NAMED_PROPERTY, scope);

        napi_value handler = nullptr;
        CHKRV_SCOPE(item->env, napi_get_reference_value(item->env, item->ref, &handler), GET_REFERENCE_VALUE, scope);
        napi_value ret = nullptr;
        CHKRV_SCOPE(item->env, napi_call_function(item->env, nullptr, handler, 1, &object, &ret), CALL_FUNCTION, scope);
        napi_close_handle_scope(item->env, scope);
    }
}

void JsEventTarget::EmitMouseLocationEvent(const JsUtil::MouseCallbackData &mouseEvent)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    auto mouseLocationEvent = mouseLocationListeners_.find(mouseEvent.networkId);
    if (mouseLocationEvent == mouseLocationListeners_.end()) {
        FI_HILOGE("Not exist mouseLocationEvent");
        return;
    }
    for (const auto &item : mouseLocationEvent->second) {
        if (item->env == nullptr) {
            FI_HILOGW("Item->env is nullptr, skip then continue");
            continue;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env, &scope);

        napi_value displayX = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(mouseEvent.displayX), &displayX),
            CREATE_INT32, scope);
        napi_value displayY = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(mouseEvent.displayY), &displayY),
            CREATE_INT32, scope);
        napi_value displayWidth = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(mouseEvent.displayWidth),
            &displayWidth), CREATE_INT32, scope);
        napi_value displayHeight = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(mouseEvent.displayHeight),
            &displayHeight), CREATE_INT32, scope);

        napi_value object = nullptr;
        CHKRV_SCOPE(item->env, napi_create_object(item->env, &object), CREATE_OBJECT, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "displayX", displayX),
            SET_NAMED_PROPERTY, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "displayY", displayY),
            SET_NAMED_PROPERTY, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "displayWidth", displayWidth),
            SET_NAMED_PROPERTY, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "displayHeight", displayHeight),
            SET_NAMED_PROPERTY, scope);

        napi_value handler = nullptr;
        CHKRV_SCOPE(item->env, napi_get_reference_value(item->env, item->ref, &handler), GET_REFERENCE_VALUE, scope);
        napi_value ret = nullptr;
        CHKRV_SCOPE(item->env, napi_call_function(item->env, nullptr, handler, 1, &object, &ret), CALL_FUNCTION, scope);
        napi_close_handle_scope(item->env, scope);
    }
}

bool JsEventTarget::IsHandleExist(napi_env env, const std::string &networkId, napi_value handle)
{
    if (mouseLocationListeners_.find(networkId) == mouseLocationListeners_.end()) {
        FI_HILOGW("No handle of networkId:%{public}s exists", Utility::Anonymize(networkId));
        return false;
    }
    for (const auto &item : mouseLocationListeners_[networkId]) {
        CHKPC(item);
        if (JsUtil::IsSameHandle(env, handle, item->ref)) {
            FI_HILOGE("The handle already exists");
            return true;
        }
    }
    return false;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
