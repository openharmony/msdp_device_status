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

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusEvent" };
constexpr size_t EVENT_MAP_MAX { 20 };
constexpr size_t EVENT_LIST_MAX { 30 };
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
    std::lock_guard<std::mutex> guard(mutex_);
    if ((eventMap_.size() > EVENT_MAP_MAX) || (eventOnceMap_.size() > EVENT_MAP_MAX)) {
        FI_HILOGE("eventMap_ or eventOnceMap_ size over");
        return false;
    }
    if (eventMap_[eventType].size() > EVENT_LIST_MAX || eventOnceMap_[eventType].size() > EVENT_LIST_MAX) {
        FI_HILOGE("list size over");
        return false;
    }
    if (isOnce) {
        if (!SaveCallbackByEvent(eventType, handler, isOnce, eventOnceMap_)) {
            FI_HILOGE("Failed to save eventOnceMap_ callback");
            return false;
        }
    } else {
        if (!SaveCallbackByEvent(eventType, handler, isOnce, eventMap_)) {
            FI_HILOGE("Failed to save eventMap_ callback");
            return false;
        }
    }
    return true;
}

bool DeviceStatusEvent::SaveCallbackByEvent(int32_t eventType, napi_value handler, bool isOnce,
    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>> eventMap_)
{
    CALL_DEBUG_ENTER;
    napi_ref onHandlerRef = nullptr;
    napi_status status = napi_create_reference(env_, handler, 1, &onHandlerRef);
    if (status != napi_ok) {
        FI_HILOGE("Failed to napi_create_reference");
        return false;
    }
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        FI_HILOGE("eventType:%{public}d not exists", eventType);
        eventMap_[eventType] = std::list<std::shared_ptr<DeviceStatusEventListener>>();
    }
    if (eventMap_[eventType].empty()) {
        FI_HILOGE("eventMap_ save callback");
        SaveCallback(eventType, onHandlerRef, isOnce);
        return true;
    }
    if (!IsNoExistCallback(eventMap_[eventType], handler, eventType)) {
        FI_HILOGE("callback already exists");
        return false;
    }
    SaveCallback(eventType, onHandlerRef, isOnce);
    return true;
}

bool DeviceStatusEvent::IsNoExistCallback(std::list<std::shared_ptr<DeviceStatusEventListener>>,
    napi_value handler, int32_t eventType)
{
    CALL_DEBUG_ENTER;
    napi_value result = nullptr;
    bool equal = false;
    for (const auto& item : eventMap_[eventType]) {
        napi_status status = napi_get_reference_value(env_, item->onHandlerRef, &result);
        if (status != napi_ok) {
            FI_HILOGE("Failed to napi_get_reference_value");
            return false;
        }
        status = napi_strict_equals(env_, result, handler, &equal);
        if (status != napi_ok) {
            FI_HILOGE("Failed to napi_strict_equals");
            return false;
        }
        if (equal) {
            FI_HILOGE("Map callback is exist");
            return false;
        }
    }
    return true;
}

void DeviceStatusEvent::SaveCallback(int32_t eventType, napi_ref onHandlerRef, bool isOnce)
{
    auto listener = std::make_shared<DeviceStatusEventListener>();
    listener->onHandlerRef = onHandlerRef;
    if (isOnce) {
        eventOnceMap_[eventType].push_back(listener);
    } else {
        eventMap_[eventType].push_back(listener);
    }
    FI_HILOGD("Add handler to list %{public}d", eventType);
}

bool DeviceStatusEvent::Off(int32_t eventType, napi_value handler)
{
    FI_HILOGD("DeviceStatusEvent off in for event:%{public}d", eventType);
    std::lock_guard<std::mutex> guard(mutex_);
    auto iter = eventMap_.find(eventType);
    if (iter == eventMap_.end()) {
        FI_HILOGE("eventType %{public}d not found", eventType);
        return false;
    }
    bool equal = false;
    napi_value result = nullptr;

    for (auto listener : eventMap_[eventType]) {
        napi_status status = napi_get_reference_value(env_, listener->onHandlerRef, &result);
        if (status != napi_ok) {
            FI_HILOGE("Failed to napi_get_reference_value");
            return false;
        }
        status = napi_strict_equals(env_, result, handler, &equal);
        if (status != napi_ok) {
            FI_HILOGE("Failed to napi_strict_equals");
            return false;
        }
        if (equal) {
            FI_HILOGI("Delete handler from list %{public}d", eventType);
            status = napi_delete_reference(env_, listener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Failed to napi_delete_reference");
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
        FI_HILOGE("eventType %{public}d not found", eventType);
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
                FI_HILOGW("Failed to napi_delete_reference");
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
    napi_value handler = nullptr;
    napi_status status = napi_ok;
    status = napi_get_reference_value(env_, typeHandler->onHandlerRef, &handler);
    if (status != napi_ok) {
        FI_HILOGE("OnEvent handler for %{public}d failed, status:%{public}d", eventType, status);
        return;
    }
    napi_value result = nullptr;
    SendRet(eventType, value, result);
    napi_value callResult = nullptr;
    FI_HILOGD("Report to hap");
    status = napi_call_function(env_, nullptr, handler, argc, &result, &callResult);
    if (status != napi_ok) {
        FI_HILOGE("CheckRet:napi_call_function for %{public}d failed, status:%{public}d", eventType, status);
        return;
    }
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
    FI_HILOGD("OnEvent for %{public}d, isOnce:%{public}d", eventType, isOnce);;
    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>>::iterator typeHandler;
    if (isOnce) {
        typeHandler = eventOnceMap_.find(eventType);
        if (typeHandler == eventOnceMap_.end()) {
            FI_HILOGE("OnEvent eventType %{public}d not found", eventType);
            return;
        }
    } else {
        typeHandler = eventMap_.find(eventType);
        if (typeHandler == eventMap_.end()) {
            FI_HILOGE("OnEvent:eventType %{public}d not found", eventType);
            return;
        }
    }
    FI_HILOGD("%{public}zu callbacks of eventType %{public}d are sent",
        typeHandler->second.size(), eventType);
    for (auto handler : typeHandler->second) {
        CheckRet(eventType, argc, value, handler);
    }
}

void DeviceStatusEvent::ClearEventMap()
{
    for (const auto &iter : eventMap_) {
        for (const auto &eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Failed to napi_delete_reference");
            }
        }
    }
    for (const auto &iter : eventOnceMap_) {
        for (const auto &eventListener : iter.second) {
            napi_status status = napi_delete_reference(env_, eventListener->onHandlerRef);
            if (status != napi_ok) {
                FI_HILOGW("Failed to napi_delete_reference");
                napi_delete_reference(env_, eventListener->onHandlerRef);
            }
        }
    }
    eventMap_.clear();
    eventOnceMap_.clear();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
