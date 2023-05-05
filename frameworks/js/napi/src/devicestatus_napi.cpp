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

#include "devicestatus_napi.h"

#include <js_native_api.h>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "devicestatus_client.h"
#include "devicestatus_common.h"
#include "devicestatus_napi_error.h"
#include "stationary_manager.h"

using namespace OHOS;
using namespace OHOS::Msdp;
using namespace OHOS::Msdp::DeviceStatus;
namespace {
constexpr size_t ARG_0 = 0;
constexpr size_t ARG_1 = 1;
constexpr size_t ARG_2 = 2;
constexpr size_t ARG_3 = 3;
constexpr size_t ARG_4 = 4;
constexpr int32_t NAPI_BUF_LENGTH  = 256;
static const std::vector<std::string> vecDeviceStatusValue {
    "VALUE_ENTER", "VALUE_EXIT"
};
thread_local DeviceStatusNapi *g_obj = nullptr;
} // namespace
std::map<int32_t, sptr<IRemoteDevStaCallback>> DeviceStatusNapi::callbackMap_;
napi_ref DeviceStatusNapi::devicestatusValueRef_ = nullptr;

struct ResponseEntity {
    OnChangedValue value;
};

void DeviceStatusCallback::OnDeviceStatusChanged(const Data& devicestatusData)
{
    DEV_HILOGD(JS_NAPI, "OnDeviceStatusChanged enter");
    std::lock_guard<std::mutex> guard(mutex_);
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        DEV_HILOGE(JS_NAPI, "loop is nullptr");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        DEV_HILOGE(JS_NAPI, "work is nullptr");
        return;
    }
    DEV_HILOGD(JS_NAPI, "devicestatusData.type:%{public}d, devicestatusData.value:%{public}d",
        devicestatusData.type, devicestatusData.value);
    data_ = devicestatusData;
    work->data = static_cast<void *>(&data_);
    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, EmitOnEvent);
    if (ret != 0) {
        DEV_HILOGE(JS_NAPI, "Failed to execute work queue");
    }
}

void DeviceStatusCallback::EmitOnEvent(uv_work_t *work, int status)
{
    if (work == nullptr) {
        DEV_HILOGE(JS_NAPI, "work is nullptr");
        return;
    }
    Data* data = static_cast<Data*>(work->data);
    delete work;
    if (data == nullptr) {
        DEV_HILOGE(JS_NAPI, "work->data is nullptr");
        return;
    }
    DeviceStatusNapi* deviceStatusNapi = DeviceStatusNapi::GetDeviceStatusNapi();
    if (deviceStatusNapi == nullptr) {
        DEV_HILOGE(JS_NAPI, "deviceStatusNapi is nullptr");
        return;
    }

    int32_t type = static_cast<int32_t>(data->type);
    int32_t value = static_cast<int32_t>(data->value);
    DEV_HILOGD(JS_NAPI, "type:%{public}d, value:%{public}d", type, value);
    deviceStatusNapi->OnDeviceStatusChangedDone(type, value, false);
}

DeviceStatusNapi* DeviceStatusNapi::GetDeviceStatusNapi()
{
    DEV_HILOGD(JS_NAPI, "Enter");
    return g_obj;
}

DeviceStatusNapi::DeviceStatusNapi(napi_env env) : DeviceStatusEvent(env)
{
    env_ = env;
    callbackRef_ = nullptr;
    devicestatusValueRef_ = nullptr;
    DeviceStatusClient::GetInstance().RegisterDeathListener([this] {
        DEV_HILOGI(JS_NAPI, "Receive death notification");
        callbackMap_.clear();
        ClearEventMap();
    });
}

DeviceStatusNapi::~DeviceStatusNapi()
{
    if (callbackRef_ != nullptr) {
        napi_delete_reference(env_, callbackRef_);
    }
    if (devicestatusValueRef_ != nullptr) {
        napi_delete_reference(env_, devicestatusValueRef_);
    }
    if (g_obj != nullptr) {
        delete g_obj;
        g_obj = nullptr;
    }
}

