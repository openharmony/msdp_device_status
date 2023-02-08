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

JsDragManager::JsDragManager()
{
    CALL_DEBUG_ENTER;
    auto ret = listeners_.insert({ STATE_TYPE, std::vector<std::unique_ptr<CallbackInfo>>() });
    CK(ret.second, VAL_NOT_EXP);
}

void JsDragManager::RegisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = listeners_.find(type);
    if (iter == listeners_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
        return;
    }
    for (const auto &item : iter->second) {
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
    iter->second.push_back(std::move(monitor));
    if (!hasRegistered_) {
        hasRegistered_ = true;
    }
}

void JsDragManager::UnregisterListener(napi_env env, const std::string &type, napi_value handle)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = listeners_.find(type);
    if (iter == listeners_.end()) {
        FI_HILOGE("Find %{public}s failed", type.c_str());
        return;
    }
    if (handle == nullptr) {
        iter->second.clear();
        goto monitorLabel;
    }
    for (auto it = iter->second.begin(); it != iter->second.end(); ++it) {
        if ((*it)->env == env && IsSameHandle(env, handle, (*it)->ref)) {
            FI_HILOGE("Removing monitor successfully");
            iter->second.erase(it);
            goto monitorLabel;
        }
    }

monitorLabel:
    if (hasRegistered_ && iter->second.empty()) {
        hasRegistered_ = false;
    }
}

void JsDragManager::EmitStartThumbnailDraw(int32_t pixmap, int32_t errCode)
{
    (void)pixmap;
    (void)errCode;
}

void JsDragManager::EmitNoticeThumbnailDraw(int32_t dragStates, int32_t errCode)
{
    (void)dragStates;
    (void)errCode;
}

void JsDragManager::EmitEndThumbnailDraw(int32_t errCode)
{
    (void)errCode;
}

void JsDragManager::ReleaseReference()
{
    for (auto item : thumbnailDrawCb_) {
        if (item != nullptr) {
            if (item->env != nullptr && item->ref != nullptr) {
                napi_delete_reference(item->env, item->ref);
                item->env = nullptr;
                item->ref = nullptr;
            }
        }
    }
}

void JsDragManager::RegisterThumbnailDraw(napi_env env, size_t argc, napi_value* argv)
{
    CALL_INFO_TRACE;
    if (thumbnailDrawCb_.empty()) {
        for (size_t i = 0; i < argc; ++i) {
            auto cb = new (std::nothrow) CallbackInfo();
            thumbnailDrawCb_.push_back(cb);
        }
    }
    for (size_t i = 0; i < argc; ++i) {
        if (thumbnailDrawCb_[i] == nullptr) {
            thumbnailDrawCb_[i] = new (std::nothrow) CallbackInfo();
        }
        CHKPV(thumbnailDrawCb_[i]);
        napi_ref ref = nullptr;
        if (napi_create_reference(env, argv[i], 1, &ref) != napi_ok) {
            ReleaseReference();
            FI_HILOGE("Create reference failed");
            return;
        }
        thumbnailDrawCb_[i]->env = env;
        thumbnailDrawCb_[i]->ref = ref;
    }
    auto startCallback = std::bind(&JsDragManager::EmitStartThumbnailDraw, this,
        std::placeholders::_1, std::placeholders::_2);
    auto noticeCallback = std::bind(&JsDragManager::EmitNoticeThumbnailDraw,
        this, std::placeholders::_1, std::placeholders::_2);
    auto endCallback = std::bind(&JsDragManager::EmitEndThumbnailDraw, this, std::placeholders::_1);
    if (InteractionMgr->RegisterThumbnailDraw(startCallback, noticeCallback, endCallback) != RET_OK) {
        ReleaseReference();
        FI_HILOGE("Call register thumbnail draw failed");
    }
}

void JsDragManager::EmitUnregisterThumbnailDraw(int32_t errCode)
{

}

void JsDragManager::UnregisterThumbnailDraw(napi_env env)
{
    CALL_INFO_TRACE;
    auto callback = std::bind(&JsDragManager::EmitUnregisterThumbnailDraw, this, std::placeholders::_1);
    InteractionMgr->UnregisterThumbnailDraw(callback);
    ReleaseReference();
}

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
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
