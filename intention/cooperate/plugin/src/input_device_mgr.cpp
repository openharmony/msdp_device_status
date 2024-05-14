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

#include "input_device_mgr.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
InputDeviceMgr::NotifyInputDevice(const DSoftbusNotifyDeviceInfo &notice)
{
    CALL_INFO_TRACE;
    NetPacket packet;
    if (!SerializeDeviceInfo(notice.deviceInfo, packet)) {
        FI_HILOGE(SerializeDeviceInfo failed);
    }
    if (SendPacket(notice.remoteNetworkId, packet) != RET_OK) {
        FI_HILOGE("SendPacket failed");
        return RET_ERR;
    }
    return RET_OK;
}

void InputDeviceMgr::OnNotifyInputDevice(const DSoftbusNotifyDeviceInfo &notice)
{
    std::shared_ptr<MMI::InputDevice> deviceInfo;
    context_->GetInput().NotifyVirtualDeviceInfo(deviceInfo, );

}

bool InputDeviceMgr::SerializeDeviceInfo(const std::string &deviceInfo, NetPacket &packet)
{
    CHKPR(device, false);
    return true;
}

bool InputDeviceMgr::DeSerializeDeviceInfo(NetPacket &packet, const std::string &deviceInfo)
{
    CHKPR(device, false);
    return true;
}

int32_t InputDeviceMgr::SendPacket(const std::string &remoteNetworkId, NetPacket &packet)
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    if (context_->GetDSoftbus().OpenSession(remoteNetworkId) != RET_OK) {
        FI_HILOGE("Failed to connect to %{public}s", Utility::Anonymize(remoteNetworkId).c_str());
        return RET_ERR;
    }
    if (context_->GetDSoftbus().SendPacket(remoteNetworkId, packet) != RET_OK) {
        FI_HILOGE("SendPacket failed to %{public}s", Utility::Anonymize(remoteNetworkId).c_str());
        return RET_ERR;
    }
    return RET_OK;
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_INPUT_DEVICE_MANAGER_H
