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

#include "js_drag_manager.h"

#include "devicestatus_define.h"
#include "interaction_manager.h"
#include "napi_constants.h"
#include "util_napi.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "JsDragManager" };
} // namespace

void JsDragManager::ResetEnv()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    listeners_.clear();
}

bool JsDragManager::IsSameHandle(napi_env env, napi_value handle, napi_ref ref)
{
    napi_value handlerTemp = nullptr;
    CHKRF(napi_get_reference_value(env, ref, &handlerTemp), GET_REFERENCE_VALUE);
    bool isEqual = false;
    CHKRF(napi_strict_equals(env, handle, handlerTemp, &isEqual), STRICT_EQUALS);
    return isEqual;
}

void JsDragManager::RegisterListener(napi_env env, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    for (const auto &item : listeners_) {
        CHKPC(item);
        if (item->env == env && IsSameHandle(env, handle, item->ref)) {
            FI_HILOGE("The handle already exists");
            return;
        }
    }
    napi_ref ref = nullptr;
    CHKRV(napi_create_reference(env, handle, 1, &ref), CREATE_REFERENCE);
    auto monitor = std::make_unique<CallbackInfo>();
    monitor->env = env;
    monitor->ref = ref;
    listeners_.push_back(std::move(monitor));
    if (!hasRegistered_) {
        hasRegistered_ = true;
        InteractionMgr->AddDraglistener(shared_from_this());
    }
}

void JsDragManager::UnregisterListener(napi_env env, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (listeners_.empty()) {
        FI_HILOGE("The listener list is empty");
        return;
    }
    for (auto iter = listeners_.begin(); iter != listeners_.end();) {
        if (handle == nullptr) {
            RELEASE_CALLBACKINFO((*iter)->env, (*iter)->ref);
            iter = listeners_.erase(iter);
        } else {
            if ((*iter)->env == env && IsSameHandle(env, handle, (*iter)->ref)) {
                FI_HILOGD("Removing monitor successfully");
                RELEASE_CALLBACKINFO((*iter)->env, (*iter)->ref);
                iter = listeners_.erase(iter);
                break;
            }
            ++iter;
        }
    }
    if (hasRegistered_ && listeners_.empty()) {
        hasRegistered_ = false;
        InteractionMgr->RemoveDraglistener(shared_from_this());
    }
}

void JsDragManager::EmitStartThumbnailDraw(std::shared_ptr<DragData> dragData)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(thumbnailDrawCb_);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(thumbnailDrawCb_->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_t *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    thumbnailDrawCb_->dragData = dragData;
    work->data = thumbnailDrawCb_.GetRefPtr();
    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallStartThumbnailDrawAsyncWork);
    if (ret != 0) {
        FI_HILOGE("uv_queue_work failed");
        DeletePtr<uv_work_t*>(work);
    }
}

void JsDragManager::EmitNoticeThumbnailDraw(int32_t msgType, bool allowDragIn, std::u16string message)
{
    CALL_INFO_TRACE;
    CHKPV(thumbnailDrawCb_);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(thumbnailDrawCb_->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_t *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    thumbnailDrawCb_->msgType = msgType;
    thumbnailDrawCb_->allowDragIn = allowDragIn;
    thumbnailDrawCb_->message = message;
    work->data = thumbnailDrawCb_.GetRefPtr();
    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallNoticeThumbnailDrawAsyncWork);
    if (ret != 0) {
        FI_HILOGE("uv_queue_work failed");
        DeletePtr<uv_work_t*>(work);
    }
}

void JsDragManager::EmitEndThumbnailDraw(int32_t pid, int32_t result)
{
    CALL_INFO_TRACE;
    CHKPV(thumbnailDrawCb_);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(thumbnailDrawCb_->env, &loop), GET_UV_EVENT_LOOP);
    uv_work_t *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    work->data = thumbnailDrawCb_.GetRefPtr();
    int32_t ret = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallEndThumbnailDrawAsyncWork);
    if (ret != 0) {
        FI_HILOGE("uv_queue_work failed");
        DeletePtr<uv_work_t*>(work);
    }
}

