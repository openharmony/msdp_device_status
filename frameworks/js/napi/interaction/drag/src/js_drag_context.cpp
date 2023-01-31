/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "js_drag_context.h"

#include "devicestatus_define.h"
#include "drag_message.h"
#include "napi_constants.h"
#include "util_napi_error.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "JsDragContext" };
const char* DRAG_CLASS = "drag_class";
const char* DRAG = "drag";
} // namespace

JsDragContext::JsDragContext()
    : mgr_(std::make_shared<JsDragManager>()) {}

JsDragContext::~JsDragContext()
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto jsDragMgr = mgr_;
    mgr_.reset();
    if (jsDragMgr != nullptr) {
        jsDragMgr->ResetEnv();
    }
}

std::shared_ptr<JsDragManager> JsDragContext::GetJsDragMgr()
{
    std::lock_guard<std::mutex> guard(mutex_);
    return mgr_;
}

napi_value JsDragContext::CreateInstance(napi_env env)
{
    CALL_INFO_TRACE;
    napi_value global = nullptr;
    CHKRP(napi_get_global(env, &global), GET_GLOBAL);

    constexpr char className[] = "JsDragContext";
    napi_value jsClass = nullptr;
    napi_property_descriptor desc[] = {};
    napi_status status = napi_define_class(env, className, sizeof(className),
        JsDragContext::JsConstructor, nullptr, sizeof(desc) / sizeof(desc[0]), nullptr, &jsClass);
    CHKRP(status, DEFINE_CLASS);

    status = napi_set_named_property(env, global, DRAG_CLASS, jsClass);
    CHKRP(status, SET_NAMED_PROPERTY);

    napi_value jsInstance = nullptr;
    CHKRP(napi_new_instance(env, jsClass, 0, nullptr, &jsInstance), NEW_INSTANCE);
    CHKRP(napi_set_named_property(env, global, DRAG, jsInstance),
        SET_NAMED_PROPERTY);

    JsDragContext *jsContext = nullptr;
    CHKRP(napi_unwrap(env, jsInstance, (void**)&jsContext), UNWRAP);
    CHKPP(jsContext);
    CHKRP(napi_create_reference(env, jsInstance, 1, &(jsContext->contextRef_)), CREATE_REFERENCE);

    uint32_t refCount = 0;
    status = napi_reference_ref(env, jsContext->contextRef_, &refCount);
    if (status != napi_ok) {
        FI_HILOGE("ref is nullptr");
        napi_delete_reference(env, jsContext->contextRef_);
        return nullptr;
    }
    return jsInstance;
}

napi_value JsDragContext::JsConstructor(napi_env env, napi_callback_info info)
{
    CALL_INFO_TRACE;
    napi_value thisVar = nullptr;
    void *data = nullptr;
    CHKRP(napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data), GET_CB_INFO);

    JsDragContext *jsContext = new (std::nothrow) JsDragContext();
    CHKPP(jsContext);
    napi_status status = napi_wrap(env, thisVar, jsContext, [](napi_env env, void *data, void *hin) {
        FI_HILOGI("jsvm ends");
        JsDragContext *context = static_cast<JsDragContext*>(data);
        delete context;
    }, nullptr, nullptr);
    if (status != napi_ok) {
        delete jsContext;
        FI_HILOGE("%{public}s failed", std::string(WRAP).c_str());
        auto infoTemp = std::string(__FUNCTION__) + ": " + std::string(WRAP) + " failed";
        napi_throw_error(env, nullptr, infoTemp.c_str());
        return nullptr;
    }
    return thisVar;
}

JsDragContext *JsDragContext::GetInstance(napi_env env)
{
    CALL_INFO_TRACE;
    napi_value global = nullptr;
    CHKRP(napi_get_global(env, &global), GET_GLOBAL);

    bool result = false;
    CHKRP(napi_has_named_property(env, global, DRAG, &result), HAS_NAMED_PROPERTY);
    if (!result) {
        FI_HILOGE("Drag was not found");
        return nullptr;
    }

    napi_value object = nullptr;
    CHKRP(napi_get_named_property(env, global, DRAG, &object), GET_NAMED_PROPERTY);
    if (object == nullptr) {
        FI_HILOGE("object is nullptr");
        return nullptr;
    }

    JsDragContext *instance = nullptr;
    CHKRP(napi_unwrap(env, object, (void**)&instance), UNWRAP);
    if (instance == nullptr) {
        FI_HILOGE("instance is nullptr");
        return nullptr;
    }
    return instance;
}

