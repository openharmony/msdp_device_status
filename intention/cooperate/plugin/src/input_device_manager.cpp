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

#include "input_device_manager.h"

#include <openssl/sha.h>
#include <regex>

#include "devicestatus_define.h"
#include "dsoftbus_adapter.h"
#include "event_manager.h"
#include "napi_constants.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "InputDeviceManager" };
constexpr size_t NETWORK_ID_NUMS { 3 };
constexpr size_t DESCRIPTOR_INDEX { 2 };
} // namespace

InputDeviceManager::Device::Device(std::shared_ptr<IDevice> dev)
    : device_(dev)
{
    Populate();
}

int32_t InputDeviceManager::Device::GetId() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetId();
}

std::string InputDeviceManager::Device::GetName() const
{
    CHKPS(device_);
    return device_->GetName();
}

std::string InputDeviceManager::Device::GetDhid() const
{
    return dhid_;
}

std::string InputDeviceManager::Device::GetNetworkId() const
{
    return networkId_;
}

int32_t InputDeviceManager::Device::GetProduct() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetProduct();
}

int32_t InputDeviceManager::Device::GetVendor() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetVendor();
}

std::string InputDeviceManager::Device::GetPhys() const
{
    CHKPS(device_);
    return device_->GetPhys();
}

std::string InputDeviceManager::Device::GetUniq() const
{
    CHKPS(device_);
    return device_->GetUniq();
}

bool InputDeviceManager::Device::IsPointerDevice() const
{
    CHKPF(device_);
    return device_->IsPointerDevice();
}

IDevice::KeyboardType InputDeviceManager::Device::GetKeyboardType() const
{
    CHKPR(device_, IDevice::KeyboardType::KEYBOARD_TYPE_NONE);
    return device_->GetKeyboardType();
}

bool InputDeviceManager::Device::IsKeyboard() const
{
    CHKPF(device_);
    return device_->IsKeyboard();
}

void InputDeviceManager::Device::Populate()
{
    CALL_DEBUG_ENTER;
    if (IsRemote()) {
        networkId_ = MakeNetworkId(GetPhys());
    }
    dhid_ = GenerateDescriptor();
}

bool InputDeviceManager::Device::IsRemote()
{
    const std::string INPUT_VIRTUAL_DEVICE_NAME { "DistributedInput " };
    return (GetName().find(INPUT_VIRTUAL_DEVICE_NAME) != std::string::npos);
}

std::string InputDeviceManager::Device::MakeNetworkId(const std::string &phys) const
{
    std::vector<std::string> idParts;
    const std::string SPLIT_SYMBOL { "|" };
    StringSplit(phys, SPLIT_SYMBOL, idParts);
    if (idParts.size() == NETWORK_ID_NUMS) {
        return idParts[1];
    }
    return {};
}

std::string InputDeviceManager::Device::GenerateDescriptor()
{
    const std::string phys = GetPhys();
    const std::string SPLIT_SYMBOL { "|" };
    const std::string DH_ID_PREFIX { "Input_" };
    std::string descriptor;
    if (IsRemote() && !phys.empty()) {
        FI_HILOGD("physicalPath:%{public}s", phys.c_str());
        std::vector<std::string> idParts;
        StringSplit(phys.c_str(), SPLIT_SYMBOL, idParts);
        if (idParts.size() == NETWORK_ID_NUMS) {
            descriptor = idParts[DESCRIPTOR_INDEX];
        }
        return descriptor;
    }

    const std::string name = GetName();
    const std::string uniq = GetUniq();
    std::string rawDescriptor = StringPrintf(":%04x:%04x:", GetVendor(), GetProduct());

    if (!uniq.empty()) {
        rawDescriptor += "uniqueId:" + uniq;
    }
    if (!phys.empty()) {
        rawDescriptor += "physicalPath:" + phys;
    }
    if (!name.empty()) {
        rawDescriptor += "name:" + std::regex_replace(name, std::regex(" "), "");
    }
    descriptor = DH_ID_PREFIX + Sha256(rawDescriptor);
    FI_HILOGD("Created descriptor raw:%{public}s", rawDescriptor.c_str());
    return descriptor;
}

