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

#include "devicestatus_napi.h"

#include "devicestatus_common.h"
#include "devicestatus_client.h"

using namespace OHOS::Msdp;
using namespace OHOS;
namespace {
auto &g_DevicestatusClient = DevicestatusClient::GetInstance();
static constexpr uint8_t ARG_0 = 0;
static constexpr uint8_t ARG_1 = 1;
static constexpr uint8_t ARG_2 = 2;
static constexpr uint8_t ARG_3 = 3;
static constexpr uint8_t ARG_4 = 4;
static const std::vector<std::string> vecDevicestatusValue {
    "VALUE_ENTER", "VALUE_EXIT"
};
}
std::map<int32_t, sptr<IdevicestatusCallback>> DevicestatusNapi::callbackMap_;
std::map<int32_t, DevicestatusNapi*> DevicestatusNapi::objectMap_;
napi_ref DevicestatusNapi::devicestatusValueRef_;

struct ResponseEntity {
    DevicestatusDataUtils::DevicestatusValue value;
};

void DevicestatusCallback::OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    DEV_HILOGD(JS_NAPI, "Callback enter");
    DevicestatusNapi* devicestatusNapi = DevicestatusNapi::GetDevicestatusNapi(devicestatusData.type);
    if (devicestatusNapi == nullptr) {
        DEV_HILOGD(JS_NAPI, "devicestatus is nullptr");
        return;
    }
    static DevicestatusDataUtils::DevicestatusData statusData;
    statusData = devicestatusData;
    work->data = &statusData;
    int32_t ret = uv_queue_work(loop, work, [](uv_work_t* work){}, [](uv_work_t* work, int32_t status){
        DevicestatusDataUtils::DevicestatusData* deviceStatusData =
            static_cast<DevicestatusDataUtils::DevicestatusData*>(work->data);
        DevicestatusNapi* deviceStatusNapi = DevicestatusNapi::GetDevicestatusNapi(deviceStatusData->type);
        if (deviceStatusNapi == nullptr) {
            DEV_HILOGD(JS_NAPI, "device status napi is nullptr");
            delete work;
            return;
        }
        deviceStatusNapi->OnDevicestatusChangedDone(static_cast<int32_t>(deviceStatusData->type),
            static_cast<int32_t>(deviceStatusData->value),false);
        delete work;
    });
    if (ret != 0) {
        DEV_HILOGE(JS_NAPI, "Failed to execute work queue");
        delete work;
    }
    DEV_HILOGD(JS_NAPI, "Callback exit");
}

DevicestatusNapi* DevicestatusNapi::GetDevicestatusNapi(int32_t type)
{
    DEV_HILOGD(JS_NAPI, "Enter, type = %{public}d", type);

    DevicestatusNapi* obj = nullptr;
    bool isExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isExists = true;
            obj = (DevicestatusNapi*)(it->second);
            DEV_HILOGD(JS_NAPI, "Found object");
            break;
        }
    }
    if (!isExists) {
        DEV_HILOGE(JS_NAPI, "Didn't find object");
    }
    return obj;
}

DevicestatusNapi::DevicestatusNapi(napi_env env) : DeviceStatusEvent(env)
{
    env_ = env;
    callbackRef_ = nullptr;
    devicestatusValueRef_ = nullptr;
}

DevicestatusNapi::~DevicestatusNapi()
{
    if (callbackRef_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }

    if (devicestatusValueRef_ != nullptr) {
        napi_delete_reference(env_, devicestatusValueRef_);
    }
}

void DevicestatusNapi::OnDevicestatusChangedDone(const int32_t& type, const int32_t& value, bool isOnce)
{
    DEV_HILOGD(JS_NAPI, "Enter, value: %{public}d", value);
    OnEvent(type, ARG_1, value, isOnce);
    DEV_HILOGD(JS_NAPI, "Exit");
}

int32_t DevicestatusNapi::ConvertTypeToInt(const std::string &type)
{
    if (type == "highStill") {
        return DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL;
    } else if (type == "fineStill") {
        return DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL;
    } else if (type == "carBluetooht") {
        return DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH;
    } else {
        return DevicestatusDataUtils::DevicestatusType::TYPE_INVALID;
    }
}
bool DevicestatusNapi::CheckArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_4] = {};
    size_t argc = ARG_4;
    napi_value args[ARG_4] = {};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t i = 0; i < ARG_4; i++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[i], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get arguments");
            return false;
        }
        DEV_HILOGD(JS_NAPI, "valueType:%{public}d", valueType);
        arr[i] = valueType;
    }
    if (arr[ARG_0] != napi_string || arr[ARG_1] != napi_number || arr[ARG_2] != napi_number ||
        arr[ARG_3] != napi_function) {
        DEV_HILOGE(JS_NAPI, "Failed to get arguements");
        return false;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return true;
}

