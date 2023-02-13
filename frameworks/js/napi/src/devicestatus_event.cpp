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

using namespace OHOS::Msdp;

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
    DEV_HILOGD(JS_NAPI, "On for event:%{public}d, isOnce:%{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGE(JS_NAPI, "scope is nullptr");
        return false;
    }
    napi_ref onHandlerRef;
    napi_status status = napi_ok;
    if (isOnce) {
        auto iter = eventOnceMap_.find(eventType);
        if (iter == eventOnceMap_.end()) {
            DEV_HILOGD(JS_NAPI, "eventType:%{public}d not exists", eventType);
            eventOnceMap_[eventType] = std::list<std::shared_ptr<DeviceStatusEventListener>>();
        }
        auto listener = std::make_shared<DeviceStatusEventListener>();
        status = napi_create_reference(env_, handler, 1, &onHandlerRef);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to create reference");
            napi_close_handle_scope(env_, scope);
            return false;
        }
        listener->onHandlerRef = onHandlerRef;
        eventOnceMap_[eventType].push_back(listener);
        DEV_HILOGI(JS_NAPI, "Add once handler to list %{public}d", eventType);
    } else {
        auto iter = eventMap_.find(eventType);
        if (iter == eventMap_.end()) {
            DEV_HILOGD(JS_NAPI, "eventType:%{public}d not exists", eventType);
            eventMap_[eventType] = std::list<std::shared_ptr<DeviceStatusEventListener>>();
        }
        auto listener = std::make_shared<DeviceStatusEventListener>();
        status = napi_create_reference(env_, handler, 1, &onHandlerRef);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to create reference");
            napi_close_handle_scope(env_, scope);
            return false;
        }
        listener->onHandlerRef = onHandlerRef;
        eventMap_[eventType].push_back(listener);
        DEV_HILOGI(JS_NAPI, "Add handler to list %{public}d", eventType);
    }
    napi_close_handle_scope(env_, scope);
    return true;
}

bool DeviceStatusEvent::Off(int32_t eventType, napi_value handler)
{
    DEV_HILOGD(JS_NAPI, "DeviceStatusEvent off in for event:%{public}d", eventType);

    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        DEV_HILOGE(JS_NAPI, "eventType %{public}d not find", eventType);
        return false;
    }
    bool equal = false;
    napi_value result = nullptr;

    for (auto listener : eventMap_[eventType]) {
        napi_status status = napi_get_reference_value(env_, listener->onHandlerRef, &result);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get_reference_value");
            return false;
        }
        status = napi_strict_equals(env_, result, handler, &equal);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to strict_equals");
            return false;
        }
        if (equal) {
            DEV_HILOGI(JS_NAPI, "delete handler from list %{public}d", eventType);
            status = napi_delete_reference(env_, listener->onHandlerRef);
            if (status != napi_ok) {
                DEV_HILOGW(JS_NAPI, "Delete failed");
            }
            eventMap_[eventType].remove(listener);
            break;
        }
    }
    DEV_HILOGI(JS_NAPI, "%{public}zu listeners in the list of %{public}d",
        eventMap_[eventType].size(), eventType);
    return eventMap_[eventType].empty();
}

bool DeviceStatusEvent::OffOnce(int32_t eventType, napi_value handler)
{
    DEV_HILOGD(JS_NAPI, "DeviceStatusEvent OffOnce in for event:%{public}d", eventType);

    auto iter = eventOnceMap_.find(eventType);
    if (iter == eventOnceMap_.end()) {
        DEV_HILOGE(JS_NAPI, "eventType %{public}d not find", eventType);
        return false;
    }
    bool equal = false;
    napi_value result = nullptr;
    for (auto listener : eventOnceMap_[eventType]) {
        napi_get_reference_value(env_, listener->onHandlerRef, &result);
        napi_strict_equals(env_, result, handler, &equal);
        if (equal) {
            DEV_HILOGI(JS_NAPI, "delete once handler from list %{public}d", eventType);
            napi_status status = napi_delete_reference(env_, listener->onHandlerRef);
            if (status != napi_ok) {
                DEV_HILOGW(JS_NAPI, "Delete failed");
            }
            eventOnceMap_[eventType].remove(listener);
            break;
        }
    }
    DEV_HILOGI(JS_NAPI, "%{public}zu listeners in the once list of %{public}d",
        eventOnceMap_[eventType].size(), eventType);
    return eventMap_[eventType].empty();
}