void DeviceStatusNapi::OnDeviceStatusChangedDone(int32_t type, int32_t value, bool isOnce)
{
    DEV_HILOGD(JS_NAPI, "Enter, value:%{public}d", value);
    OnEvent(type, ARG_1, value, isOnce);
    DEV_HILOGD(JS_NAPI, "Exit");
}

int32_t DeviceStatusNapi::ConvertTypeToInt(const std::string &type)
{
    if (type == "absoluteStill") {
        return Type::TYPE_ABSOLUTE_STILL;
    } else if (type == "horizontalPosition") {
        return Type::TYPE_HORIZONTAL_POSITION;
    } else if (type == "verticalPosition") {
        return Type::TYPE_VERTICAL_POSITION;
    } else if (type == "still") {
        return Type::TYPE_STILL;
    } else if (type == "relativeStill") {
        return Type::TYPE_RELATIVE_STILL;
    } else if (type == "carBluetooth") {
        return Type::TYPE_CAR_BLUETOOTH;
    } else {
        return Type::TYPE_INVALID;
    }
}

bool DeviceStatusNapi::CheckArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_4] = {};
    size_t argc = ARG_4;
    napi_value args[ARG_4] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t i = 0; i < ARG_4; i++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[i], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get valueType");
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

bool DeviceStatusNapi::CheckUnsubArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_3] = {};
    size_t argc = ARG_3;
    napi_value args[ARG_3] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t i = 0; i < ARG_3; i++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[i], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get valueType");
            return false;
        }
        DEV_HILOGD(JS_NAPI, "valueType:%{public}d", valueType);
        arr[i] = valueType;
    }
    if (arr[ARG_0] != napi_string || arr[ARG_1] != napi_number || arr[ARG_2] != napi_function) {
        DEV_HILOGE(JS_NAPI, "Failed to get arguements");
        return false;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return true;
}

bool DeviceStatusNapi::CheckGetArguments(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    int arr[ARG_2] = {};
    size_t argc = ARG_2;
    napi_value args[ARG_2] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info");
        return false;
    }
    for (size_t i = 0; i < ARG_2; i++) {
        napi_valuetype valueType = napi_undefined;
        status = napi_typeof(env, args[i], &valueType);
        if (status != napi_ok) {
            DEV_HILOGE(JS_NAPI, "Failed to get valueType");
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

std::tuple<bool, napi_value, std::string, int32_t, int32_t> DeviceStatusNapi::CheckSubscribeParam(napi_env env,
    napi_callback_info info)
{
    std::tuple<bool, napi_value, std::string, int32_t, int32_t> result { false, nullptr, "", -1, -1 };
    size_t argc = ARG_4;
    napi_value args[ARG_4] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if ((status != napi_ok) || (argc < ARG_4)) {
        ThrowErr(env, PARAM_ERROR, "Bad parameters");
        return result;
    }
    if (!CheckArguments(env, info)) {
        ThrowErr(env, PARAM_ERROR, "Failed to get on arguments");
        return result;
    }
    size_t modLen = 0;
    status = napi_get_value_string_utf8(env, args[ARG_0], nullptr, 0, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get string item");
        return result;
    }
    char mode[NAPI_BUF_LENGTH] = {};
    status = napi_get_value_string_utf8(env, args[ARG_0], mode, modLen + 1, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get mode");
        return result;
    }
    int32_t eventMode = 0;
    status = napi_get_value_int32(env, args[ARG_1], &eventMode);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get event value item");
        return result;
    }
    int32_t latencyMode = 0;
    status = napi_get_value_int32(env, args[ARG_2], &latencyMode);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get latency value item");
        return result;
    }
    return std::make_tuple(true, args[ARG_3], std::string(mode), eventMode, latencyMode);
}

