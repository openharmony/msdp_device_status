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

#include "device.h"
#include "devicestatus_define.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "InputDeviceMgr"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
constexpr int32_t MAX_INPUT_DEV_NUM { 100 };
constexpr size_t MAX_INPUT_DEV_PER_DEVICE { 10 };
constexpr int32_t INVALID_DEVICE_ID { -1 };

InputDeviceMgr::InputDeviceMgr(IContext *context) : env_(context)
{
    observer_ = std::make_shared<DSoftbusObserver>(*this);
}

void InputDeviceMgr::Enable(Channel<CooperateEvent>::Sender sender)
{
    CALL_INFO_TRACE;
    if (enable_) {
        return;
    }
    sender_ = sender;
    env_->GetDSoftbus().AddObserver(observer_);
    FI_HILOGI("Enable InputDeviceMgr");
    enable_ = true;
}

void InputDeviceMgr::Disable()
{
    CALL_INFO_TRACE;
    if (enable_) {
        enable_ = false;
        env_->GetDSoftbus().RemoveObserver(observer_);
    }
    FI_HILOGI("Disable InputDeviceMgr");
}

void InputDeviceMgr::OnSoftbusSessionOpened(const DSoftbusSessionOpened &notice)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    NotifyInputDeviceToRemote(notice.networkId);
}

void InputDeviceMgr::OnSoftbusSessionClosed(const DSoftbusSessionClosed &notice)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    RemoveAllRemoteInputDevice(notice.networkId);
}

void InputDeviceMgr::OnLocalHotPlug(const InputHotplugEvent &notice)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    BroadcastHotPlugToRemote(notice);
}

bool InputDeviceMgr::OnRawData(const std::string &networkId, const void *data, uint32_t dataLen)
{
    return false;
}

bool InputDeviceMgr::OnPacket(const std::string &networkId, NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    switch (auto messageId = packet.GetMsgId(); messageId) {
        case MessageId::DSOFTBUS_INPUT_DEV_SYNC: {
            OnRemoteInputDevice(networkId, packet);
            break;
        }
        case MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG: {
            OnRemoteHotPlug(networkId, packet);
            break;
        }
        default: {
            FI_HILOGD("Unexpected messageId:%{public}d", static_cast<int32_t>(messageId));
            return false;
        }
    }
    return true;
}

void InputDeviceMgr::OnRemoteInputDevice(const std::string &networkId, NetPacket &packet)
{
    CALL_INFO_TRACE;
    int32_t devNum { 0 };
    packet >> devNum;
    FI_HILOGI("devNum:%{public}d", devNum);
    if (devNum <= 0 || devNum >= MAX_INPUT_DEV_NUM) {
        FI_HILOGE("Invalid devNum:%{public}d", devNum);
        return;
    }
    auto device = std::make_shared<Device>(INVALID_DEVICE_ID);
    for (int32_t i = 0; i < devNum; i++) {
        if (DeserializeDevice(device, packet) != RET_OK) {
            FI_HILOGE("DeserializeDevice failed");
            return;
        }
        DispDeviceInfo(device);
        AddRemoteInputDevice(networkId, device);
    }
}

void InputDeviceMgr::OnRemoteHotPlug(const std::string &networkId, NetPacket &packet)
{
    CALL_INFO_TRACE;
    int32_t type { -1 };
    packet >> type;
    FI_HILOGI("Hot plug type:%{public}d", type);
    InputHotplugType hotPlugType = static_cast<InputHotplugType>(type);
    auto device = std::make_shared<Device>(INVALID_DEVICE_ID);
    int32_t deviceId { -1 };
    if (hotPlugType == InputHotplugType::PLUG) {
        if (DeserializeDevice(device, packet) != RET_OK) {
            FI_HILOGE("DeserializeDevice failed");
            return;
        }
        AddRemoteInputDevice(networkId, device);
        deviceId = device->GetId();
    }
    if (hotPlugType == InputHotplugType::UNPLUG) {
        packet >> deviceId;
        device->SetId(deviceId);
        RemoveRemoteInputDevice(networkId, device);
    }
    auto ret = sender_.Send(CooperateEvent(
        CooperateEventType::REMOTE_HOTPLUG_EVENT,
        RemoteHotPlugEvent {
            .networkId = networkId,
            .remoteDeviceId = deviceId,
            .type = hotPlugType,
        }));
    if (ret != Channel<CooperateEvent>::NO_ERROR) {
        FI_HILOGE("Failed to send event via channel, error:%{public}d", ret);
    }
}

