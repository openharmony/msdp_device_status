/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <uv.h>
#include <map>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include <js_native_api.h>
using namespace OHOS::Msdp;

DevicestatusEvent::DevicestatusEvent(napi_env env, napi_value thisVar)
{
    env_ = env;
    thisVarRef_ = nullptr;
    napi_create_reference(env, thisVar, 1, &thisVarRef_);
}

DevicestatusEvent::~DevicestatusEvent()
{
    eventOnceMap_.clear();
    eventMap_.clear();
    napi_delete_reference(env_, thisVarRef_);
}

bool DevicestatusEvent::On(const int32_t& eventType, napi_value handler, bool isOnce)
{
    DEV_HILOGI(JS_NAPI, \
        "DevicestatusEvent On in for event: %{public}d, isOnce: %{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGI(JS_NAPI, "scope is nullptr");
        return false;
    }
    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator iter;
    napi_ref onHandlerRef;
    if (isOnce) {
        iter = eventOnceMap_.find(eventType);
        if (iter != eventOnceMap_.end()) {
            DEV_HILOGI(JS_NAPI, "eventType: %{public}d already exists", eventType);
            iter->second->refCount++;
            napi_create_reference(env_, handler, 1, &(onHandlerRef));
            iter->second->onHandlerRefSet.insert(onHandlerRef);
        } else {
            auto listener = std::make_shared<DevicestatusEventListener>();
            std::set<napi_ref>  OnRefSet;
            listener->refCount = 1;
            napi_create_reference(env_, handler, 1, &onHandlerRef);
            OnRefSet.insert(onHandlerRef);
            listener->onHandlerRefSet = OnRefSet;
            eventOnceMap_[eventType] = listener;
        }
    } else {
        iter = eventMap_.find(eventType);
        if (iter != eventMap_.end()) {
            DEV_HILOGI(JS_NAPI, "eventType: %{public}d already exists", eventType);
            iter->second->refCount++;
            napi_create_reference(env_, handler, 1, &(onHandlerRef));
            iter->second->onHandlerRefSet.insert(onHandlerRef);
        } else {
            auto listener = std::make_shared<DevicestatusEventListener>();
            std::set<napi_ref>  OnRefSet;
            listener->refCount = 1;
            napi_create_reference(env_, handler, 1, &onHandlerRef);
            OnRefSet.insert(onHandlerRef);

            listener->onHandlerRefSet = OnRefSet;
            eventMap_[eventType] = listener;
        }
    }
    napi_close_handle_scope(env_, scope);
    return true;
}

bool DevicestatusEvent::Off(const int32_t& eventType, bool isOnce)
{
    DEV_HILOGI(JS_NAPI, \
        "DevicestatusEvent off in for event: %{public}d, isOnce: %{public}d", eventType, isOnce);
    
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGI(JS_NAPI, "scope is nullptr");
        return false;
    }

    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator iter;
    if (isOnce) {
        iter = eventOnceMap_.find(eventType);
        if (iter == eventOnceMap_.end()) {
            DEV_HILOGI(JS_NAPI, "eventType %{public}d not find", eventType);
            return false;
        }
    } else {
        iter = eventMap_.find(eventType);
        if (iter == eventMap_.end()) {
            DEV_HILOGI(JS_NAPI, "eventType %{public}d not find", eventType);
            return false;
        }
    }
    iter->second->refCount--;
    if (iter->second->refCount == 0) {
        eventOnceMap_.erase(eventType);
    }
    napi_close_handle_scope(env_, scope);
    return true;
}

napi_value DevicestatusEvent::EnumDevicestatusEventConstructor(napi_env env, napi_callback_info info)
{
    DEV_HILOGI(JS_NAPI, "Enter");
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    DEV_HILOGI(JS_NAPI, "Exit");
    return thisArg;
}

void DevicestatusEvent::OnEvent(const int32_t& eventType, size_t argc, const int32_t& value, bool isOnce)
{
    DEV_HILOGI(JS_NAPI, "OnEvent for %{public}d, isOnce: %{public}d", eventType, isOnce);
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env_, &scope);
    if (scope == nullptr) {
        DEV_HILOGI(JS_NAPI, "scope is nullptr");
        return;
    }

    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>>::iterator typeIter;
    if (isOnce) {
        typeIter = eventOnceMap_.find(eventType);
        if (typeIter == eventOnceMap_.end()) {
            DEV_HILOGI(JS_NAPI, "OnEvent: eventType %{public}d not find", eventType);
            return;
        }
    } else {
        typeIter = eventMap_.find(eventType);
        if (typeIter == eventMap_.end()) {
            DEV_HILOGI(JS_NAPI, "OnEvent: eventType %{public}d not find", eventType);
            return;
        }
    }

    for (auto iter: typeIter->second->onHandlerRefSet) {
        napi_value handler = nullptr;
        napi_value thisVar = nullptr;
        napi_status status = napi_ok;
        status = napi_get_reference_value(env_, iter, &handler);
        if (status != napi_ok) {
            DEV_HILOGI(JS_NAPI, \
                "OnEvent napi_get_reference_value handler for %{public}d failed, status=%{public}d", eventType, status);
            return;
        }
        napi_value callResult = nullptr;
        bool flag = value == 0 ? false : true;
        napi_value tmpValue = nullptr;
        napi_value result;
        napi_create_object(env_, &result);

        // int32_t type = 0;
        napi_create_int32(env_,eventType,&tmpValue);
        napi_set_named_property(env_,result,"type",tmpValue);
        napi_get_boolean(env_, flag, &tmpValue);
        napi_set_named_property(env_, result, "value", tmpValue);

        status = napi_call_function(env_, thisVar, handler, argc, &result, &callResult);
        if (status != napi_ok) {
            DEV_HILOGI(JS_NAPI, \
                "OnEvent: napi_call_function for %{public}d failed, status=%{public}d", eventType, status);
            return;
        }
    }

    napi_close_handle_scope(env_, scope);
    DEV_HILOGI(JS_NAPI, "Exit");
}