napi_value JsDragContext::On(napi_env env, napi_callback_info info)
{
    CALL_INFO_TRACE;
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    CHKRP(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr), GET_CB_INFO);
    if (argc == 0) {
        THROWERR_CUSTOM(env, COMMON_PARAMETER_ERROR, "Wrong number of parameters");
        return nullptr;
    }
    JsDragContext *jsDev = JsDragContext::GetInstance(env);
    CHKPP(jsDev);
    auto jsDragMgr = jsDev->GetJsDragMgr();
    CHKPP(jsDragMgr);
    if (!UtilNapi::TypeOf(env, argv[0], napi_function)) {
        THROWERR(env, COMMON_PARAMETER_ERROR, "callback", "function");
        return nullptr;
    }
    jsDragMgr->RegisterListener(env, STATE_TYPE, argv[0]);
    return nullptr;
}

napi_value JsDragContext::Off(napi_env env, napi_callback_info info)
{
    CALL_INFO_TRACE;
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    CHKRP(napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr), GET_CB_INFO);

    JsDragContext *jsDev = JsDragContext::GetInstance(env);
    CHKPP(jsDev);
    auto jsDragMgr = jsDev->GetJsDragMgr();
    CHKPP(jsDragMgr);
    if (argc == 0) {
        jsDragMgr->UnregisterListener(env, STATE_TYPE);
        return nullptr;
    }
    if (!UtilNapi::TypeOf(env, argv[0], napi_function)) {
        THROWERR(env, COMMON_PARAMETER_ERROR, "callback", "function");
        return nullptr;
    }
    jsDragMgr->UnregisterListener(env, STATE_TYPE, argv[0]);
    return nullptr;
}

void JsDragContext::DeclareDragData(napi_env env, napi_value exports)
{
    napi_value startMsg = nullptr;
    CHKRV(napi_create_int32(env, static_cast<int32_t>(DragMessage::MSG_DRAG_STATE_START), &startMsg),
        CREATE_INT32);
    napi_value stopMsg = nullptr;
    CHKRV(napi_create_int32(env, static_cast<int32_t>(DragMessage::MSG_DRAG_STATE_STOP), &stopMsg),
        CREATE_INT32);
    napi_value errorMsg = nullptr;
    CHKRV(napi_create_int32(env, static_cast<int32_t>(DragMessage::MSG_DRAG_STATE_ERROR), &errorMsg),
        CREATE_INT32);
    napi_value cancelMsg = nullptr;
    CHKRV(napi_create_int32(env, static_cast<int32_t>(DragMessage::MSG_DRAG_STATE_CANCEL), &cancelMsg),
        CREATE_INT32);

    napi_property_descriptor msg[] = {
        DECLARE_NAPI_STATIC_PROPERTY("MSG_DRAG_STATE_START", startMsg),
        DECLARE_NAPI_STATIC_PROPERTY("MSG_DRAG_STATE_STOP", stopMsg),
        DECLARE_NAPI_STATIC_PROPERTY("MSG_DRAG_STATE_ERROR", errorMsg),
        DECLARE_NAPI_STATIC_PROPERTY("MSG_DRAG_STATE_CANCEL", cancelMsg),
    };

    napi_value eventMsg = nullptr;
    CHKRV(napi_define_class(env, "NotifyMsg", NAPI_AUTO_LENGTH, EnumClassConstructor, nullptr,
        sizeof(msg) / sizeof(*msg), msg, &eventMsg), DEFINE_CLASS);
    CHKRV(napi_set_named_property(env, exports, "NotifyMsg", eventMsg), SET_NAMED_PROPERTY);
}

void JsDragContext::DeclareDragInterface(napi_env env, napi_value exports)
{
    napi_property_descriptor functions[] = {
        DECLARE_NAPI_STATIC_FUNCTION("on", On),
        DECLARE_NAPI_STATIC_FUNCTION("off", Off),
    };
    CHKRV(napi_define_properties(env, exports,
        sizeof(functions) / sizeof(*functions), functions), DEFINE_PROPERTIES);
}

napi_value JsDragContext::EnumClassConstructor(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value args[1] = { nullptr };
    napi_value result = nullptr;
    void *data = nullptr;
    CHKRP(napi_get_cb_info(env, info, &argc, args, &result, &data), GET_CB_INFO);
    return result;
}

napi_value JsDragContext::Export(napi_env env, napi_value exports)
{
    CALL_INFO_TRACE;
    auto instance = CreateInstance(env);
    CHKPP(instance);
    DeclareDragData(env, exports);
    DeclareDragInterface(env, exports);
    return exports;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
