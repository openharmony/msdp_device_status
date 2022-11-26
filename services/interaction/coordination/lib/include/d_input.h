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

#ifndef D_INPUT_H
#define D_INPUT_H

#include "i_d_input.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DInput : public IDInput {
public:
    DInput();
    virtual ~DInput();

    /* 分布式输入消息（设备上下线，穿越变更）处理，多模可以去掉，新部件需要添加 */
    void Init(DelegateTasksCallback delegateTasksCallback);
    /* 处理鼠标按下时穿越，对端发送模拟事件，新部件添加 */
    void RegisterEventCallback(SimulateEventCallback callback);
    /* 键鼠穿越的开关 */
    void EnableInputDeviceCooperate(bool enabled);
    /* */
    int32_t OnStartInputDeviceCooperate(SessionPtr sess, int32_t userData,
        const std::string& sinkDeviceId, int32_t srcInputDeviceId);
    /* */
    int32_t OnStopDeviceCooperate(SessionPtr sess, int32_t userData);
    /* */
    int32_t OnGetInputDeviceCooperateState(SessionPtr sess, int32_t userData,
        const std::string& deviceId);
    /* */
    int32_t OnRegisterCooperateListener(SessionPtr sess);
    /* */
    int32_t OnUnregisterCooperateListener(SessionPtr sess);

    void OnKeyboardOnline(const std::string& dhid);
    void OnPointerOffline(const std::string& dhid, const std::string& sinkNetworkId,
        const std::vector<std::string>& keyboards);
    bool HandleEvent(libinput_event* event);
    bool CheckKeyboardWhiteList(std::shared_ptr<MMI::KeyEvent> keyEvent);
    bool IsNeedFilterOut(const std::string& deviceId, const std::shared_ptr<MMI::KeyEvent> keyEvent);
    std::string GetLocalDeviceId();

    void Dump(int32_t fd, const std::vector<std::string>& args);

private:

};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // D_INPUT_H