std::tuple<bool, napi_value, int32_t, int32_t> DeviceStatusNapi::CheckUnsubscribeParam(napi_env env,
    napi_callback_info info)
{
    std::tuple<bool, napi_value, int32_t, int32_t> result { false, nullptr, -1, -1 };
    size_t argc = ARG_3;
    napi_value args[ARG_3] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if ((status != napi_ok) || (argc < ARG_3)) {
        ThrowErr(env, PARAM_ERROR, "Bad parameters");
        return result;
    }
    if (!CheckUnsubArguments(env, info)) {
        ThrowErr(env, PARAM_ERROR, "Failed to get off arguments");
        return result;
    }
    size_t modLen = 0;
    status = napi_get_value_string_utf8(env, args[ARG_0], nullptr, 0, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get string item");
        return result;
    }
    char mode[NAPI_BUF_LENGTH] = {};
    status = napi_get_value_string_utf8(env, args[ARG_0], mode, modLen + 1, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get mode");
        return result;
    }
    int32_t type = DeviceStatusNapi::ConvertTypeToInt(mode);
    if ((type < Type::TYPE_ABSOLUTE_STILL) || (type > Type::TYPE_LID_OPEN)) {
        ThrowErr(env, PARAM_ERROR, "type is illegal");
        return result;
    }
    int32_t event = 0;
    status = napi_get_value_int32(env, args[ARG_1], &event);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get int32 item");
        return result;
    }
    if ((event < ActivityEvent::ENTER) || (event > ActivityEvent::ENTER_EXIT)) {
        ThrowErr(env, PARAM_ERROR, "event is illegal");
        return result;
    }
    DEV_HILOGD(JS_NAPI, "type: %{public}d, event: %{public}d", type, event);
    return std::make_tuple(true, args[ARG_2], type, event);
}

std::tuple<bool, napi_value, int32_t> DeviceStatusNapi::CheckGetParam(napi_env env, napi_callback_info info)
{
    std::tuple<bool, napi_value, int32_t> result { false, nullptr, -1 };
    size_t argc = ARG_2;
    napi_value args[ARG_2] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if ((status != napi_ok) || (argc < ARG_2)) {
        ThrowErr(env, PARAM_ERROR, "Bad parameters");
        return result;
    }
    if (!CheckGetArguments(env, info)) {
        ThrowErr(env, PARAM_ERROR, "Failed to get once arguments");
        return result;
    }
    size_t modLen = 0;
    status = napi_get_value_string_utf8(env, args[ARG_0], nullptr, 0, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get string item");
        return result;
    }
    char mode[NAPI_BUF_LENGTH] = {};
    status = napi_get_value_string_utf8(env, args[ARG_0], mode, modLen + 1, &modLen);
    if (status != napi_ok) {
        ThrowErr(env, PARAM_ERROR, "Failed to get mode");
        return result;
    }
    int32_t type = ConvertTypeToInt(mode);
    if ((type < Type::TYPE_ABSOLUTE_STILL) || (type > Type::TYPE_LID_OPEN)) {
        ThrowErr(env, PARAM_ERROR, "type is illegal");
        return result;
    }
    return std::make_tuple(true, args[ARG_1], type);
}

