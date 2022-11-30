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

#ifndef DEVICE_STATUS_EVENT_H
#define DEVICE_STATUS_EVENT_H

#include <list>
#include <map>
#include <memory>
#include <string>

#include "napi/native_api.h"
#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
struct DeviceStatusEventListener {
    napi_ref onHandlerRef;
};

class DeviceStatusEvent {
public:
    explicit DeviceStatusEvent(napi_env env);
    DeviceStatusEvent() = default;
    virtual ~DeviceStatusEvent();

    virtual bool On(int32_t eventType, napi_value handler, bool isOnce);
    virtual bool Off(int32_t eventType, napi_value handler);
    virtual bool OffOnce(int32_t eventType, napi_value handler);
    virtual void OnEvent(int32_t eventType, size_t argc, int32_t value, bool isOnce);
    void CheckRet(int32_t eventType, size_t argc, int32_t value,
        std::shared_ptr<DeviceStatusEventListener> &typeHandler);
    void SendRet(int32_t eventType, int32_t value, napi_value &result);
    void ClearEventMap();
protected:
    napi_env env_;
    napi_ref thisVarRef_ { nullptr };
    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>> eventMap_;
    std::map<int32_t, std::list<std::shared_ptr<DeviceStatusEventListener>>> eventOnceMap_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_EVENT_H
