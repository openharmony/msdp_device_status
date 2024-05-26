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

#include "parcel.h"

#include "devicestatus_define.h"
#include "raw_data_packer.h"
#include "softbus_message_id.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

InputDeviceMgr::InputDeviceMgr(IContext *context) : env_(context) { }

void InputDeviceMgr::OnSessionOpened(const DSoftbusSessionOpened &notice)
{
    CALL_INFO_TRACE;
    if (env_->GetDeviceManager().HasKeyboard()) {
        auto keyboards = env_->GetDeviceManager().GetKeyboard();
        NotifyInputDeviceToRemote(notice.networkId, keyboards);
        return;
    }
    FI_HILOGI("No keyboard existed in current device");
}

void InputDeviceMgr::OnSessionClosed(const DSoftbusSessionClosed &notice)
{
    CALL_INFO_TRACE;
    RemoveAllInputDevice(notice.networkId);
}

void InputDeviceMgr::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    sender_ = sender;
}

bool InputDeviceMgr::OnRawData(const std::string &networkId, const void *data, uint32_t dataLen)
{
    return false; 
}

bool InputDeviceMgr::OnParcel(const std::string &networkId, SoftbusMessageId messageId, Parcel &parcel)
{
    return false;
}

bool InputDeviceMgr::OnPacket(const std::string &networkId, Msdp::NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    switch (packet.GetMsgId()) {
        case MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG: {
            OnRemoteHotPlugInfo(networkId, packet);
            break;
        }
        case MessageId::DSOFTBUS_INPUT_DEV_SYNC: {
            OnRemoteInputDeviceInfo(networkId, packet);
            break;
        }
        default: {
            FI_HILOGW("Unexpected message(%{public}d) from \'%{public}s\'",
                static_cast<int32_t>(packet.GetMsgId()), Utility::Anonymize(networkId).c_str());
            return false;
        }
    }
    return true;
}

void InputDeviceMgr::OnRemoteInputDeviceInfo(const std::string &networkId, Msdp::NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    int32_t devNum { 0 };
    packet >> devNum;
    if (devNum <= 0 || devNum >= 100) {
        FI_HILOGW("Invalid devNum:%{public}d", devNum);
        return;
    }
    for (int32_t i = 0; i < devNum; i++) {
        KeyDeviceInfo devInfo;
        packet >> devInfo.deviceId >> devInfo.dhid >> devInfo.name >> devInfo.networkId >>
            devInfo.unique >> devInfo.isPointerDevice >> devInfo.isKeyboard;
        AddRemoteInputDevice(networkId, devInfo);
    }
}

void InputDeviceMgr::OnRemoteHotPlugInfo(const std::string &networkId, Msdp::NetPacket &packet)
{
    CALL_INFO_TRACE;
    KeyDeviceInfo devInfo;
    InputHotplugType hotPlugType;
    packet >> hotPlugType >> devInfo.deviceId >> devInfo.dhid >> devInfo.name >> devInfo.networkId >>
        devInfo.unique >> devInfo.isPointerDevice >> devInfo.isKeyboard;
    if (hotPlugType == InputHotplugType::PLUG) {
        AddRemoteInputDevice(networkId, devInfo);
    }
    if (hotPlugType == InputHotplugType::UNPLUG) {
        RemoveRemoteInputDevice(networkId, devInfo);
    }
}

void InputDeviceMgr::NotifyInputDeviceToRemote(const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    if (!env_->GetDeviceManager().HasKeyboard()) {
        FI_HILOGE("Local device have no keyboard, skip");
        return;
    }
    auto keyboards = env_->GetDeviceManager().GetKeyboard();
    NetPacket packet(MessageId::DSOFTBUS_INPUT_DEV_SYNC);
    packet << keyboards.size();
    for (const auto &keyboard : keyboards) {
        auto keyDeviceInfo = keyboard->GetKeyDeviceInfo();
        packet << keyDeviceInfo.deviceId << keyDeviceInfo.dhid << keyDeviceInfo.name << keyDeviceInfo.networkId <<
            keyDeviceInfo.unique << keyDeviceInfo.isPointerDevice << keyDeviceInfo.isKeyboard;
    }
    if (env_->GetDsoftbus().SendPacket(remoteNetworkId, packet) != RET_OK) {
        FI_HILOGE("SendPacket failed");
    }
}

void InputDeviceMgr::NotifyHotPlugToRemote(const std::string &remoteNetworkId, const InputHotplugEvent &notice)
{
    CALL_INFO_TRACE;
    auto device = env_->GetDeviceManager().GetDevice(notice.deviceId);
    auto keyDeviceInfo = device->GetKeyDeviceInfo();
    NetPacket packet(MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG);
    packet << notice.type << keyDeviceInfo.deviceId << keyDeviceInfo.dhid << keyDeviceInfo.name << keyDeviceInfo.networkId <<
        keyDeviceInfo.unique << keyDeviceInfo.isPointerDevice << keyDeviceInfo.isKeyboard;
    if (env_->GetDsoftbus().SendPacket(remoteNetworkId, packet) != RET_OK) {
        FI_HILOGE("SendPacket failed");
    }
}

void InputDeviceMgr::RemoveRemoteInputDevice(const std::string &networkId, const KeyDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
    if (remoteDeviceInfo_.find(networkId) == remoteDeviceInfo_.end()) {
        FI_HILOGE("Current device not existed");
        return;
    }
    remoteDeviceInfo_[networkId].erase(deviceInfo);
}

void InputDeviceMgr::AddRemoteInputDevice(const std::string &networkId, const KeyDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
    remoteDeviceInfo_[networkId].insert(deviceInfo);
}

void InputDeviceMgr::RemoveAllInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    if (remoteDeviceInfo_.find(networkId) == remoteDeviceInfo_.end()) {
        FI_HILOGE("No device exists");
        return;
    }
    remoteDeviceInfo_.erase(networkId);
}

void InputDeviceMgr::DumpInputDevice(const std::string &networkId)
{
    if (remoteDeviceInfo_.find(networkId) == remoteDeviceInfo_.end()) {
        FI_HILOGE("No input deviceInfo of networkId:%{public}s", Utility::Anonymize(networkId).c_str());
    }
    for (const auto &elem : remoteDeviceInfo_[networkId]) {
        FI_HILOGI("Device info of each device");
    }
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