std::string InputDeviceManager::Device::Sha256(const std::string &in) const
{
    unsigned char out[SHA256_DIGEST_LENGTH * 2 + 1] = { 0 }; // 2:coefficient
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, in.data(), in.size());
    SHA256_Final(&out[SHA256_DIGEST_LENGTH], &ctx);

    constexpr int32_t width = 4;
    constexpr unsigned char mask = 0x0F;
    const char* hexCode = "0123456789abcdef";
    constexpr int32_t DOUBLE_TIMES = 2;
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = out[SHA256_DIGEST_LENGTH + i];
        out[i * DOUBLE_TIMES] = hexCode[(value >> width) & mask];
        out[i * DOUBLE_TIMES + 1] = hexCode[value & mask];
    }
    out[SHA256_DIGEST_LENGTH * DOUBLE_TIMES] = 0;
    return reinterpret_cast<char*>(out);
}

InputDeviceManager::InputDeviceManager(IContext *env)
    : env_(env)
{}

InputDeviceManager::~InputDeviceManager()
{
    Disable();
}

void InputDeviceManager::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    sender_ = sender;
}

bool InputDeviceManager::Enable()
{
    CALL_DEBUG_ENTER;
    if (observer_ != nullptr) {
        return true;
    }
    observer_ = std::make_shared<DeviceObserver>(shared_from_this());
    int32_t ret = env_->GetDeviceManager().AddDeviceObserver(observer_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to add observer");
        observer_.reset();
        return false;
    }
    env_->GetDeviceManager().RetriggerHotplug(observer_);
    return true;
}

void InputDeviceManager::Disable()
{
    CALL_DEBUG_ENTER;
    CHKPV(observer_);
    env_->GetDeviceManager().RemoveDeviceObserver(observer_);
    observer_.reset();
    devices_.clear();
}

bool InputDeviceManager::IsRemote(int32_t id)
{
    if (auto devIter = devices_.find(id); devIter != devices_.end()) {
        CHKPF(devIter->second);
        return devIter->second->IsRemote();
    }
    return false;
}

std::vector<std::string> InputDeviceManager::GetCooperateDhids(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    std::vector<std::string> inputDeviceDhids;
    auto devIter = devices_.find(deviceId);
    if (devIter == devices_.end()) {
        FI_HILOGW("Cannot find pointer id:%{public}d", deviceId);
        return inputDeviceDhids;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Device is nullptr");
        return inputDeviceDhids;
    }
    std::shared_ptr<Device> dev = devIter->second;
    if (!dev->IsPointerDevice()) {
        FI_HILOGD("Not pointer device");
        return inputDeviceDhids;
    }
    inputDeviceDhids.push_back(dev->GetDhid());
    FI_HILOGD("unq:%{public}s, type:%{public}s", inputDeviceDhids.back().c_str(), "pointer");

    const std::string localNetworkId { DSoftbusAdapter::GetLocalNetworkId() };
    const std::string pointerNetworkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };

    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        const std::string networkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };
        if (networkId != pointerNetworkId) {
            continue;
        }
        if (dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            inputDeviceDhids.push_back(dev->GetDhid());
            FI_HILOGD("unq:%{public}s, type:%{public}s", inputDeviceDhids.back().c_str(), "supportkey");
        }
    }
    return inputDeviceDhids;
}

std::vector<std::string> InputDeviceManager::GetCooperateDhids(const std::string &dhid, bool isRemote) const
{
    int32_t deviceId { -1 };
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (dev->GetDhid() == dhid && dev->IsRemote() == isRemote) {
            deviceId = id;
            break;
        }
    }
    return GetCooperateDhids(deviceId);
}

