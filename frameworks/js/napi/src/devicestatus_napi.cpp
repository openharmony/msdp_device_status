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
static napi_ref g_responseConstructor;
static const uint8_t ARG_0 = 0;
static const uint8_t ARG_1 = 1;
static const uint8_t ARG_2 = 2;
static const int32_t CALLBACK_SUCCESS = 200;
static const int32_t ERROR_MESSAGE = -1;
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
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Callback enter");
    DevicestatusNapi* devicestatusNapi = DevicestatusNapi::GetDevicestatusNapi(devicestatusData.type);
    if (devicestatusNapi == nullptr) {
        DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "devicestatus is nullptr");
        return;
    }
    devicestatusNapi->OnDevicestatusChangedDone(static_cast<int32_t> (devicestatusData.type),
        static_cast<int32_t> (devicestatusData.value), false);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Callback exit");
}

DevicestatusNapi* DevicestatusNapi::GetDevicestatusNapi(int32_t type)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter, type = %{public}d", type);

    DevicestatusNapi* obj = nullptr;
    bool isExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isExists = true;
            obj = (DevicestatusNapi*)(it->second);
            DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Found object");
            break;
        }
    }
    if (!isExists) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Didn't find object");
    }
    return obj;
}

DevicestatusNapi::DevicestatusNapi(napi_env env, napi_value thisVar) : DevicestatusEvent(env, thisVar)
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

napi_value DevicestatusNapi::CreateInstanceForResponse(napi_env env, int32_t value)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value cons = nullptr;
    napi_value instance = nullptr;
    napi_status callBackStatus;
    ResponseEntity *entity = nullptr;

    callBackStatus = napi_get_reference_value(env, g_responseConstructor, &cons);
    if (callBackStatus != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "napi get reference value failed");
        return nullptr;
    }
    callBackStatus = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (callBackStatus != napi_ok || instance == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "napi new reference failed");
        return nullptr;
    }
    callBackStatus = napi_unwrap(env, instance, (void **)&entity);
    if (callBackStatus != napi_ok || entity == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "%{public}s: cannot unwrap entity from instance", __func__);
        return nullptr;
    }
    entity->value = DevicestatusDataUtils::DevicestatusValue(value);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");

    return instance;
}

void DevicestatusNapi::OnDevicestatusChangedDone(const int32_t& type, const int32_t& value, bool isOnce)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter, value = %{public}d", value);
    OnEvent(type, ARG_1, value, isOnce);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
}

void DevicestatusNapi::InvokeCallBack(napi_env env, napi_value *args, bool voidParameter, int32_t value)
{
    napi_value callback = nullptr;
    napi_value indexObj = nullptr;
    napi_create_object(env, &indexObj);
    napi_value successIndex = nullptr;
    if (!voidParameter) {
        napi_create_int32(env, value, &successIndex);
    }

    napi_ref callbackSuccess = nullptr;
    napi_value ret;
    napi_set_named_property(env, indexObj, "devicestatusValue", successIndex);
    napi_create_reference(env, args[ARG_1], 1, &callbackSuccess);
    napi_get_reference_value(env, callbackSuccess, &callback);
    napi_call_function(env, nullptr, callback, 1, &indexObj, &ret);
    napi_delete_reference(env, callbackSuccess);
}