void InputDeviceMgr::AddVirtualInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    FI_HILOGI("Add virtual device from %{public}s", Utility::Anonymize(networkId).c_str());
    for (const auto &device : remoteDevices_[networkId]) {
        CHKPC(device);
        AddVirtualInputDevice(networkId, device->GetId());
    }
}

void InputDeviceMgr::RemoveVirtualInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    FI_HILOGI("Remove virtual device from %{public}s", Utility::Anonymize(networkId).c_str());
    for (const auto &device : remoteDevices_[networkId]) {
        CHKPC(device);
        RemoveVirtualInputDevice(networkId, device->GetId());
    }
}

void InputDeviceMgr::HandleRemoteHotPlug(const RemoteHotPlugEvent &notice)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (notice.type == InputHotplugType::UNPLUG) {
        if (remote2VirtualIds_.find(notice.remoteDeviceId) == remote2VirtualIds_.end()) {
            FI_HILOGI("No virtual matches remote deviceId:%{public}d", notice.remoteDeviceId);
            return;
        }
        RemoveVirtualInputDevice(notice.networkId, notice.remoteDeviceId);
    }
    if (notice.type == InputHotplugType::PLUG) {
        AddVirtualInputDevice(notice.networkId, notice.remoteDeviceId);
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
    int32_t keyboardNum = keyboards.size();
    packet << keyboardNum;
    for (const auto &keyboard : keyboards) {
        if (SerializeDevice(keyboard, packet) != RET_OK) {
            FI_HILOGE("SerializeDevice failed");
            return;
        }
        DispDeviceInfo(keyboard);
    }
    if (int32_t ret = env_->GetDSoftbus().SendPacket(remoteNetworkId, packet); ret != RET_OK) {
        FI_HILOGE("SenPacket to networkId:%{public}s failed, ret:%{public}d",
            Utility::Anonymize(remoteNetworkId).c_str(), ret);
        return;
    }
    FI_HILOGI("NotifyInputDeviceToRemote networkId:%{public}s", Utility::Anonymize(remoteNetworkId).c_str());
}

void InputDeviceMgr::BroadcastHotPlugToRemote(const InputHotplugEvent &notice)
{
    CALL_INFO_TRACE;
    FI_HILOGI("HotplugType%{public}d deviceId:%{public}d", static_cast<int32_t>(notice.type), notice.deviceId);
    if (!notice.isKeyboard) {
        FI_HILOGI("Not keyboard, skip");
        return;
    }
    NetPacket packet(MessageId::DSOFTBUS_INPUT_DEV_HOT_PLUG);
    packet << static_cast<int32_t>(notice.type);
    if (notice.type == InputHotplugType::PLUG) {
        auto device = env_->GetDeviceManager().GetDevice(notice.deviceId);
        CHKPV(device);
        DispDeviceInfo(device);
        if (SerializeDevice(device, packet) != RET_OK) {
            FI_HILOGE("SerializeDevice failed");
            return;
        }
    }
    if (notice.type == InputHotplugType::UNPLUG) {
        packet << notice.deviceId;
        if (packet.ChkRWError()) {
            FI_HILOGE("Write packet failed");
            return;
        }
    }
    if (int32_t ret = env_->GetDSoftbus().BroadcastPacket(packet); ret != RET_OK) {
        FI_HILOGE("BroadcastPacket failed");
        return;
    }
}

void InputDeviceMgr::RemoveRemoteInputDevice(const std::string &networkId, std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    if (remoteDevices_.find(networkId) == remoteDevices_.end()) {
        FI_HILOGE("NetworkId:%{public}s have no device existed", Utility::Anonymize(networkId).c_str());
        return;
    }
    DispDeviceInfo(device);
    remoteDevices_[networkId].erase(device);
}

