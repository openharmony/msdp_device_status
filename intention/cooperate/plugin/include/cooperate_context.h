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

#ifndef COOPERATE_CONTEXT_H
#define COOPERATE_CONTEXT_H

#include <memory>

#include "cooperate_events.h"
#include "device_manager.h"
#include "dinput_adapter.h"
#include "event_manager.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
struct Context {
    IContext *env_ { nullptr };
    std::string remoteNetworkId_;
    int32_t startDeviceId_;
    bool isUnchain_ { false };
    Channel<CooperateEvent>::Sender sender_;

    std::shared_ptr<DeviceManager> devMgr_;
    std::shared_ptr<DInputAdapter> dinput_;
    EventManager evMgr_;

    Context(IContext *env);
    void Enable();
    void Disable();
    int32_t EnableDevMgr();
    void DisableDevMgr();
    int32_t EnableDSoftbus();
    void DisableDSoftbus();
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_CONTEXT_H