void DeviceStatusEvent::CheckRet(int32_t eventType, size_t argc, int32_t value,
    std::shared_ptr<DeviceStatusEventListener> &typeHandler)
{
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGE(JS_NAPI, "scope is nullptr");
        return;
    }
    napi_value handler = nullptr;
    napi_status status = napi_ok;
    status = napi_get_reference_value(env_, typeHandler->onHandlerRef, &handler);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "OnEvent handler for %{public}d failed, status:%{public}d", eventType, status);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_value result;
    SendRet(eventType, value, result);
    napi_value callResult = nullptr;
    DEV_HILOGD(JS_NAPI, "Report to hap");
    status = napi_call_function(env_, nullptr, handler, argc, &result, &callResult);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "CheckRet:napi_call_function for %{public}d failed, status:%{public}d", eventType, status);
        napi_close_handle_scope(env_, scope);
        return;
    }
    napi_close_handle_scope(env_, scope);
}

void DeviceStatusEvent::SendRet(int32_t eventType, int32_t value, napi_value &result)
{
    napi_status status = napi_create_object(env_, &result);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to create object");
        return;
    }
    DEV_HILOGD(JS_NAPI, "eventType:%{public}d,value:%{public}d", eventType, value);
    napi_value tmpValue = nullptr;
    status = napi_create_int32(env_, eventType, &tmpValue);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to create object");
        return;
    }
    status = napi_set_named_property(env_, result, "type", tmpValue);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to set name");
        return;
    }
    if (value >= static_cast<int32_t>(DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID)
        && value <= static_cast<int32_t>(DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT)) {
        status = napi_create_int32(env_, value, &tmpValue);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get int32");
            return;
        }
    }
    status = napi_set_named_property(env_, result, "value", tmpValue);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to set_named");
        return;
    }
}

void DeviceStatusEvent::OnEvent(int32_t eventType, size_t argc, int32_t value, bool isOnce)
{
    DEV_HILOGD(JS_NAPI, "OnEvent for %{public}d, isOnce:%{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGE(JS_NAPI, "scope is nullptr");
        return;
    }

    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>>::iterator typeHandler;
    if (isOnce) {
        typeHandler = eventOnceMap_.find(eventType);
        if (typeHandler == eventOnceMap_.end()) {
            DEV_HILOGE(JS_NAPI, "OnEvent eventType %{public}d not find", eventType);
            napi_close_handle_scope(env_, scope);
            return;
        }
    } else {
        typeHandler = eventMap_.find(eventType);
        if (typeHandler == eventMap_.end()) {
            DEV_HILOGE(JS_NAPI, "OnEvent:eventType %{public}d not find", eventType);
            napi_close_handle_scope(env_, scope);
            return;
        }
    }
    DEV_HILOGD(JS_NAPI, "%{public}zu callbacks of eventType %{public}d are sent",
        typeHandler->second.size(), eventType);
    for (auto handler : typeHandler->second) {
        CheckRet(eventType, argc, value, handler);
    }
    napi_close_handle_scope(env_, scope);
    DEV_HILOGD(JS_NAPI, "Exit");
}

void DeviceStatusEvent::ClearEventMap()
{
    for (auto iter : eventMap_) {
        for (auto eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                DEV_HILOGW(JS_NAPI, "Failed to delete reference");
            }
        }
    }
    for (auto iter : eventOnceMap_) {
        for (auto eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                DEV_HILOGW(JS_NAPI, "Failed to delete reference");
                napi_delete_reference(env_, eventListener->onHandlerRef);
            }
        }
    }
    eventMap_.clear();
    eventOnceMap_.clear();
}
