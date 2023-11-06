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

#include "js_event_cooperate_target.h"

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"
#include "napi_constants.h"
#include "util_napi_error.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "JsEventCooperateTarget" };
inline constexpr std::string_view COORDINATION { "cooperation" };
inline constexpr std::string_view CREATE_PROMISE { "napi_create_promise" };
inline constexpr std::string_view GET_UNDEFINED { "napi_get_undefined" };
inline constexpr std::string_view RESOLVE_DEFERRED { "napi_resolve_deferred" };
inline constexpr std::string_view REJECT_DEFERRED { "napi_reject_deferred" };
inline constexpr std::string_view CLOSE_SCOPE { "napi_close_handle_scope" };
std::mutex mutex_;
} // namespace

JsEventCooperateTarget::JsEventCooperateTarget()
{
    CALL_DEBUG_ENTER;
    auto ret = coordinationListeners_.insert({ COORDINATION,
        std::vector<sptr<JsUtilCooperate::CallbackInfo>>()});
    if (!ret.second) {
        FI_HILOGW("Failed to insert, errCode:%{public}d", static_cast<int32_t>(DeviceStatus::VAL_NOT_EXP));
    }
}

void JsEventCooperateTarget::EmitJsEnable(sptr<JsUtilCooperate::CallbackInfo> cb,
    const std::string &networkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.enableResult = (msg == CoordinationMessage::PREPARE || msg == CoordinationMessage::UNPREPARE);
    cb->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(cb->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    cb->IncStrongRef(nullptr);
    work->data = cb.GetRefPtr();
    int32_t result = 0;
    if (cb->ref == nullptr) {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallEnablePromiseWork, uv_qos_default);
    } else {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallEnableAsyncWork, uv_qos_default);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work_with_qos failed");
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        cb->DecStrongRef(nullptr);
    }
}

void JsEventCooperateTarget::EmitJsStart(sptr<JsUtilCooperate::CallbackInfo> cb,
    const std::string &remoteNetworkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.startResult = (msg == CoordinationMessage::ACTIVATE_SUCCESS);
    cb->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(cb->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    cb->IncStrongRef(nullptr);
    work->data = cb.GetRefPtr();
    int32_t result = 0;
    if (cb->ref == nullptr) {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallStartPromiseWork, uv_qos_default);
    } else {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallStartAsyncWork, uv_qos_default);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work_with_qos failed");
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        cb->DecStrongRef(nullptr);
    }
}

void JsEventCooperateTarget::EmitJsStop(sptr<JsUtilCooperate::CallbackInfo> cb,
    const std::string &networkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.stopResult = (msg == CoordinationMessage::DEACTIVATE_SUCCESS);
    cb->data.errCode = static_cast<int32_t>(msg);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(cb->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    cb->IncStrongRef(nullptr);
    work->data = cb.GetRefPtr();
    int32_t result = 0;
    if (cb->ref == nullptr) {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallStopPromiseWork, uv_qos_default);
    } else {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallStopAsyncWork, uv_qos_default);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work_with_qos failed");
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        cb->DecStrongRef(nullptr);
    }
}

void JsEventCooperateTarget::EmitJsGetState(sptr<JsUtilCooperate::CallbackInfo> cb, bool state)
{
    CALL_INFO_TRACE;
    CHKPV(cb);
    CHKPV(cb->env);
    cb->data.coordinationOpened = state;
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(cb->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_s *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    cb->IncStrongRef(nullptr);
    work->data = cb.GetRefPtr();
    int32_t result = 0;
    if (cb->ref == nullptr) {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallGetStatePromiseWork, uv_qos_default);
    } else {
        result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {}, CallGetStateAsyncWork, uv_qos_default);
    }

    if (result != 0) {
        FI_HILOGE("uv_queue_work_with_qos failed");
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        cb->DecStrongRef(nullptr);
    }
}

void JsEventCooperateTarget::AddListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListeners_.find(type);
    if (iter == coordinationListeners_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
        return;
    }

    for (const auto &item : iter->second) {
        CHKPC(item);
        if (JsUtilCooperate::IsSameHandle(env, handle, item->ref)) {
            FI_HILOGE("The handle already exists");
            return;
        }
    }
    napi_ref ref = nullptr;
    CHKRV(napi_create_reference(env, handle, 1, &ref), CREATE_REFERENCE);
    sptr<JsUtilCooperate::CallbackInfo> monitor = new (std::nothrow) JsUtilCooperate::CallbackInfo();
    CHKPV(monitor);
    monitor->env = env;
    monitor->ref = ref;
    iter->second.push_back(std::move(monitor));
    if (!isListeningProcess_) {
        isListeningProcess_ = true;
        INTERACTION_MGR->RegisterCoordinationListener(shared_from_this());
    }
}

void JsEventCooperateTarget::RemoveListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = coordinationListeners_.find(type);
    if (iter == coordinationListeners_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
        return;
    }
    if (handle == nullptr) {
        iter->second.clear();
        goto monitorLabel;
    }
    for (auto it = iter->second.begin(); it != iter->second.end(); ++it) {
        if (JsUtilCooperate::IsSameHandle(env, handle, (*it)->ref)) {
            FI_HILOGE("Success in removing monitor");
            iter->second.erase(it);
            goto monitorLabel;
        }
    }