napi_value DevicestatusNapi::SubscribeDevicestatus(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = {0};
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    napi_valuetype valueType2 = napi_undefined;
    napi_typeof(env, args[ARG_1], &valueType2);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "valueType2: %{public}d", valueType2);
    NAPI_ASSERT(env, valueType2 == napi_function, "type mismatch for parameter 2");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Failed to get type");
        return result;
    }

    if (type < 0 || type > DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        InvokeCallBack(env, args, false, ERROR_MESSAGE);
        return result;
    }

    DevicestatusNapi* obj = nullptr;
    bool isObjExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isObjExists = true;
            DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Object already exists");
            return result;
        }
    }
    if (!isObjExists) {
        DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Didn't find object, so created it");
        obj = new DevicestatusNapi(env, jsthis);
        napi_wrap(env, jsthis, reinterpret_cast<void *>(obj),
            [](napi_env env, void *data, void *hint) {
                (void)env;
                (void)hint;
                DevicestatusNapi *devicestatus = (DevicestatusNapi *)data;
                delete devicestatus;
            },
            nullptr, &(obj->callbackRef_));
        objectMap_.insert(std::pair<int32_t, DevicestatusNapi*>(type, obj));
    }

    if (!obj->On(type, args[ARG_1], false)) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "type: %{public}d already exists", type);
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
        DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Didn't find callback, so created it");
        callback = new DevicestatusCallback();
        g_DevicestatusClient.SubscribeCallback(DevicestatusDataUtils::DevicestatusType(type), callback);
        callbackMap_.insert(std::pair<int32_t, sptr<IdevicestatusCallback>>(type, callback));
        InvokeCallBack(env, args, false, CALLBACK_SUCCESS);
    } else {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Callback exists.");
        return result;
    }

    napi_get_undefined(env, &result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::UnSubscribeDevicestatus(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Failed to get type");
        return result;
    }

    if (type < 0 || type > DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        InvokeCallBack(env, args, true, ERROR_MESSAGE);
        return result;
    }

    DevicestatusNapi* obj = nullptr;
    bool isObjExists = false;
    for (auto it = objectMap_.begin(); it != objectMap_.end(); ++it) {
        if (it->first == type) {
            isObjExists = true;
            obj = (DevicestatusNapi*)(it->second);
            DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Found object");
        }
    }
    if (!isObjExists) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Didn't find object, so created it");
        return result;
    }

    if (!obj->Off(type, false)) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Failed to get callback for type: %{public}d", type);
        return result;
    } else {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "erase objectMap_");
        InvokeCallBack(env, args, true, CALLBACK_SUCCESS);
        objectMap_.erase(type);
    }

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
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "No existed callback");
        return result;
    } else if (callback != nullptr) {
        g_DevicestatusClient.UnSubscribeCallback(DevicestatusDataUtils::DevicestatusType(type), callback);
        callbackMap_.erase(type);
    }
    napi_get_undefined(env, &result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::GetDevicestatus(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value result = nullptr;
    size_t argc = ARG_2;
    napi_value args[ARG_2] = {0};
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 0), "Bad parameters");

    napi_valuetype valueType1 = napi_undefined;
    napi_typeof(env, args[ARG_0], &valueType1);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "valueType1: %{public}d", valueType1);
    NAPI_ASSERT(env, valueType1 == napi_number, "type mismatch for parameter 1");

    napi_valuetype valueType2 = napi_undefined;
    napi_typeof(env, args[ARG_1], &valueType2);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "valueType2: %{public}d", valueType2);
    NAPI_ASSERT(env, valueType2 == napi_function, "type mismatch for parameter 2");

    int32_t type;
    status = napi_get_value_int32(env, args[ARG_0], &type);
    if (status != napi_ok) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Failed to get type");
        return result;
    }

    DevicestatusNapi* obj = new DevicestatusNapi(env, jsthis);
    napi_wrap(env, jsthis, reinterpret_cast<void *>(obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DevicestatusNapi *devicestatus = (DevicestatusNapi *)data;
            delete devicestatus;
        },
        nullptr, &(obj->callbackRef_));

    if (!obj->On(type, args[ARG_1], true)) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "type: %{public}d already exists", type);
        return result;
    }

    DevicestatusDataUtils::DevicestatusData devicestatusData = \
        g_DevicestatusClient.GetDevicestatusData(DevicestatusDataUtils::DevicestatusType(type));

    obj->OnDevicestatusChangedDone(devicestatusData.type, devicestatusData.value, true);
    obj->Off(devicestatusData.type, true);

    napi_get_undefined(env, &result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return result;
}

napi_value DevicestatusNapi::EnumDevicestatusTypeConstructor(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return thisArg;
}

