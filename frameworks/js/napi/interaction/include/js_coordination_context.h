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

#ifndef JS_COORDINATION_CONTEXT_H
#define JS_COORDINATION_CONTEXT_H

#include <memory>

#include "napi/native_node_api.h"
#include "js_coordination_manager.h"

namespace OHOS {
namespace Msdp {
class JsCoordinationContext final {
public:
    JsCoordinationContext();
    ~JsCoordinationContext();
    DISALLOW_COPY_AND_MOVE(JsCoordinationContext);

    static napi_value Export(napi_env env, napi_value exports);
    static napi_value Enable(napi_env env, napi_callback_info info);
    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value GetState(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);

    std::shared_ptr<JsCoordinationManager> GetJsInputDeviceCoordinationMgr();

private:
    std::shared_ptr<JsCoordinationManager> mgr_ { nullptr };
    std::mutex mutex_;
    napi_ref contextRef_ { nullptr };

    static napi_value CreateInstance(napi_env env);
    static napi_value JsConstructor(napi_env env, napi_callback_info info);
    static JsCoordinationContext *GetInstance(napi_env env);
    static void DeclareDeviceCoordinationInterface(napi_env env, napi_value exports);
    static void DeclareDeviceCoordinationData(napi_env env, napi_value exports);
    static napi_value EnumClassConstructor(napi_env env, napi_callback_info info);
};
} // namespace Msdp
} // namespace OHOS
#endif // JS_COORDINATION_CONTEXT_H
