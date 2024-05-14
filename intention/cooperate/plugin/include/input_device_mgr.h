/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef COOPERATE_INPUT_DEVICE_MANAGER_H
#define COOPERATE_INPUT_DEVICE_MANAGER_H

#include <memory>

#include "device.h"
#include "nocopyable.h"

#include "cooperate_events.h"
#include "i_context.h"
#include "net_packet.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class InputDeviceMgr {
public:
    InputDeviceMgr(IContext *context) : context_(context) {}
    ~InputDeviceMgr() = default;
    DISALLOW_COPY_AND_MOVE(InputDeviceMgr);
    void NotifyInputDevice(const DSoftbusNotifyDeviceInfo &notice);
    void OnNotifyInputDevice(const DSoftbusNotifyDeviceInfo &notice);

private:
    static bool SerializeDeviceInfo(const std::string &deviceInfo, NetPacket &packet);
    static bool DeSerializeDeviceInfo(NetPacket &packet, const std::string &deviceInfo);
    int32_t InputDeviceMgr::SendPacket(const std::string &remoteNetworkId, NetPacket &packet);

private:
    IContext *context_ { nullptr };
};

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_INPUT_DEVICE_MANAGER_H
