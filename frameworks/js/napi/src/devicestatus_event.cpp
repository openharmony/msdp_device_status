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

#include "devicestatus_event.h"

#include <js_native_api.h>
#include <map>
#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "devicestatus_common.h"
#include "devicestatus_define.h"

using namespace OHOS::Msdp;
using namespace OHOS::Msdp::DeviceStatus;

namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusEvent" };
constexpr size_t EVENT_MAP_MAX = 20;
} // namespace

DeviceStatusEvent::DeviceStatusEvent(napi_env env)
{
    env_ = env;
}

DeviceStatusEvent::~DeviceStatusEvent()
{
    eventOnceMap_.clear();
    eventMap_.clear();
}

bool DeviceStatusEvent::On(int32_t eventType, napi_value handler, bool isOnce)
{
    FI_HILOGD("On for event:%{public}d, isOnce:%{public}d", eventType, isOnce);
    if ((eventMap_.size() > EVENT_MAP_MAX) || (eventOnceMap_.size() > EVENT_MAP_MAX)) {
        FI_HILOGE("EventMap_ or eventOnceMap_ size over");
        return false;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    CHKPF(scope);
    napi_ref onHandlerRef;
    napi_status status = napi_ok;
    if (isOnce) {
        auto iter = eventOnceMap_.find(eventType);
        if (iter == eventOnceMap_.end()) {
            FI_HILOGD("EventType:%{public}d not exists", eventType);
            eventOnceMap_[eventType] = std::list<std::shared_ptr<DeviceStatusEventListener>>();
        }
        auto listener = std::make_shared<DeviceStatusEventListener>();
        status = napi_create_reference(env_, handler, 1, &onHandlerRef);
        if (status != napi_ok) {
            FI_HILOGE("Failed to create reference");
            napi_close_handle_scope(env_, scope);
            return false;
        }
        listener->onHandlerRef = onHandlerRef;
        eventOnceMap_[eventType].push_back(listener);
        FI_HILOGI("Add once handler to list %{public}d", eventType);
    } else {
        auto iter = eventMap_.find(eventType);
        if (iter == eventMap_.end()) {
            FI_HILOGD("EventType:%{public}d not exists", eventType);
            eventMap_[eventType] = std::list<std::shared_ptr<DeviceStatusEventListener>>();
        }
        auto listener = std::make_shared<DeviceStatusEventListener>();
        status = napi_create_reference(env_, handler, 1, &onHandlerRef);
        if (status != napi_ok) {
            FI_HILOGE("Failed to create reference");
            napi_close_handle_scope(env_, scope);
            return false;
        }
        listener->onHandlerRef = onHandlerRef;
        eventMap_[eventType].push_back(listener);
        FI_HILOGI("Add handler to list %{public}d", eventType);
    }
    napi_close_handle_scope(env_, scope);
    return true;
}

bool DeviceStatusEvent::Off(int32_t eventType, napi_value handler)
{
    FI_HILOGD("DeviceStatusEvent off in for event:%{public}d", eventType);
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        FI_HILOGE("EventType %{public}d not found", eventType);
        return false;
    }
    bool equal = false;
    napi_value result = nullptr;

    for (auto listener : eventMap_[eventType]) {
        napi_status status = napi_get_reference_value(env_, listener->onHandlerRef, &result);
        if (status != napi_ok) {
            FI_HILOGE("Failed to get_reference_value");
            return false;
        }
        status = napi_strict_equals(env_, result, handler, &equal);
        if (status != napi_ok) {
            FI_HILOGE("Failed to strict_equals");
            return false;
        }
        if (equal) {
            FI_HILOGI("Delete handler from list %{public}d", eventType);
            status = napi_delete_reference(env_, listener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Delete failed");
            }
            eventMap_[eventType].remove(listener);
            break;
        }
    }
    FI_HILOGI("%{public}zu listeners in the list of %{public}d",
        eventMap_[eventType].size(), eventType);
    return eventMap_[eventType].empty();
}

