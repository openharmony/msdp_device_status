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

#include "js_event_target.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"
#include "napi_constants.h"
#include "util_napi_error.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "JsEventTarget" };
constexpr std::string_view COORDINATION = "coordination";
std::mutex mutex_;
} // namespace

JsEventTarget::JsEventTarget()
{
    CALL_DEBUG_ENTER;
    auto ret = coordinationListener_.insert({ COORDINATION, std::vector<std::unique_ptr<JsUtil::CallbackInfo>>() });
    CK(ret.second, DeviceStatus::VAL_NOT_EXP);
}

void JsEventTarget::EmitJsEnable(int32_t userData, std::string deviceId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = callback_.find(userData);
    if (iter == callback_.end()) {
        FI_HILOGE("Failed to search for userData");
        return;
    }
    CHKPV(iter->second);
    if (iter->second->env == nullptr) {
        callback_.erase(iter);
        FI_HILOGE("The env is nullptr");
        return;
    }
    iter->second->data.enableResult =
        (msg == CoordinationMessage::OPEN_SUCCESS || msg == CoordinationMessage::CLOSE_SUCCESS) ? true : false;
    iter->second->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(iter->second->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    int32_t *uData = new (std::nothrow) int32_t(userData);
    if (uData == nullptr) {
        FI_HILOGE("uData is nullptr");
        delete work;
        work = nullptr;
        return;
    }
    work->data = static_cast<void*>(uData);
    int32_t result;
    if (iter->second->ref == nullptr) {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallEnablePromiseWork);
    } else {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallEnableAsyncWork);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work failed");
        JsUtil::DeletePtr<uv_work_t*>(work);
        JsUtil::DeletePtr<int32_t*>(uData);
    }
}

void JsEventTarget::EmitJsStart(int32_t userData, std::string deviceId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = callback_.find(userData);
    if (iter == callback_.end()) {
        FI_HILOGE("Failed to search for userData");
        return;
    }
    CHKPV(iter->second);
    if (iter->second->env == nullptr) {
        callback_.erase(iter);
        FI_HILOGE("The env is nullptr");
        return;
    }
    iter->second->data.startResult = (msg == CoordinationMessage::INFO_SUCCESS ? true : false);
    iter->second->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(iter->second->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    int32_t *uData = new (std::nothrow) int32_t(userData);
    if (uData == nullptr) {
        FI_HILOGE("uData is nullptr");
        delete work;
        work = nullptr;
        return;
    }
    work->data = static_cast<void*>(uData);
    int32_t result;
    if (iter->second->ref == nullptr) {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallStartPromiseWork);
    } else {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallStartAsyncWork);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work failed");
        JsUtil::DeletePtr<uv_work_t*>(work);
        JsUtil::DeletePtr<int32_t*>(uData);
    }
}

void JsEventTarget::EmitJsStop(int32_t userData, std::string deviceId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = callback_.find(userData);
    if (iter == callback_.end()) {
        FI_HILOGE("Failed to search for userData");
        return;
    }
    CHKPV(iter->second);
    if (iter->second->env == nullptr) {
        callback_.erase(iter);
        FI_HILOGE("The env is nullptr");
        return;
    }
    iter->second->data.stopResult = (msg == CoordinationMessage::STOP_SUCCESS ? true : false);
    iter->second->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(iter->second->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    int32_t *uData = new (std::nothrow) int32_t(userData);
    if (uData == nullptr) {
        FI_HILOGE("uData is nullptr");
        delete work;
        work = nullptr;
        return;
    }
    work->data = static_cast<void*>(uData);
    int32_t result;
    if (iter->second->ref == nullptr) {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallStopPromiseWork);
    } else {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallStopAsyncWork);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work failed");
        JsUtil::DeletePtr<uv_work_t*>(work);
        JsUtil::DeletePtr<int32_t*>(uData);
    }
}

