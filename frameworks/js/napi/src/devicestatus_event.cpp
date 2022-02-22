/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "devicestatus_event.h"
#include "devicestatus_common.h"

using namespace OHOS::Msdp;

DevicestatusEvent::DevicestatusEvent(napi_env env, napi_value thisVar)
{
    env_ = env;
    thisVarRef_ = nullptr;
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

DevicestatusEvent::~DevicestatusEvent()
{
    for (auto iter = eventMap_.begin(); iter != eventMap_.end(); iter++) {
        auto listener = iter->second;
        napi_delete_reference(env_, listener->handlerRef);
    }
    eventMap_.clear();
    napi_delete_reference(env_, thisVarRef_);
}

bool DevicestatusEvent::On(const int32_t& eventType, napi_value handler, bool isOnce)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, \
        "DevicestatusEvent On in for event: %{public}d, isOnce: %{public}d", eventType, isOnce);

    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator iter;
    if (isOnce) {
        iter = eventOnceMap_.find(eventType);
        if (iter != eventOnceMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "eventType: %{public}d already exists", eventType);
            return false;
        }
    } else {
        iter = eventMap_.find(eventType);
        if (iter != eventMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "eventType: %{public}d already exists", eventType);
            return false;
        }
    }
    auto listener = std::make_shared<DevicestatusEventListener>();
    listener->eventType = eventType;
    napi_create_reference(env_, handler, 1, &listener->handlerRef);
    if (isOnce) {
        eventOnceMap_[eventType] = listener;
    } else {
        eventMap_[eventType] = listener;
    }
    return true;
}

bool DevicestatusEvent::Off(const int32_t& eventType, bool isOnce)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, \
        "DevicestatusEvent off in for event: %{public}d, isOnce: %{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "scope is nullptr");
        return false;
    }

    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator iter;
    if (isOnce) {
        iter = eventOnceMap_.find(eventType);
        if (iter == eventOnceMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "eventType %{public}d not find", eventType);
            return false;
        }
    } else {
        iter = eventMap_.find(eventType);
        if (iter == eventMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "eventType %{public}d not find", eventType);
            return false;
        }
    }

    auto listener = iter->second;
    napi_delete_reference(env_, listener->handlerRef);
    if (isOnce) {
        eventOnceMap_.erase(eventType);
    } else {
        eventMap_.erase(eventType);
    }
    napi_close_handle_scope(env_, scope);
    return true;
}

void DevicestatusEvent::OnEvent(const int32_t& eventType, size_t argc, const int32_t& value, bool isOnce)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "OnEvent for %{public}d, isOnce: %{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "scope is nullptr");
        return;
    }

    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator iter;
    if (isOnce) {
        iter = eventOnceMap_.find(eventType);
        if (iter == eventOnceMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "OnEvent: eventType %{public}d not find", eventType);
            return;
        }
    } else {
        iter = eventMap_.find(eventType);
        if (iter == eventMap_.end()) {
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "OnEvent: eventType %{public}d not find", eventType);
            return;
        }
    }

    auto listener = iter->second;
    napi_value thisVar = nullptr;
    napi_status status = napi_get_reference_value(env_, thisVarRef_, &thisVar);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, \
            "OnEvent napi_get_reference_value thisVar for %{public}d failed, status=%{public}d", eventType, status);
        return;
    }
    napi_value handler = nullptr;
    status = napi_get_reference_value(env_, listener->handlerRef, &handler);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, \
            "OnEvent napi_get_reference_value handler for %{public}d failed, status=%{public}d", eventType, status);
        return;
    }
    napi_value callResult = nullptr;
    napi_value result;
    napi_create_object(env_, &result);
    JsResponse jsResponse;
    jsResponse.devicestatusValue_ = value;

    napi_value tmpValue;
    napi_create_int32(env_, jsResponse.devicestatusValue_, &tmpValue);
    napi_set_named_property(env_, result, "devicestatusValue", tmpValue);

    status = napi_call_function(env_, thisVar, handler, argc, &result, &callResult);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, \
            "OnEvent: napi_call_function for %{public}d failed, status=%{public}d", eventType, status);
        return;
    }
    napi_close_handle_scope(env_, scope);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
}