monitorLabel:
    if (isListeningProcess_ && iter->second.empty()) {
        isListeningProcess_ = false;
        INTERACTION_MGR->UnregisterCoordinationListener(shared_from_this());
    }
}

napi_value JsEventCooperateTarget::CreateCallbackInfo(napi_env env,
    napi_value handle, sptr<JsUtilCooperate::CallbackInfo> cb)
{
    CALL_INFO_TRACE;
    CHKPP(cb);
    cb->env = env;
    napi_value promise = nullptr;
    if (handle == nullptr) {
        CHKRP(napi_create_promise(env, &cb->deferred, &promise), CREATE_PROMISE);
    } else {
        CHKRP(napi_create_reference(env, handle, 1, &cb->ref), CREATE_REFERENCE);
    }
    return promise;
}

void JsEventCooperateTarget::ResetEnv()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    INTERACTION_MGR->UnregisterCoordinationListener(shared_from_this());
}

void JsEventCooperateTarget::OnCoordinationMessage(const std::string &networkId, CoordinationMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto changeEvent = coordinationListeners_.find(COORDINATION);
    if (changeEvent == coordinationListeners_.end()) {
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
        item->data.deviceDescriptor = networkId;
        item->IncStrongRef(nullptr);
        work->data = item.GetRefPtr();
        int32_t result = uv_queue_work_with_qos(loop, work, [](uv_work_t *work) {},
            EmitCoordinationMessageEvent, uv_qos_default);
        if (result != 0) {
            FI_HILOGE("uv_queue_work_with_qos failed");
            item->DecStrongRef(nullptr);
            JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        }
    }
}

void JsEventCooperateTarget::CallEnablePromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetEnableInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventCooperateTarget::CallEnableAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetEnableInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
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

void JsEventCooperateTarget::CallStartPromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetStartInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventCooperateTarget::CallStartAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetStartInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
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

void JsEventCooperateTarget::CallStopPromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetStopInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }

    napi_valuetype valueType = napi_undefined;
    if (napi_typeof(cb->env, object, &valueType) != napi_ok) {
        FI_HILOGE("napi typeof failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    if (valueType != napi_undefined) {
        CHKRV_SCOPE(cb->env, napi_reject_deferred(cb->env, cb->deferred, object), REJECT_DEFERRED, scope);
    } else {
        CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventCooperateTarget::CallStopAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetStopInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
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

void JsEventCooperateTarget::CallGetStatePromiseWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value object = JsUtilCooperate::GetStateInfo(cb);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    CHKRV_SCOPE(cb->env, napi_resolve_deferred(cb->env, cb->deferred, object), RESOLVE_DEFERRED, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventCooperateTarget::CallGetStateAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<JsUtilCooperate::CallbackInfo> cb(static_cast<JsUtilCooperate::CallbackInfo *>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    if (scope == nullptr) {
        FI_HILOGE("scope is nullptr");
        RELEASE_CALLBACKINFO(cb->env, cb->ref);
        return;
    }
    napi_value resultObj[2];
    CHKRV_SCOPE(cb->env, napi_get_undefined(cb->env, &resultObj[0]), GET_UNDEFINED, scope);
    resultObj[1] = JsUtilCooperate::GetStateInfo(cb);
    if (resultObj[1] == nullptr) {
        FI_HILOGE("Object is nullptr");
        napi_close_handle_scope(cb->env, scope);
    }
    napi_value handlerInfo = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref, &handlerInfo), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    size_t argc = TWO_PARAM;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handlerInfo, argc, resultObj, &result),
        CALL_FUNCTION, scope);
    RELEASE_CALLBACKINFO(cb->env, cb->ref);
    napi_close_handle_scope(cb->env, scope);
}

void JsEventCooperateTarget::EmitCoordinationMessageEvent(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(work);
    if (work->data == nullptr) {
        JsUtilCooperate::DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }

    sptr<JsUtilCooperate::CallbackInfo> temp(static_cast<JsUtilCooperate::CallbackInfo*>(work->data));
    JsUtilCooperate::DeletePtr<uv_work_t*>(work);
    temp->DecStrongRef(nullptr);
    auto messageEvent = coordinationListeners_.find(COORDINATION);
    if (messageEvent == coordinationListeners_.end()) {
        FI_HILOGE("Find messageEvent failed");
        return;
    }
    for (const auto &item : messageEvent->second) {
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env, &scope);
        CHKPC(item->env);
        if (item->ref != temp->ref) {
            continue;
        }
        napi_value deviceDescriptor = nullptr;
        CHKRV_SCOPE(item->env, napi_create_string_utf8(item->env, item->data.deviceDescriptor.c_str(),
            NAPI_AUTO_LENGTH, &deviceDescriptor), CREATE_STRING_UTF8, scope);
        napi_value eventMsg = nullptr;
        auto iter = messageTransform.find(item->data.msg);
        if (iter == messageTransform.end()) {
            FI_HILOGE("Find message code failed");
            CHKRV(napi_close_handle_scope(item->env, scope), CLOSE_SCOPE);
            return;
        }
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(iter->second), &eventMsg),
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

void JsEventCooperateTarget::HandleExecuteResult(napi_env env, int32_t errCode)
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