bool DevicestatusNapi::CheckUnsubArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_3] = {};
    size_t argc = ARG_3;
    napi_value args[ARG_3] = {};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t arg = 0; arg < ARG_3; arg++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[arg], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get arguments");
            return false;
        }
        DEV_HILOGD(JS_NAPI, "valueType:%{public}d", valueType);
        arr[arg] = valueType;
    }
    if (arr[ARG_0] != napi_string || arr[ARG_1] != napi_number || arr[ARG_2] != napi_function) {
        DEV_HILOGE(JS_NAPI, "Failed to get arguements");
        return false;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return true;
}

bool DevicestatusNapi::CheckGetArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_2] = {};
    size_t argc = ARG_2;
    napi_value args[ARG_2] = {};
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t i = 0; i < ARG_2; i++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[i], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get arguments");
            return false;
        }
        DEV_HILOGD(JS_NAPI, "valueType:%{public}d", valueType);
        arr[i] = valueType;
    }
    if (arr[ARG_0] != napi_string || arr[ARG_1] != napi_function) {
        DEV_HILOGE(JS_NAPI, "Failed to get arguements");
        return false;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return true;
}

napi_value DevicestatusNapi::SubscribeDevicestatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = {0};
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, status == napi_ok, "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEV_HILOGD(JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    napi_valuetype valueType2 = napi_undefined;
    napi_typeof(env, args[ARG_1], &valueType2);
    DEV_HILOGD(JS_NAPI, "valueType2: %{public}d", valueType2);
    NAPI_ASSERT(env, valueType2 == napi_function, "type mismatch for parameter 2");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get type");
        return result;
    }

    if (type < 0 || type > DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        return result;
    }

    DevicestatusNapi* obj = nullptr;
    bool isObjExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isObjExists = true;
            DEV_HILOGE(JS_NAPI, "Object already exists");
            return result;
        }
    }
    if (!isObjExists) {
        DEV_HILOGD(JS_NAPI, "Didn't find object, so created it");
        obj = new (std::nothrow) DevicestatusNapi(env);
        if (obj == nullptr) {
            DEV_HILOGE(JS_NAPI, "obj is nullptr");
            return result;
        }
        napi_wrap(env, nullptr, reinterpret_cast<void *>(obj),
            [](napi_env env, void *data, void *hint) {
                (void)env;
                (void)hint;
                DevicestatusNapi *devicestatus = (DevicestatusNapi *)data;
                delete devicestatus;
            },
            nullptr, &(obj->callbackRef_));
        objectMap_.insert(std::pair<int32_t, DevicestatusNapi*>(type, obj));
    }

    if (obj == nullptr) {
        DEV_HILOGE(JS_NAPI, "obj is nullptr");
        return result;
    }
    if (!obj->On(type, args[ARG_1], false)) {
        DEV_HILOGE(JS_NAPI, "type: %{public}d already exists", type);
        return result;
    }

    sptr<IdevicestatusCallback> callback;
    bool isCallbackExists = false;
    for (auto it = callbackMap_.begin(); it != callbackMap_.end(); ++it) {
        if (it->first == type) {
            isCallbackExists = true;
            break;
        }
    }
    if (!isCallbackExists) {
        DEV_HILOGD(JS_NAPI, "Didn't find callback, so created it");
        callback = new DevicestatusCallback();
        g_DevicestatusClient.SubscribeCallback(DevicestatusDataUtils::DevicestatusType(type), callback);
        callbackMap_.insert(std::pair<int32_t, sptr<IdevicestatusCallback>>(type, callback));
        InvokeCallBack(env, args, false, CALLBACK_SUCCESS);
    } else {
        DEV_HILOGE(JS_NAPI, "Callback exists.");
        return result;
    }
    DEV_HILOGD(JS_NAPI, "Didn't find callback, so created it");
    sptr<IdevicestatusCallback> callback = new (std::nothrow) DevicestatusCallback(env);
    if (callback == nullptr) {
        DEV_HILOGE(JS_NAPI, "Callback is nullptr.");
        return result;
    }
    g_DevicestatusClient.SubscribeCallback(DevicestatusDataUtils::DevicestatusType(type), callback);
    callbackMap_.insert(std::pair<int32_t, sptr<IdevicestatusCallback>>(type, callback));

    napi_get_undefined(env, &result);
    DEV_HILOGD(JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::UnSubscribeDevicestatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, status == napi_ok, "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEV_HILOGD(JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get type");
        return result;
    }

    if (type < 0 || type > DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        return result;
    }

    DevicestatusNapi* obj = nullptr;
    bool isObjExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isObjExists = true;
            obj = (DevicestatusNapi*)(it->second);
            DEV_HILOGD(JS_NAPI, "Found object");
        }
    }
    if (!isObjExists) {
        DEV_HILOGE(JS_NAPI, "Didn't find object, so created it");
        return result;
    }

    if (obj == nullptr) {
        DEV_HILOGE(JS_NAPI, "obj is nullptr");
        return result;
    }
    if (!obj->Off(type, args[ARG_1])) {
        DEV_HILOGE(JS_NAPI, "Failed to get callback for type: %{public}d", type);
        return result;
    } else {
        DEV_HILOGE(JS_NAPI, "erase objectMap_");
        InvokeCallBack(env, args, true, CALLBACK_SUCCESS);
        objectMap_.erase(type);
    }
    DEV_HILOGW(JS_NAPI, "erase objectMap_");
    objectMap_.erase(type);

    sptr<IdevicestatusCallback> callback;
    bool isCallbackExists = false;
    for (auto it = callbackMap_.begin(); it != callbackMap_.end(); ++it) {
        if (it->first == type) {
            isCallbackExists = true;
            callback = (sptr<IdevicestatusCallback>)(it->second);
            break;
        }
    }
    if (!isCallbackExists) {
        DEV_HILOGE(JS_NAPI, "No existed callback");
        return result;
    } else if (callback != nullptr) {
        g_DevicestatusClient.UnSubscribeCallback(DevicestatusDataUtils::DevicestatusType(type), callback);
        callbackMap_.erase(type);
    }
    napi_get_undefined(env, &result);
    DEV_HILOGD(JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::GetDevicestatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = {0};
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, status == napi_ok, "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEV_HILOGD(JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    napi_valuetype valueType2 = napi_undefined;
    napi_typeof(env, args[ARG_1], &valueType2);
    DEV_HILOGD(JS_NAPI, "valueType2: %{public}d", valueType2);
    NAPI_ASSERT(env, valueType2 == napi_function, "type mismatch for parameter 2");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get type");
        return result;
    }

    DevicestatusNapi* obj = new (std::nothrow) DevicestatusNapi(env);
    if (obj == nullptr) {
        DEV_HILOGE(JS_NAPI, "obj is nullptr");
        return result;
    }
    napi_wrap(env, nullptr, reinterpret_cast<void *>(obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DevicestatusNapi *devicestatus = (DevicestatusNapi *)data;
            delete devicestatus;
        },
        nullptr, &(obj->callbackRef_));

    if (!obj->On(type, args[ARG_1], true)) {
        DEV_HILOGE(JS_NAPI, "type: %{public}d already exists", type);
        return result;
    }

    DevicestatusDataUtils::DevicestatusData devicestatusData = \
        g_DevicestatusClient.GetDevicestatusData(DevicestatusDataUtils::DevicestatusType(type));

    obj->OnDevicestatusChangedDone(devicestatusData.type, devicestatusData.value, true);
    obj->OffOnce(devicestatusData.type, args[ARG_1]);

    napi_get_undefined(env, &result);
    DEV_HILOGD(JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::Init(napi_env env, napi_value exports)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", SubscribeDevicestatus),
        DECLARE_NAPI_FUNCTION("off", UnSubscribeDevicestatus),
        DECLARE_NAPI_FUNCTION("once", GetDevicestatus),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    DEV_HILOGD(JS_NAPI, "Exit");
    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value DevicestatusInit(napi_env env, napi_value exports)
{
    DEV_HILOGD(JS_NAPI, "Enter");

    napi_value ret = DevicestatusNapi::Init(env, exports);

    DEV_HILOGD(JS_NAPI, "Exit");

    return ret;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "devicestatus",
    .nm_register_func = DevicestatusInit,
    .nm_modname = "devicestatus",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