void JsDragManager::ReleaseReference()
{
    CHKPV(thumbnailDrawCb_);
    CHKPV(thumbnailDrawCb_->env);
    for (auto item : thumbnailDrawCb_->ref) {
        if (item == nullptr) {
            continue;
        }
        if (napi_delete_reference(thumbnailDrawCb_->env, item) != napi_ok) {
            FI_HILOGW("Delete reference failed");
            continue;
        }
    }
    thumbnailDrawCb_->env = nullptr;
    thumbnailDrawCb_ = nullptr;
}

void JsDragManager::RegisterThumbnailDraw(napi_env env, size_t argc, napi_value* argv)
{
    CALL_INFO_TRACE;
    if (thumbnailDrawCb_ != nullptr) {
        ReleaseReference();
    }
    thumbnailDrawCb_ = new (std::nothrow) ThumbnailDrawCb();
    CHKPV(thumbnailDrawCb_);
    thumbnailDrawCb_->env = env;
    for (size_t i = 0; i < 3; ++i) {
        napi_ref ref = nullptr;
        if (napi_create_reference(env, argv[i], 1, &ref) != napi_ok) {
            ReleaseReference();
            FI_HILOGE("Create reference failed");
            return;
        }
        thumbnailDrawCb_->ref[i] = ref;
    }
    auto startCallback = std::bind(&JsDragManager::EmitStartThumbnailDraw, this,
        std::placeholders::_1);
    auto noticeCallback = std::bind(&JsDragManager::EmitNoticeThumbnailDraw,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto endCallback = std::bind(&JsDragManager::EmitEndThumbnailDraw, this,
        std::placeholders::_1, std::placeholders::_2);
    if (InteractionMgr->RegisterThumbnailDraw(startCallback, noticeCallback, endCallback) != RET_OK) {
        ReleaseReference();
        FI_HILOGE("Call register thumbnail draw failed");
    }
}

void JsDragManager::EmitUnregisterThumbnailDraw(sptr<ThumbnailDrawCb> callbackInfo)
{
    CALL_INFO_TRACE;
    CHKPV(callbackInfo);
    uv_loop_s *loop = nullptr;
    CHKRV(napi_get_uv_event_loop(callbackInfo->env, &loop), GET_UV_EVENT_LOOP);
    callbackInfo->errCode = RET_OK;
    uv_work_t *work = new (std::nothrow) uv_work_t;
    CHKPV(work);
    callbackInfo->IncStrongRef(nullptr);
    work->data = callbackInfo.GetRefPtr();
    int32_t result = uv_queue_work(loop, work, [](uv_work_t *work) {}, CallUnregisterThumbnailDrawAsyncWork);
    if (result != 0) {
        FI_HILOGE("uv_queue_work failed");
        DeletePtr(work);
        callbackInfo->DecStrongRef(nullptr);
    }
}

void JsDragManager::UnregisterThumbnailDraw(napi_env env, napi_value argv)
{
    CALL_INFO_TRACE;
    ReleaseReference();
    napi_ref ref = nullptr;
    if (napi_create_reference(env, argv, 1, &ref) != napi_ok) {
        FI_HILOGE("Create reference failed");
        return;
    }
    sptr<ThumbnailDrawCb> cb = new (std::nothrow) ThumbnailDrawCb();
    CHKPV(cb);
    cb->env = env;
    cb->ref[0] = ref;
    auto callback = std::bind(&JsDragManager::EmitUnregisterThumbnailDraw, this, cb);
    if (InteractionMgr->UnregisterThumbnailDraw(callback) != RET_OK) {
        FI_HILOGE("Call Unregister thumbnail draw failed");
    }
}
napi_value JsDragManager::GetDragOption(sptr<ThumbnailDrawCb> cb)
{
    return nullptr;
}

napi_value JsDragManager::GetNoticeMsg(sptr<ThumbnailDrawCb> cb)
{
    return nullptr;
}

void JsDragManager::CallStartThumbnailDrawAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_DEBUG_ENTER;
    CHKPV(work);
    if (work->data == nullptr) {
        DeletePtr(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<ThumbnailDrawCb> cb(static_cast<ThumbnailDrawCb *>(work->data));
    DeletePtr(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    CHKPV(cb->ref[0]);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    CHKPV(scope);
    napi_value callResult[2] = { 0 };
    if (GetErrorCode(cb->env, cb->errCode, &callResult[0]) != RET_OK){
        FI_HILOGE("Get error code failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    callResult[1] = GetDragOption(cb);
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref[0], &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 2, callResult, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsDragManager::CallNoticeThumbnailDrawAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_DEBUG_ENTER;
    CHKPV(work);
    if (work->data == nullptr) {
        DeletePtr(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<ThumbnailDrawCb> cb(static_cast<ThumbnailDrawCb *>(work->data));
    DeletePtr(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    CHKPV(cb->ref[1]);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    CHKPV(scope);
    napi_value callResult[2] = { 0 };
    if (GetErrorCode(cb->env, cb->errCode, &callResult[0]) != RET_OK){
        FI_HILOGE("Get error code failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    callResult[1] = GetNoticeMsg(cb);
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref[1], &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 2, callResult, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

void JsDragManager::CallEndThumbnailDrawAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_DEBUG_ENTER;
    CHKPV(work);
    if (work->data == nullptr) {
        DeletePtr(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<ThumbnailDrawCb> cb(static_cast<ThumbnailDrawCb *>(work->data));
    DeletePtr(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    CHKPV(cb->ref[2]);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    CHKPV(scope);
    napi_value callResult[3] = { 0 };
    if (GetErrorCode(cb->env, cb->errCode, &callResult[0]) != RET_OK){
        FI_HILOGE("Get error code failed");
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    
    napi_value handler = nullptr;
    CHKRV_SCOPE(cb->env, napi_get_reference_value(cb->env, cb->ref[2], &handler), GET_REFERENCE_VALUE, scope);
    napi_value result = nullptr;
    CHKRV_SCOPE(cb->env, napi_call_function(cb->env, nullptr, handler, 3, callResult, &result), CALL_FUNCTION, scope);
    napi_close_handle_scope(cb->env, scope);
}

napi_value JsDragManager::GreateBusinessError(napi_env env, int32_t errCode, std::string errMessage)
{
    CALL_DEBUG_ENTER;
    napi_value result = nullptr;
    napi_value resultCode = nullptr;
    napi_value resultMessage = nullptr;
    CHKRP(napi_create_int32(env, errCode, &resultCode), CREATE_INT32);
    CHKRP(napi_create_string_utf8(env, errMessage.data(), NAPI_AUTO_LENGTH, &resultMessage), CREATE_STRING_UTF8);
    CHKRP(napi_create_error(env, nullptr, resultMessage, &result), CREATE_ERROR);
    CHKRP(napi_set_named_property(env, result, ERR_CODE.c_str(), resultCode), SET_NAMED_PROPERTY);
    return result;
}

int32_t JsDragManager::GetErrorCode(napi_env env, int32_t errCode, napi_value* callResult)
{
    if (callResult == nullptr) {
        FI_HILOGE("callResult is nullptr");
        return RET_ERR;
    }
    if (errCode == RET_OK) {
        if (napi_get_undefined(env, callResult) != napi_ok) {
            FI_HILOGE(" get undefined failed");
            return RET_ERR;
        }
        return RET_OK;
    }
    if (errCode == RET_ERR) {
        FI_HILOGE("Other errors");
        return RET_ERR;
    }
    NapiError codeMsg;
    if (!UtilNapiError::GetApiError(errCode, codeMsg)) {
        FI_HILOGE("Error code %{public}d not found", errCode);
        return RET_ERR;
    }
    *callResult = GreateBusinessError(env, errCode, codeMsg.msg);
    if (*callResult == nullptr) {
        FI_HILOGE("callResult is nullptr");
    }
    return RET_ERR;
}

void JsDragManager::CallUnregisterThumbnailDrawAsyncWork(uv_work_t *work, int32_t status)
{
    CALL_DEBUG_ENTER;
    CHKPV(work);
    if (work->data == nullptr) {
        DeletePtr(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    sptr<ThumbnailDrawCb> cb(static_cast<ThumbnailDrawCb *>(work->data));
    DeletePtr(work);
    cb->DecStrongRef(nullptr);
    CHKPV(cb->env);
    CHKPV(cb->ref[0]);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(cb->env, &scope);
    CHKPV(scope);
    napi_value callResult = nullptr;
    if (GetErrorCode(cb->env, cb->errCode, &callResult) != RET_OK){
        FI_HILOGE("Get error code failed");
        RELEASE_CALLBACKINFO(cb->env, cb->ref[0]);
        napi_close_handle_scope(cb->env, scope);
        return;
    }
    napi_value handler = nullptr;
    if (napi_get_reference_value(cb->env, cb->ref[0], &handler) != napi_ok) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref[0]);
        napi_close_handle_scope(cb->env, scope);
        FI_HILOGE(" napi_get_reference_value failed");
        return;
    }
    napi_value result = nullptr;
    if (napi_call_function(cb->env, nullptr, handler, 1, &callResult, &result) != napi_ok) {
        RELEASE_CALLBACKINFO(cb->env, cb->ref[0]);
        napi_close_handle_scope(cb->env, scope);
        FI_HILOGE("napi_call_function failed");
        return;
    }
    RELEASE_CALLBACKINFO(cb->env, cb->ref[0]);
    napi_close_handle_scope(cb->env, scope);
}


void JsDragManager::OnDragMessage(DragMessage msg)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (listeners_.empty()) {
        FI_HILOGE("The listener list is empty");
        return;
    }
    for (auto &item : listeners_) {
        CHKPC(item);
        CHKPC(item->env);
        uv_loop_s* loop = nullptr;
        CHKRV(napi_get_uv_event_loop(item->env, &loop), GET_UV_EVENT_LOOP);
        uv_work_t* work = new (std::nothrow) uv_work_t;
        CHKPV(work);
        item->msg = msg;
        work->data = static_cast<void*>(&item);
        int32_t result = uv_queue_work(loop, work, [](uv_work_t* work) {}, CallDragMsg);
        if (result != 0) {
            FI_HILOGE("uv_queue_work failed");
            DeletePtr<uv_work_t*>(work);
        }
    }
}

void JsDragManager::CallDragMsg(uv_work_t *work, int32_t status)
{
    CALL_INFO_TRACE;
    CHKPV(work);
    if (work->data == nullptr) {
        DeletePtr<uv_work_t*>(work);
        FI_HILOGE("Check data is nullptr");
        return;
    }
    auto temp = static_cast<std::unique_ptr<CallbackInfo>*>(work->data);
    DeletePtr<uv_work_t*>(work);
    std::lock_guard<std::mutex> guard(mutex_);
    if (listeners_.empty()) {
        FI_HILOGE("The listener list is empty");
        return;
    }
    for (const auto &item : listeners_) {
        CHKPC(item->env);
        if (item->ref != (*temp)->ref) {
            continue;
        }
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(item->env, &scope);
        CHKPC(scope);
        napi_value stateMsg = nullptr;
        CHKRV_SCOPE(item->env, napi_create_int32(item->env, static_cast<int32_t>(item->msg), &stateMsg),
            CREATE_INT32, scope);
        napi_value handler = nullptr;
        CHKRV_SCOPE(item->env, napi_get_reference_value(item->env, item->ref, &handler), GET_REFERENCE_VALUE, scope);
        napi_value ret = nullptr;
        CHKRV_SCOPE(item->env,
            napi_call_function(item->env, nullptr, handler, 1, &stateMsg, &ret), CALL_FUNCTION, scope);
        napi_close_handle_scope(item->env, scope);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