bool DeviceStatusEvent::OffOnce(int32_t eventType, napi_value handler)
{
    FI_HILOGD("DeviceStatusEvent OffOnce in for event:%{public}d", eventType);
    auto iter = eventOnceMap_.find(eventType);
    if (iter == eventOnceMap_.end()) {
        FI_HILOGE("EventType %{public}d not found", eventType);
        return false;
    }
    bool equal = false;
    napi_value result = nullptr;
    for (auto listener : eventOnceMap_[eventType]) {
        napi_get_reference_value(env_, listener->onHandlerRef, &result);
        napi_strict_equals(env_, result, handler, &equal);
        if (equal) {
            FI_HILOGI("Delete once handler from list %{public}d", eventType);
            napi_status status = napi_delete_reference(env_, listener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Delete failed");
            }
            eventOnceMap_[eventType].remove(listener);
            break;
        }
    }
    FI_HILOGI("%{public}zu listeners in the once list of %{public}d",
        eventOnceMap_[eventType].size(), eventType);
    return eventMap_[eventType].empty();
}

bool DeviceStatusEvent::RemoveAllCallback(int32_t eventType)
{
    CALL_DEBUG_ENTER;
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        FI_HILOGE("EvenType %{public}d not found", eventType);
        return false;
    }
    eventMap_.erase(eventType);
    return true;
}

void DeviceStatusEvent::CheckRet(int32_t eventType, size_t argc, int32_t value,
    std::shared_ptr<DeviceStatusEventListener> &typeHandler)
{
    CHKPV(typeHandler);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    CHKPV(scope);
    napi_value handler = nullptr;
    napi_status status = napi_ok;
    status = napi_get_reference_value(env_, typeHandler->onHandlerRef, &handler);
    if (status != napi_ok) {
        FI_HILOGE("OnEvent handler for %{public}d failed, status:%{public}d", eventType, status);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value result = nullptr;
    SendRet(eventType, value, result);
    napi_value callResult = nullptr;
    FI_HILOGD("Report to hap");
    status = napi_call_function(env_, nullptr, handler, argc, &result, &callResult);
    if (status != napi_ok) {
        FI_HILOGE("CheckRet:napi_call_function for %{public}d failed, status:%{public}d", eventType, status);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_close_handle_scope(env_, scope);
}

void DeviceStatusEvent::SendRet(int32_t eventType, int32_t value, napi_value &result)
{
    napi_status status = napi_create_object(env_, &result);
    if (status != napi_ok) {
        FI_HILOGE("Failed to create object");
        return;
    }
    napi_value tmpValue = nullptr;
    status = napi_create_int32(env_, eventType, &tmpValue);
    if (status != napi_ok) {
        FI_HILOGE("Failed to create object");
        return;
    }
    status = napi_set_named_property(env_, result, "type", tmpValue);
    if (status != napi_ok) {
        FI_HILOGE("Failed to set name");
        return;
    }

    status = napi_create_int32(env_, value, &tmpValue);
    if (status != napi_ok) {
        FI_HILOGE("Failed to create value");
        return;
    }
    status = napi_set_named_property(env_, result, "value", tmpValue);
    if (status != napi_ok) {
        FI_HILOGE("Failed to set_named");
        return;
    }
}

void DeviceStatusEvent::OnEvent(int32_t eventType, size_t argc, int32_t value, bool isOnce)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("OnEvent for %{public}d, isOnce:%{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    CHKPV(scope);
    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>>::iterator typeHandler;
    if (isOnce) {
        typeHandler = eventOnceMap_.find(eventType);
        if (typeHandler == eventOnceMap_.end()) {
            FI_HILOGE("OnEvent eventType %{public}d not found", eventType);
            napi_close_handle_scope(env_, scope);
            return;
        }
    } else {
        typeHandler = eventMap_.find(eventType);
        if (typeHandler == eventMap_.end()) {
            FI_HILOGE("OnEvent:eventType %{public}d not found", eventType);
            napi_close_handle_scope(env_, scope);
            return;
        }
    }
    FI_HILOGD("%{public}zu callbacks of eventType %{public}d are sent",
        typeHandler->second.size(), eventType);
    for (auto handler : typeHandler->second) {
        CheckRet(eventType, argc, value, handler);
    }
    napi_close_handle_scope(env_, scope);
}

void DeviceStatusEvent::ClearEventMap()
{
    for (const auto &iter : eventMap_) {
        for (const auto &eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Failed to delete reference");
            }
        }
    }
    for (const auto &iter : eventOnceMap_) {
        for (const auto &eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Failed to delete reference");
                napi_delete_reference(env_, eventListener->onHandlerRef);
            }
        }
    }
    eventMap_.clear();
    eventOnceMap_.clear();
}