std::string InputDeviceManager::GetOriginNetworkId(int32_t id) const
{
    CALL_INFO_TRACE;
    auto devIter = devices_.find(id);
    if (devIter == devices_.end()) {
        FI_HILOGE("Failed to search for the device, id:%{public}d", id);
        return {};
    }
    CHKPS(devIter->second);
    auto OriginNetworkId = devIter->second->GetNetworkId();
    if (OriginNetworkId.empty()) {
        OriginNetworkId = DSoftbusAdapter::GetLocalNetworkId();
    }
    return OriginNetworkId;
}

std::string InputDeviceManager::GetOriginNetworkId(const std::string &dhid) const
{
    CALL_INFO_TRACE;
    if (dhid.empty()) {
        FI_HILOGD("The current netWorkId is an empty string");
        return {};
    }
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            return dev->GetNetworkId();
        }
    }
    FI_HILOGD("The current netWorkId is an empty string");
    return {};
}

std::string InputDeviceManager::GetDhid(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->GetDhid();
        }
        FI_HILOGW("Device is nullptr");
    }
    return {};
}

bool InputDeviceManager::HasLocalPointerDevice() const
{
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (!dev->IsRemote() && dev->IsPointerDevice()) {
            FI_HILOGD("It is currently a mouse device");
            return true;
        }
    }
    FI_HILOGD("Not currently a mouse device");
    return false;
}

void InputDeviceManager::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    CHKPV(device);
    auto dev = std::make_shared<InputDeviceManager::Device>(device);
    devices_.insert_or_assign(dev->GetId(), dev);
    if (dev->IsKeyboard()) {
        sender_.Send(CooperateEvent(CooperateEventType::INPUT_PLUG_KEYBOARD, InputHotplugEvent {
            .dhid = dev->GetDhid()
        }));
    }
    FI_HILOGD("Add device %{public}d:%{public}s, Dhid:\"%{public}s\", Network id:\"%{public}s\", "
        "local/remote:\"%{public}s\"", device->GetId(), device->GetDevPath().c_str(), dev->GetDhid().c_str(),
        dev->GetNetworkId().c_str(), dev->IsRemote() ? "Remote Device" : "Local Device");
}

void InputDeviceManager::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    CHKPV(device);
    auto iter = devices_.find(device->GetId());
    if (iter == devices_.end()) {
        FI_HILOGE("The device corresponding to the current id:%{public}d cannot be found", device->GetId());
        return;
    }
    std::shared_ptr<Device> dev = iter->second;
    devices_.erase(iter);
    CHKPV(dev);
    if (dev->IsPointerDevice()) {
        sender_.Send(CooperateEvent(CooperateEventType::INPUT_UNPLUG_POINTER, InputHotplugEvent {
            .dhid = dev->GetDhid()
        }));
    } else if (dev->IsKeyboard() && !dev->IsRemote() &&
               (dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD)) {
        sender_.Send(CooperateEvent(CooperateEventType::INPUT_UNPLUG_KEYBOARD, InputHotplugEvent {
            .dhid = dev->GetDhid()
        }));
    }
}

InputDeviceManager::DeviceObserver::DeviceObserver(std::shared_ptr<InputDeviceManager> devMgr)
    : devMgr_(devMgr)
{}

void InputDeviceManager::DeviceObserver::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    std::shared_ptr<InputDeviceManager> devMgr = devMgr_.lock();
    if (devMgr == nullptr) {
        FI_HILOGE("Reference has expired");
        return;
    }
    devMgr->OnDeviceAdded(device);
}

void InputDeviceManager::DeviceObserver::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    std::shared_ptr<InputDeviceManager> devMgr = devMgr_.lock();
    if (devMgr == nullptr) {
        FI_HILOGE("Reference has expired");
        return;
    }
    devMgr->OnDeviceRemoved(device);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS