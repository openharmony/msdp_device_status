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

#ifndef I_INPUTCONTEXT_H
#define I_INPUTCONTEXT_H

#include <functional>
#include <string>

#include "devicestatus_define.h"
#include "display_info.h"
#include "key_event.h"
#include "pointer_event.h"
#include "stub.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

struct MouseLocation {
    int32_t physicalX { 0 };
    int32_t physicalY { 0 };
};
enum class CooperationMessage {
    OPEN_SUCCESS = 100,
    OPEN_FAIL = 101,
    INFO_START = 200,
    INFO_SUCCESS = 201,
    INFO_FAIL = 202,
    CLOSE = 300,
    CLOSE_SUCCESS = 301,
    STOP = 400,
    STOP_SUCCESS = 401,
    STOP_FAIL = 402,
    STATE_ON = 500,
    STATE_OFF = 501,
    INPUT_DEVICE_ID_ERROR = 4400001,
    COOPERATE_FAIL = 4400002,
    COOPERATION_DEVICE_ERROR = 4400003,
};

class IInputContext {
public:
    virtual int32_t AddTimer(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback) = 0;
    virtual int32_t RemoveTimer(int32_t timerId) = 0;
    virtual void SelectAutoRepeat(std::shared_ptr<MMI::KeyEvent>& keyEvent) = 0;
    virtual void SetJumpInterceptState(bool isJump) = 0;
    virtual std::vector<std::string> GetCooperateDhids(int32_t deviceId) = 0;
    virtual std::vector<std::string> GetCooperateDhids(const std::string &dhid) = 0;
    virtual std::string GetOriginNetworkId(int32_t id) = 0;
    virtual std::string GetOriginNetworkId(const std::string &dhid) = 0;
    virtual bool IsRemote(struct libinput_device *inputDevice) = 0;
    virtual bool IsRemote(int32_t id) = 0;
    virtual int32_t SetPointerVisible(int32_t pid, bool visible) = 0;
    virtual MouseLocation GetMouseInfo() = 0;
    virtual const MMI::DisplayGroupInfo& GetDisplayGroupInfo() = 0;
    virtual std::shared_ptr<MMI::PointerEvent> GetPointerEvent() = 0;
    virtual bool HasLocalPointerDevice() = 0;
    virtual void SetAbsolutionLocation(double xPercent, double yPercent) = 0;
    virtual std::string GetDhid(int32_t deviceId) = 0;
    virtual int32_t FindInputDeviceId(struct libinput_device* inputDevice) = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_INPUTCONTEXT_H