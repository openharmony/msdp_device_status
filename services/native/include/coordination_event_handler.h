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
#ifndef COORDINATION_EVENT_HANDLER_H
#define COORDINATION_EVENT_HANDLER_H

#include <string>

#include "uds_server.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationEventHandler final {
public:
    CoordinationEventHandler() = default;
    ~CoordinationEventHandler() = default;

    void Init(UDSServer& udsServer);
    int32_t OnRegisterCoordinationListener(int32_t pid);
    int32_t OnUnregisterCoordinationListener(int32_t pid);
    int32_t OnEnableInputDeviceCoordination(int32_t pid, int32_t userData, bool enabled);
    int32_t OnStartInputDeviceCoordination(int32_t pid, int32_t userData, const std::string &sinkDeviceId,
        int32_t srcInputDeviceId);
    int32_t OnStopInputDeviceCoordination(int32_t pid, int32_t userData);
    int32_t OnGetInputDeviceCoordinationState(int32_t pid, int32_t userData, const std::string &deviceId);
private:
    UDSServer *udsServer_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_EVENT_HANDLER_H
