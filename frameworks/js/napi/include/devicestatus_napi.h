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

#ifndef DEVICESTATUS_NAPI_H
#define DEVICESTATUS_NAPI_H

#include <map>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "devicestatus_callback_stub.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_event.h"

namespace OHOS {
namespace Msdp {
class DevicestatusCallback : public DevicestatusCallbackStub {
public:
    explicit DevicestatusCallback() {};
    virtual ~DevicestatusCallback() {};
    void OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData& devicestatusData) override;
};

class DevicestatusNapi : public DevicestatusEvent {
public:
    explicit DevicestatusNapi(napi_env env, napi_value thisVar);
    virtual ~DevicestatusNapi();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value SubscribeDevicestatus(napi_env env, napi_callback_info info);
    static napi_value UnSubscribeDevicestatus(napi_env env, napi_callback_info info);
    static napi_value GetDevicestatus(napi_env env, napi_callback_info info);
    static napi_value EnumDevicestatusTypeConstructor(napi_env env, napi_callback_info info);
    static napi_value CreateEnumDevicestatusType(napi_env env, napi_value exports);
    static napi_value EnumDevicestatusValueConstructor(napi_env env, napi_callback_info info);
    static napi_value CreateDevicestatusValueType(napi_env env, napi_value exports);
    static napi_value ResponseConstructor(napi_env env, napi_callback_info info);
    static napi_status AddProperty(napi_env env, napi_value object, const std::string name, int32_t enumValue);
    static napi_value CreateDevicestatusValueObject(napi_env env);
    static napi_value CreateResponseClass(napi_env env, napi_value exports);
    static napi_value CreateInstanceForResponse(napi_env env, int32_t value);
    static void RegisterCallback(const int32_t& eventType);
    static void InvokeCallBack(napi_env env, napi_value *args, bool voidParameter, int32_t value);
    void OnDevicestatusChangedDone(const int32_t& type, const int32_t& value, bool isOnce);
    static DevicestatusNapi* GetDevicestatusNapi(int32_t type);
    static std::map<int32_t, sptr<IdevicestatusCallback>> callbackMap_;
    static std::map<int32_t, DevicestatusNapi*> objectMap_;

private:
    napi_ref callbackRef_;
    static napi_ref devicestatusValueRef_;
    napi_env env_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_NAPI_H