napi_value DeviceStatusNapi::SubscribeDeviceStatusCallback(napi_env env, napi_callback_info info, napi_value handler,
    int32_t type, int32_t event, int32_t latency)
{
    if (g_obj == nullptr) {
        g_obj = new (std::nothrow) DeviceStatusNapi(env);
        if (g_obj == nullptr) {
            DEV_HILOGE(JS_NAPI, "Failed to new g_obj");
            return nullptr;
        }
        DEV_HILOGD(JS_NAPI, "Didn't find object, so created it");
    }
    napi_wrap(env, nullptr, reinterpret_cast<void *>(g_obj),
        [](napi_env env, void *data, void *hint) {
            (void)env;
            (void)hint;
            DeviceStatusNapi *devicestatus = static_cast<DeviceStatusNapi *>(data);
            delete devicestatus;
        },
        nullptr, &(g_obj->callbackRef_));
    if (!g_obj->On(type, handler, false)) {
        DEV_HILOGE(JS_NAPI, "type:%{public}d already exists", type);
        return nullptr;
    }
    auto callbackIter = callbackMap_.find(type);
    if (callbackIter != callbackMap_.end()) {
        DEV_HILOGD(JS_NAPI, "Callback exists");
        return nullptr;
    }
    sptr<IRemoteDevStaCallback> callback = new (std::nothrow) DeviceStatusCallback(env);
    if (callback == nullptr) {
        DEV_HILOGE(JS_NAPI, "callback is nullptr");
        return nullptr;
    }
    auto subscribeRet = StationaryManager::GetInstance()->SubscribeCallback(Type(type),
        ActivityEvent(event), ReportLatencyNs(latency), callback);
    if (subscribeRet != RET_OK) {
        ThrowErr(env, SERVICE_EXCEPTION, "on: Failed to SubscribeCallback");
        return nullptr;
    }
    auto ret = callbackMap_.insert(std::pair<int32_t, sptr<IRemoteDevStaCallback>>(type, callback));
    if (!ret.second) {
        DEV_HILOGE(JS_NAPI, "Failed to insert");
        return nullptr;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return nullptr;
}

napi_value DeviceStatusNapi::SubscribeDeviceStatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    const auto [ret, handler, typeMode, event, latency] = CheckSubscribeParam(env, info);
    if (!ret) {
        DEV_HILOGE(JS_NAPI, "on: SubscribeDeviceStatus is failed");
        return nullptr;
    }
    int32_t type = ConvertTypeToInt(typeMode);
    DEV_HILOGD(JS_NAPI, "type:%{public}d, event:%{public}d, latency:%{public}d", type, event, latency);
    if ((type < Type::TYPE_ABSOLUTE_STILL) || (type > Type::TYPE_LID_OPEN)) {
        ThrowErr(env, PARAM_ERROR, "type is illegal");
        return nullptr;
    }
    if ((event < ActivityEvent::ENTER) || (event > ActivityEvent::ENTER_EXIT)) {
        ThrowErr(env, PARAM_ERROR, "event is illegal");
        return nullptr;
    }
    if ((latency < ReportLatencyNs::SHORT) || (latency > ReportLatencyNs::LONG)) {
        ThrowErr(env, PARAM_ERROR, "latency is illegal");
        return nullptr;
    }
    return SubscribeDeviceStatusCallback(env, info, handler, type, event, latency);
}

napi_value DeviceStatusNapi::UnsubscribeDeviceStatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    const auto [ret, handler, type, event] = CheckUnsubscribeParam(env, info);
    if (!ret) {
        DEV_HILOGE(JS_NAPI, "off: UnsubscribeDeviceStatus is failed");
        return nullptr;
    }
    if (g_obj == nullptr) {
        DEV_HILOGE(JS_NAPI, "g_obj is nullptr");
        return nullptr;
    }
    if (!g_obj->Off(type, handler)) {
        DEV_HILOGE(JS_NAPI, "Not ready to Unsubscribe for type:%{public}d", type);
        return nullptr;
    }
    auto callbackIter = callbackMap_.find(type);
    if (callbackIter != callbackMap_.end()) {
        auto unsubscribeRet = StationaryManager::GetInstance()->UnsubscribeCallback(Type(type),
            ActivityEvent(event), callbackIter->second);
        if (unsubscribeRet != RET_OK) {
            ThrowErr(env, SERVICE_EXCEPTION, "off: Failed to UnsubscribeCallback");
        }
        callbackMap_.erase(type);
    } else {
        NAPI_ASSERT(env, false, "No existed callback");
        return nullptr;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return nullptr;
}