napi_value DevicestatusNapi::CreateEnumDevicestatusType(napi_env env, napi_value exports)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value highStill = nullptr;
    napi_value fineStill = nullptr;
    napi_value carBluetooth = nullptr;

    napi_create_int32(env, (int32_t)DevicestatusDataUtils::DevicestatusType::TYPE_HIGH_STILL, &highStill);
    napi_create_int32(env, (int32_t)DevicestatusDataUtils::DevicestatusType::TYPE_FINE_STILL, &fineStill);
    napi_create_int32(env, (int32_t)DevicestatusDataUtils::DevicestatusType::TYPE_CAR_BLUETOOTH, &carBluetooth);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("TYPE_HIGH_STILL", highStill),
        DECLARE_NAPI_STATIC_PROPERTY("TYPE_FINE_STILL", fineStill),
        DECLARE_NAPI_STATIC_PROPERTY("TYPE_CAR_BLUETOOTH", carBluetooth),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DevicestatusType", NAPI_AUTO_LENGTH, EnumDevicestatusTypeConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_set_named_property(env, exports, "DevicestatusType", result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return exports;
}

napi_value DevicestatusNapi::EnumDevicestatusValueConstructor(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return thisArg;
}

napi_value DevicestatusNapi::CreateDevicestatusValueType(napi_env env, napi_value exports)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value enter = nullptr;
    napi_value exit = nullptr;

    napi_create_int32(env, (int32_t)DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER, &enter);
    napi_create_int32(env, (int32_t)DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT, &exit);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("VALUE_ENTER", enter),
        DECLARE_NAPI_STATIC_PROPERTY("VALUE_EXIT", exit),
    };
    napi_value result = nullptr;
    napi_define_class(env, "DevicestatusValue", NAPI_AUTO_LENGTH, EnumDevicestatusValueConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_set_named_property(env, exports, "DevicestatusValue", result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return exports;
}

napi_value DevicestatusNapi::ResponseConstructor(napi_env env, napi_callback_info info)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto entity = new ResponseEntity();
    napi_wrap(
        env, thisVar, entity,
        [](napi_env env, void *data, void *hint) {
            DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Destructor");
            auto entity = (ResponseEntity*)data;
            delete entity;
        },
        nullptr, nullptr);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");

    return thisVar;
}

napi_status DevicestatusNapi::AddProperty(napi_env env, napi_value object, const std::string name, int32_t enumValue)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_status status;
    napi_value enumNapiValue;

    status = napi_create_int32(env, enumValue, &enumNapiValue);
    if (status == napi_ok) {
        status = napi_set_named_property(env, object, name.c_str(), enumNapiValue);
    }
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return status;
}

napi_value DevicestatusNapi::CreateDevicestatusValueObject(napi_env env)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_value result = nullptr;
    napi_status status;
    std::string propName;

    status = napi_create_object(env, &result);
    if (status == napi_ok) {
        for (unsigned int i = DevicestatusDataUtils::DevicestatusValue::VALUE_ENTER; \
            i < vecDevicestatusValue.size(); i++) {
            propName = vecDevicestatusValue[i];
            DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "propName: %{public}s", propName.c_str());
            status = AddProperty(env, result, propName, i);
            if (status != napi_ok) {
                DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "Failed to add named prop!");
                break;
            }
            propName.clear();
        }
    }
    if (status == napi_ok) {
        // The reference count is for creation of devicestatus value Object Reference
        status = napi_create_reference(env, result, 1, &devicestatusValueRef_);
        if (status == napi_ok) {
            return result;
        }
    }
    DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_JS_NAPI, "CreateDevicestatusValueObject is Failed!");
    napi_get_undefined(env, &result);

    return result;
}

napi_value DevicestatusNapi::CreateResponseClass(napi_env env, napi_value exports)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_PROPERTY("devicestatusValue", CreateDevicestatusValueObject(env)),
    };

    napi_value result = nullptr;
    napi_define_class(env, "Response", NAPI_AUTO_LENGTH, ResponseConstructor, nullptr,
        sizeof(desc) / sizeof(*desc), desc, &result);

    napi_create_reference(env, result, 1, &g_responseConstructor);
    napi_set_named_property(env, exports, "Response", result);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return exports;
}

napi_value DevicestatusNapi::Init(napi_env env, napi_value exports)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", SubscribeDevicestatus),
        DECLARE_NAPI_FUNCTION("off", UnSubscribeDevicestatus),
        DECLARE_NAPI_FUNCTION("once", GetDevicestatus),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    CreateEnumDevicestatusType(env, exports);
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");
    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value DevicestatusInit(napi_env env, napi_value exports)
{
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Enter");

    napi_value ret = DevicestatusNapi::Init(env, exports);

    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_JS_NAPI, "Exit");

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