void InputDeviceMgr::AddRemoteInputDevice(const std::string &networkId, std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    DispDeviceInfo(device);
    if (remoteDevices_.find(networkId) != remoteDevices_.end() &&
        remoteDevices_[networkId].size() >= MAX_INPUT_DEV_PER_DEVICE) {
        FI_HILOGE("Input device num from networkId:%{public}s exceeds limit", Utility::Anonymize(networkId).c_str());
        return;
    }
    remoteDevices_[networkId].insert(device);
}

void InputDeviceMgr::RemoveAllRemoteInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    if (remoteDevices_.find(networkId) == remoteDevices_.end()) {
        FI_HILOGE("NetworkId:%{public}s have no device existed", Utility::Anonymize(networkId).c_str());
        return;
    }
    remoteDevices_.erase(networkId);
}

void InputDeviceMgr::DispDeviceInfo(std::shared_ptr<IDevice> device)
{
    CHKPV(device);
    FI_HILOGI("  device %{public}d:%{public}s", device->GetId(), device->GetDevPath().c_str());
    FI_HILOGI("  sysPath:       \"%{public}s\"", device->GetSysPath().c_str());
    FI_HILOGI("  bus:           %{public}04x", device->GetBus());
    FI_HILOGI("  vendor:        %{public}04x", device->GetVendor());
    FI_HILOGI("  product:       %{public}04x", device->GetProduct());
    FI_HILOGI("  version:       %{public}04x", device->GetVersion());
    FI_HILOGI("  name:          \"%{public}s\"", device->GetName().c_str());
    FI_HILOGI("  location:      \"%{public}s\"", device->GetPhys().c_str());
    FI_HILOGI("  unique id:     \"%{public}s\"", device->GetUniq().c_str());
    FI_HILOGI("  is pointer:    %{public}s, is keyboard:%{public}s",
        device->IsPointerDevice() ? "True" : "False", device->IsKeyboard() ? "True" : "False");
}

void InputDeviceMgr::DumpRemoteInputDevice(const std::string &networkId)
{
    CALL_INFO_TRACE;
    if (remoteDevices_.find(networkId) == remoteDevices_.end()) {
        FI_HILOGE("NetworkId:%{public}s have no device existed", Utility::Anonymize(networkId).c_str());
        return;
    }
    FI_HILOGI("NetworkId%{public}s, device mount:%{public}zu", Utility::Anonymize(networkId).c_str(),
        remoteDevices_.size());
    for (const auto &elem : remoteDevices_[networkId]) {
        FI_HILOGI("DeviceId:%{public}d, deviceName:%{public}s", elem->GetId(), elem->GetName().c_str());
    }
}