void JsEventTarget::EmitJsGetState(int32_t userData, bool state)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = callback_.find(userData);
    if (iter == callback_.end()) {
        FI_HILOGE("Failed to search for userData");
        return;
    }
    CHKPV(iter->second);
    if (iter->second->env == nullptr) {
        callback_.erase(iter);
        FI_HILOGE("The env is nullptr");
        return;
    }
    iter->second->data.coordinationOpened = state;
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(iter->second->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    int32_t *uData = new (std::nothrow) int32_t(userData);
    if (uData == nullptr) {
        FI_HILOGE("uData is null");
        delete work;
        work = nullptr;
        return;
    }
    work->data = static_cast<void*>(uData);
    int32_t result;
    if (iter->second->ref == nullptr) {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallGetStatePromiseWork);
    } else {
        result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallGetStateAsyncWork);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work failed");
        JsUtil::DeletePtr<uv_work_t*>(work);
        JsUtil::DeletePtr<int32_t*>(uData);
    }
}

void JsEventTarget::AddListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListener_.find(type);
    if (iter == coordinationListener_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
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
    auto monitor = std::make_unique<JsUtil::CallbackInfo>();
    monitor->env = env;
    monitor->ref = ref;
    iter->second.push_back(std::move(monitor));
    if (!isListeningProcess_) {
        isListeningProcess_ = true;
        InteractionMgr->RegisterCoordinationListener(shared_from_this());
    }
}

void JsEventTarget::RemoveListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListener_.find(type);
    if (iter == coordinationListener_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
        return;
    }
    if (handle == nullptr) {
        iter->second.clear();
        goto monitorLabel;
    }
    for (auto it = iter->second.begin(); it != iter->second.end(); ++it) {
        if (JsUtil::IsSameHandle(env, handle, (*it)->ref)) {
            FI_HILOGE("Success in removing monitor");
            iter->second.erase(it);
            goto monitorLabel;
        }
    }

monitorLabel:
    if (isListeningProcess_ && iter->second.empty()) {
        isListeningProcess_ = false;
        InteractionMgr->UnregisterCoordinationListener(shared_from_this());
    }
}

napi_value JsEventTarget::CreateCallbackInfo(napi_env env, napi_value handle, int32_t userData)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto cb = std::make_unique<JsUtil::CallbackInfo>();
    cb->env = env;
    napi_value promise = nullptr;
    if (handle == nullptr) {
        CHKRP(napi_create_promise(env, &cb->deferred, &promise), CREATE_PROMISE);
    } else {
        CHKRP(napi_create_reference(env, handle, 1, &cb->ref), CREATE_REFERENCE);
    }
    callback_.emplace(userData, std::move(cb));
    return promise;
}

void JsEventTarget::RemoveCallbackInfo(int32_t userData)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = callback_.find(userData);
    if (iter != callback_.end()) {
        callback_.erase(iter);
    }
}

void JsEventTarget::ResetEnv()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    callback_.clear();
    coordinationListener_.clear();
    InteractionMgr->UnregisterCoordinationListener(shared_from_this());
}

void JsEventTarget::OnCoordinationMessage(const std::string &deviceId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto changeEvent = coordinationListener_.find(COORDINATION);
    if (changeEvent == coordinationListener_.end()) {
        FI_HILOGE("Find %{public}s failed", std::string(COORDINATION).c_str());
        return;
    }

    for (auto &item : changeEvent->second) {
        CHKPC(item);
        CHKPC(item->env);
        uv_loop_s *loop = nullptr;
        CHKRV(napi_get_uv_event_loop(item->env, &loop), GET_UV_EVENT_LOOP);
        uv_work_t *work = new (std::nothrow) uv_work_t;
        CHKPV(work);
        item->data.msg = msg;
        item->data.deviceDescriptor = deviceId;
        work->data = static_cast<void*>(&item);
        int32_t result = uv_queue_work(loop, work, [](uv_work_t *work) {}, EmitCoordinationMessageEvent);
        if (result != 0) {
            FI_HILOGE("uv_queue_work failed");
            JsUtil::DeletePtr<uv_work_t*>(work);
        }
    }
}

