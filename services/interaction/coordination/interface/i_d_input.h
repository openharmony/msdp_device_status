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

#ifndef I_D_INPUT_H
#define I_D_INPUT_H

#include <functional>
#include <memory>
#include <vector>

#include "i_context.h"
#include "key_event.h"
#include "uds_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDInput {
public:
    using SimulateEventCallback = std::function<void(uint32_t type, uint32_t code, int32_t value)>;
    virtual void Init() = 0;
    virtual void RegisterEventCallback(SimulateEventCallback callback) = 0;
    virtual void EnableInputDeviceCoordination(bool enabled) = 0;
    virtual int32_t OnStartInputDeviceCoordination(SessionPtr sess, int32_t userData,
        const std::string &sinkDeviceId, int32_t srcInputDeviceId) = 0;
    virtual int32_t OnStopDeviceCoordination(SessionPtr sess, int32_t userData) = 0;
    virtual int32_t OnGetInputDeviceCoordinationState(SessionPtr sess, int32_t userData,
        const std::string& deviceId) = 0;
    virtual int32_t OnRegisterCoordinationListener(SessionPtr sess) = 0;
    virtual int32_t OnUnregisterCoordinationListener(SessionPtr sess) = 0;

    virtual void OnKeyboardOnline(const std::string& dhid) = 0;
    virtual void OnPointerOffline(const std::string& dhid, const std::string& sinkNetworkId,
        const std::vector<std::string>& keyboards) = 0;
    virtual bool HandleEvent(libinput_event* event) = 0;
    virtual bool CheckKeyboardWhiteList(std::shared_ptr<MMI::KeyEvent> keyEvent) = 0;
    virtual std::string GetLocalDeviceId() = 0;

    virtual void Dump(int32_t fd, const std::vector<std::string>& args) = 0;
};

extern "C" IDInput* CreateIDInpt(IContext* context);
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_D_INPUT_H