int32_t InputDeviceMgr::SerializeDevice(std::shared_ptr<IDevice> device, NetPacket &packet)
{
    CALL_INFO_TRACE;
    packet << device->GetId() << device->GetDevPath() << device->GetSysPath() << device->GetBus() <<
    device->GetVendor() << device->GetProduct() << device->GetVersion() << device->GetName() <<
    device->GetPhys() << device->GetUniq() << device->IsPointerDevice()  << device->IsKeyboard() <<
    static_cast<int32_t> (device->GetKeyboardType());
    if (packet.ChkRWError()) {
        FI_HILOGE("Write packet failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t InputDeviceMgr::DeserializeDevice(std::shared_ptr<IDevice> device, NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    int32_t data;
    std::string str;
    packet >> data;
    device->SetId(data);
    packet >> str;
    device->SetDevPath(str);
    packet >> str;
    device->SetSysPath(str);
    packet >> data;
    device->SetBus(data);
    packet >> data;
    device->SetVendor(data);
    packet >> data;
    device->SetProduct(data);
    packet >> data;
    device->SetVersion(data);
    packet >> str;
    device->SetName(str);
    packet >> str;
    device->SetPhys(str);
    packet >> str;
    device->SetUniq(str);
    bool isPointerDevice { false };
    packet >> isPointerDevice;
    if (isPointerDevice) {
        device->AddCapability(IDevice::Capability::DEVICE_CAP_POINTER);
    }
    bool isKeyboard { false };
    packet >> isKeyboard;
    if (isKeyboard) {
        device->AddCapability(IDevice::Capability::DEVICE_CAP_KEYBOARD);
    }
    int32_t keyboardType { static_cast<int32_t> (IDevice::KeyboardType::KEYBOARD_TYPE_NONE) };
    packet >> keyboardType;
    device->SetKeyboardType(static_cast<IDevice::KeyboardType>(keyboardType));
    if (packet.ChkRWError()) {
        FI_HILOGE("Packet read type failed");
        return RET_ERR;
    }
    return RET_OK;
}

std::shared_ptr<MMI::InputDevice> InputDeviceMgr::Transform(std::shared_ptr<IDevice> device)
{
    CALL_DEBUG_ENTER;
    CHKPP(device);
    auto inputDevice = std::make_shared<MMI::InputDevice>();
    inputDevice->SetId(device->GetId());
    inputDevice->SetName(device->GetName());
    inputDevice->SetBus(device->GetBus());
    inputDevice->SetVersion(device->GetVersion());
    inputDevice->SetProduct(device->GetProduct());
    inputDevice->SetVendor(device->GetVendor());
    inputDevice->SetPhys(device->GetPhys());
    inputDevice->SetUniq(device->GetUniq());
    if (device->IsKeyboard()) {
        inputDevice->AddCapability(MMI::InputDeviceCapability::INPUT_DEV_CAP_KEYBOARD);
    }
    if (device->IsPointerDevice()) {
        inputDevice->AddCapability(MMI::InputDeviceCapability::INPUT_DEV_CAP_POINTER);
    }
    return inputDevice;
}

void InputDeviceMgr::AddVirtualInputDevice(const std::string &networkId, int32_t remoteDeviceId)
{
    CALL_INFO_TRACE;
    auto device = GetRemoteDeviceById(networkId, remoteDeviceId);
    CHKPV(device);
    int32_t virtualDeviceId = -1;
    if (env_->GetInput().AddVirtualInputDevice(Transform(device), virtualDeviceId) != RET_OK) {
        FI_HILOGE("Add virtual device failed, remoteDeviceId:%{public}d, name:%{public}s", remoteDeviceId,
            device->GetName().c_str());
        return;
    }
    virtualInputDevicesAdded_[networkId].insert(virtualDeviceId);
    remote2VirtualIds_[remoteDeviceId] = virtualDeviceId;
    FI_HILOGI("Add virtual device success, virtualDeviceId:%{public}d", virtualDeviceId);
}

void InputDeviceMgr::RemoveVirtualInputDevice(const std::string &networkId, int32_t remoteDeviceId)
{
    CALL_INFO_TRACE;
    if (remote2VirtualIds_.find(remoteDeviceId) == remote2VirtualIds_.end()) {
        FI_HILOGE("No remote device from networkId%{public}s with id:%{public}d",
            Utility::Anonymize(networkId).c_str(), remoteDeviceId);
        return;
    }
    auto virtualDeviceId = remote2VirtualIds_[remoteDeviceId];
    if (env_->GetInput().RemoveVirtualInputDevice(virtualDeviceId) != RET_OK) {
        FI_HILOGE("Remove virtual device failed, virtualDeviceId:%{public}d", virtualDeviceId);
        return;
    }
    virtualInputDevicesAdded_[networkId].erase(virtualDeviceId);
    remote2VirtualIds_.erase(remoteDeviceId);
    FI_HILOGI("Remove virtual device success, virtualDeviceId:%{public}d", virtualDeviceId);
}

std::shared_ptr<IDevice> InputDeviceMgr::GetRemoteDeviceById(const std::string &networkId, int32_t remoteDeviceId)
{
    std::shared_ptr<IDevice> dev = std::make_shared<Device>(remoteDeviceId);
    if (remoteDevices_.find(networkId) == remoteDevices_.end()) {
        FI_HILOGE("No remoteDevice from networkId:%{public}s", Utility::Anonymize(networkId).c_str());
        return nullptr;
    }
    if (auto iter = remoteDevices_[networkId].find(dev); iter != remoteDevices_[networkId].end()) {
        return *iter;
    }
    FI_HILOGW("No remote device with deviceId:%{public}d", remoteDeviceId);
    return nullptr;
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
