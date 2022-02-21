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

#ifndef DEVICESTATUS_EVENT_H
#define DEVICESTATUS_EVENT_H

#include <string>
#include <list>
#include <memory>

#include <map>
#include "napi/native_api.h"

namespace OHOS {
namespace Msdp {
struct DevicestatusEventListener {
    int32_t eventType;
    napi_ref handlerRef = nullptr;
};

class DevicestatusEvent {
public:
    DevicestatusEvent(napi_env env, napi_value thisVar);
    DevicestatusEvent() {};
    virtual ~DevicestatusEvent();

    virtual bool On(const int32_t& eventType, napi_value handler, bool isOnce);
    virtual bool Off(const int32_t& eventType, bool isOnce);
    virtual void OnEvent(const int32_t& eventType, size_t argc, const int32_t& value, bool isOnce);

protected:
    napi_env env_;
    napi_ref thisVarRef_;
    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>> eventMap_;
    std::map<int32_t, std::shared_ptr<DevicestatusEventListener>> eventOnceMap_;
};

class JsResponse {
public:
    int32_t devicestatusValue_ = -1;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_EVENT_H