void JsEventTarget::CallEnablePromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetEnableInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallEnableAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetEnableInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 1, &object, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallStartPromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetStartInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallStartAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetStartInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 1, &object, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallStopPromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetStopInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallStopAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);

    napi_value object = JsUtil::GetStopInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 1, &object, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallGetStatePromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    napi_value object = JsUtil::GetStateInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::CallGetStateAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }
    std::unique_ptr<JsUtil::CallbackInfo> cb = GetCallbackInfo(work);
    CHKPV(cb);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    napi_value resultObj[2];
    CHKRV_SCOPE(cb->env, napi_get_undefined(cb->env, &resultObj[0]), GET_UNDEFINED, scope);
    resultObj[1] = JsUtil::GetStateInfo(cb);
    if (resultObj[1] == nullptr) {
        FI_HILOGE("Object is nullptr");
        napi_close_handle_scope(cb->env, scope);
    }
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 2, resultObj, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventTarget::EmitCoordinationMessageEvent(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtil::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is null");
        return;
    }

    auto temp = static_cast<std::unique_ptr<JsUtil::CallbackInfo>*>(work->data);
    JsUtil::DeletePtr<uv_work_t*>(work);

    auto messageEvent = coordinationListener_.find(COORDINATION);
    if (messageEvent == coordinationListener_.end()) {
        FI_HILOGE("Find messageEvent failed");
        return;
    }

    for (const auto &item : messageEvent->second) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env, &scope);
        CHKPC(item->env);
        if (item->ref != (*temp)->ref) {
            continue;
        }
        napi_value deviceDescriptor = nullptr;
        CHKRV_SCOPE(item->env, napi_create_string_utf8(item->env, item->data.deviceDescriptor.c_str(),
            NAPI_AUTO_LENGTH, &deviceDescriptor), CREATE_STRING_UTF8, scope);
        napi_value eventMsg = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(item->data.msg), &eventMsg),
            CREATE_INT32, scope);
        napi_value object = nullptr;
        CHKRV_SCOPE(item->env, napi_create_object(item->env, &object), CREATE_OBJECT, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "deviceDescriptor", deviceDescriptor),
            SET_NAMED_PROPERTY, scope);
        CHKRV_SCOPE(item->env, napi_set_named_property(item->env, object, "eventMsg", eventMsg),
            SET_NAMED_PROPERTY, scope);

        napi_value handler = nullptr;
        CHKRV_SCOPE(item->env, napi_get_reference_value(item->env, item->ref, &handler), GET_REFERENCE_VALUE, scope);
        napi_value ret = nullptr;
        CHKRV_SCOPE(item->env, napi_call_function(item->env, nullptr, handler, 1, &object, &ret), CALL_FUNCTION, scope);
        napi_close_handle_scope(item->env, scope);
    }
}

std::unique_ptr<JsUtil::CallbackInfo> JsEventTarget::GetCallbackInfo(uv_work_t *work)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    int32_t *uData = static_cast<int32_t*>(work->data);
    int32_t userData = *uData;
    JsUtil::DeletePtr<uv_work_t*>(work);
    JsUtil::DeletePtr<int32_t*>(uData);

    auto iter = callback_.find(userData);
    if (iter == callback_.end()) {
        FI_HILOGE("Find userData failed");
        return nullptr;
    }
    auto cb = std::move(iter->second);
    callback_.erase(iter);
    return cb;
}

void JsEventTarget::HandleExecuteResult(napi_env env, int32_t errCode)
{
    if (errCode != OTHER_ERROR && errCode != RET_OK) {
        NapiError napiError;
        if (!UtilNapiError::GetApiError(errCode, napiError)) {
            FI_HILOGE("This error code could not be found");
            return;
        }
        THROWERR_CUSTOM(env, COMMON_PARAMETER_ERROR, napiError.msg.c_str());
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
