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