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

#ifndef DEVICE_STATUS_NAPI_H
#define DEVICE_STATUS_NAPI_H

#include <map>
#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "devicestatus_data_utils.h"
#include "devicestatus_callback_stub.h"
#include "devicestatus_event.h"

namespace OHOS {
namespace Msdp {
class DeviceStatusCallback : public DevicestatusCallbackStub {
public:
    explicit DeviceStatusCallback(napi_env env) : env_(env) {}
    virtual ~DeviceStatusCallback() {};
    void OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData &devicestatusData) override;
    static void EmitOnEvent(uv_work_t *work, int status);
private:
    napi_env env_ = { nullptr };
    std::mutex mutex_;
    DevicestatusDataUtils::DevicestatusData data_;
};

class DeviceStatusNapi : public DeviceStatusEvent {
public:
    explicit DeviceStatusNapi(napi_env env);
    virtual ~DeviceStatusNapi();

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value SubscribeDeviceStatus(napi_env env, napi_callback_info info);
    static napi_value SubscribeDeviceStatusCallback(napi_env env, napi_callback_info info, napi_value *args,
        int32_t type, int32_t event, int32_t latency);
    static napi_value UnsubscribeDeviceStatus(napi_env env, napi_callback_info info);
    static napi_value GetDeviceStatus(napi_env env, napi_callback_info info);

    static int32_t ConvertTypeToInt(const std::string &type);
    void OnDeviceStatusChangedDone(int32_t type, int32_t value, bool isOnce);
    static DeviceStatusNapi* GetDeviceStatusNapi();
    static std::map<int32_t, sptr<IdevicestatusCallback>> callbackMap_;

private:
    static bool CheckArguments(napi_env env, napi_callback_info info);
    static bool CheckUnsubArguments(napi_env env, napi_callback_info info);
    static bool CheckGetArguments(napi_env env, napi_callback_info info);
    napi_ref callbackRef_ = { nullptr };
    static napi_ref devicestatusValueRef_;
    napi_env env_ = { nullptr };
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_NAPI_H
