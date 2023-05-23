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

#ifndef JS_COOPERATE_MANAGER_H
#define JS_COOPERATE_MANAGER_H

#include <mutex>
#include <string>

#include "js_event_cooperate_target.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class JsCooperateManager : public JsEventCooperateTarget {
public:
    JsCooperateManager() = default;
    DISALLOW_COPY_AND_MOVE(JsCooperateManager);
    ~JsCooperateManager() = default;

    napi_value Enable(napi_env env, bool enable, napi_value handle = nullptr);
    napi_value Start(napi_env env, const std::string &remoteNetworkDescriptor, int32_t startDeviceId,
        napi_value handle = nullptr);
    napi_value Stop(napi_env env, napi_value handle = nullptr);
    napi_value GetState(napi_env env, const std::string &deviceDescriptor, napi_value handle = nullptr);
    void RegisterListener(napi_env env, const std::string &type, napi_value handle);
    void UnregisterListener(napi_env env, const std::string &type, napi_value handle = nullptr);
    void ResetEnv();

private:
    std::mutex mutex_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // JS_COOPERATE_MANAGER_H