napi_value DeviceStatusNapi::GetDeviceStatus(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    const auto [ret, handler, type] = CheckGetParam(env, info);
    if (!ret) {
        DEV_HILOGE(JS_NAPI, "once: GetDeviceStatus is failed");
        return nullptr;
    }
    if (g_obj == nullptr) {
        g_obj = new (std::nothrow) DeviceStatusNapi(env);
        if (g_obj == nullptr) {
            DEV_HILOGE(JS_NAPI, "Failed to new g_obj");
            return nullptr;
        }
        napi_wrap(env, nullptr, reinterpret_cast<void *>(g_obj),
            [](napi_env env, void *data, void *hint) {
                (void)env;
                (void)hint;
                DeviceStatusNapi *devicestatus = static_cast<DeviceStatusNapi *>(data);
                delete devicestatus;
            },
            nullptr, &(g_obj->callbackRef_));
    }
    if (!g_obj->On(type, handler, true)) {
        DEV_HILOGE(JS_NAPI, "type:%{public}d already exists", type);
        return nullptr;
    }
    Data devicestatusData = StationaryManager::GetInstance()->GetDeviceStatusData(Type(type));
    if (devicestatusData.type == Type::TYPE_INVALID) {
        ThrowErr(env, SERVICE_EXCEPTION, "once: Failed to GetDeviceStatusData");
    }
    g_obj->OnDeviceStatusChangedDone(devicestatusData.type, devicestatusData.value, true);
    g_obj->OffOnce(devicestatusData.type, handler);
    DEV_HILOGD(JS_NAPI, "Exit");
    return nullptr;
}

napi_value DeviceStatusNapi::EnumActivityEventConstructor(napi_env env, napi_callback_info info)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_cb_info item");
        return nullptr;
    }
    napi_value global = nullptr;
    status = napi_get_global(env, &global);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to get_global item");
        return nullptr;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return thisArg;
}

napi_value DeviceStatusNapi::DeclareEventTypeInterface(napi_env env, napi_value exports)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value enter = nullptr;
    napi_status status = napi_create_int32(env, static_cast<int32_t>(ActivityEvent::ENTER), &enter);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to create ENTER item");
        return nullptr;
    }
    napi_value exit = nullptr;
    status = napi_create_int32(env, static_cast<int32_t>(ActivityEvent::EXIT), &exit);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to create EXIT item");
        return nullptr;
    }
    napi_value enter_exit = nullptr;
    status = napi_create_int32(env, static_cast<int32_t>(ActivityEvent::ENTER_EXIT), &enter_exit);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to create ENTER_EXIT item");
        return nullptr;
    }
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("ENTER", enter),
        DECLARE_NAPI_STATIC_PROPERTY("EXIT", exit),
        DECLARE_NAPI_STATIC_PROPERTY("ENTER_EXIT", enter_exit),
    };
    napi_value result = nullptr;
    status = napi_define_class(env, "ActivityEvent", NAPI_AUTO_LENGTH,
        EnumActivityEventConstructor, nullptr, sizeof(desc) / sizeof(*desc), desc, &result);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to define_class item");
        return nullptr;
    }
    status = napi_set_named_property(env, exports, "ActivityEvent", result);
    if (status != napi_ok) {
        DEV_HILOGE(JS_NAPI, "Failed to set_named_property item");
        return nullptr;
    }
    DEV_HILOGD(JS_NAPI, "Exit");
    return exports;
}

napi_value DeviceStatusNapi::Init(napi_env env, napi_value exports)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", SubscribeDeviceStatus),
        DECLARE_NAPI_FUNCTION("off", UnsubscribeDeviceStatus),
        DECLARE_NAPI_FUNCTION("once", GetDeviceStatus),
    };
    DeclareEventTypeInterface(env, exports);
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    DEV_HILOGD(JS_NAPI, "Exit");
    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value DeviceStatusInit(napi_env env, napi_value exports)
{
    DEV_HILOGD(JS_NAPI, "Enter");
    napi_value ret = DeviceStatusNapi::Init(env, exports);
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
    .nm_filename = "stationary",
    .nm_register_func = DeviceStatusInit,
    .nm_modname = "stationary",
    .nm_priv = (static_cast<void *>(0)),
    .reserved = {0}
};

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
