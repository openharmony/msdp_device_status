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

#include "coordination_device_manager.h"

#include <openssl/sha.h>
#include <regex>

#include "coordination_event_manager.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "devicestatus_define.h"
#include "napi_constants.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationDeviceManager"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr size_t NETWORK_ID_NUMS { 3 };
constexpr size_t DESCRIPTOR_INDEX { 2 };
const std::string FINGER_PRINT { "hw_fingerprint_mouse" };
} // namespace

CoordinationDeviceManager::CoordinationDeviceManager() {}
CoordinationDeviceManager::~CoordinationDeviceManager() {}

CoordinationDeviceManager::Device::Device(std::shared_ptr<IDevice> dev)
    : device_(dev)
{
    Populate();
}

int32_t CoordinationDeviceManager::Device::GetId() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetId();
}

std::string CoordinationDeviceManager::Device::GetName() const
{
    CHKPS(device_);
    return device_->GetName();
}

std::string CoordinationDeviceManager::Device::GetDhid() const
{
    return dhid_;
}

std::string CoordinationDeviceManager::Device::GetNetworkId() const
{
    return networkId_;
}

int32_t CoordinationDeviceManager::Device::GetProduct() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetProduct();
}

int32_t CoordinationDeviceManager::Device::GetVendor() const
{
    CHKPR(device_, RET_ERR);
    return device_->GetVendor();
}

std::string CoordinationDeviceManager::Device::GetPhys() const
{
    CHKPS(device_);
    return device_->GetPhys();
}

std::string CoordinationDeviceManager::Device::GetUniq() const
{
    CHKPS(device_);
    return device_->GetUniq();
}

bool CoordinationDeviceManager::Device::IsPointerDevice() const
{
    CHKPF(device_);
    if (GetName() == FINGER_PRINT) {
        return false;
    }
    return device_->IsPointerDevice();
}

IDevice::KeyboardType CoordinationDeviceManager::Device::GetKeyboardType() const
{
    CHKPR(device_, IDevice::KeyboardType::KEYBOARD_TYPE_NONE);
    return device_->GetKeyboardType();
}

bool CoordinationDeviceManager::Device::IsKeyboard() const
{
    CHKPF(device_);
    return device_->IsKeyboard();
}

void CoordinationDeviceManager::Device::Populate()
{
    CALL_DEBUG_ENTER;
    if (IsRemote()) {
        networkId_ = MakeNetworkId(GetPhys());
    }
    dhid_ = GenerateDescriptor();
}

bool CoordinationDeviceManager::Device::IsRemote()
{
    const std::string INPUT_VIRTUAL_DEVICE_NAME { "DistributedInput " };
    return (GetName().find(INPUT_VIRTUAL_DEVICE_NAME) != std::string::npos);
}

std::string CoordinationDeviceManager::Device::MakeNetworkId(const std::string &phys) const
{
    std::vector<std::string> idParts;
    const std::string SPLIT_SYMBOL { "|" };
    StringSplit(phys, SPLIT_SYMBOL, idParts);
    if (idParts.size() == NETWORK_ID_NUMS) {
        return idParts[1];
    }
    return {};
}

std::string CoordinationDeviceManager::Device::GenerateDescriptor()
{
    const std::string phys = GetPhys();
    const std::string SPLIT_SYMBOL { "|" };
    const std::string DH_ID_PREFIX { "Input_" };
    std::string descriptor;
    if (IsRemote() && !phys.empty()) {
        FI_HILOGD("Generate descriptor, physicalPath:%{public}s", phys.c_str());
        std::vector<std::string> idParts;
        StringSplit(phys.c_str(), SPLIT_SYMBOL, idParts);
        if (idParts.size() == NETWORK_ID_NUMS && idParts.size() > DESCRIPTOR_INDEX) {
            descriptor = idParts[DESCRIPTOR_INDEX];
        }
        return descriptor;
    }

    const std::string uniq = GetUniq();
    const std::string name = GetName();
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
    FI_HILOGD("Generate descriptor, created descriptor raw:%{public}s", rawDescriptor.c_str());
    return descriptor;
}

std::string CoordinationDeviceManager::Device::Sha256(const std::string &in) const
{
    unsigned char out[1 + SHA256_DIGEST_LENGTH * 2] = { 0 };
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, in.data(), in.size());
    SHA256_Final(&out[SHA256_DIGEST_LENGTH], &ctx);

    const char* hexCode = "0123456789abcdef";
    constexpr int32_t DOUBLE_TIMES = 2;
    constexpr int32_t width = 4;
    constexpr unsigned char mask = 0x0F;
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = out[SHA256_DIGEST_LENGTH + i];
        out[i * DOUBLE_TIMES] = hexCode[(value >> width) & mask];
        out[i * DOUBLE_TIMES + 1] = hexCode[value & mask];
    }
    out[DOUBLE_TIMES * SHA256_DIGEST_LENGTH] = { 0 };
    return reinterpret_cast<char*>(out);
}

void CoordinationDeviceManager::Init()
{
    CALL_INFO_TRACE;
    auto* context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    devObserver_ = std::make_shared<DeviceObserver>(*this);
    context->GetDeviceManager().AddDeviceObserver(devObserver_);
    context->GetDeviceManager().RetriggerHotplug(devObserver_);
}

bool CoordinationDeviceManager::IsRemote(int32_t id)
{
    if (auto devIter = devices_.find(id); devIter != devices_.end()) {
        CHKPF(devIter->second);
        return devIter->second->IsRemote();
    }
    return false;
}

std::vector<std::string> CoordinationDeviceManager::GetCoordinationDhids(int32_t deviceId) const
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> inputDeviceDhids;
    auto devIter = devices_.find(deviceId);
    if (devIter == devices_.end()) {
        FI_HILOGW("Failed to search for the pointer id, id:%{public}d", deviceId);
        return inputDeviceDhids;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Cannot find the device");
        return inputDeviceDhids;
    }
    std::shared_ptr<Device> dev = devIter->second;
    if (!dev->IsPointerDevice()) {
        FI_HILOGD("It is not pointer device");
        return inputDeviceDhids;
    }
    inputDeviceDhids.push_back(dev->GetDhid());
    FI_HILOGD("unq:%{public}s, type:%{public}s", inputDeviceDhids.back().c_str(), "pointer");

    const std::string localNetworkId { COORDINATION::GetLocalNetworkId() };
    const std::string pointerNetworkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };

    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        const std::string networkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };
        if (networkId != pointerNetworkId) {
            continue;
        }
        if (dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            inputDeviceDhids.push_back(dev->GetDhid());
            FI_HILOGD("type:%{public}s, unq:%{public}s", "supportkey", inputDeviceDhids.back().c_str());
        }
    }
    return inputDeviceDhids;
}

std::vector<std::string> CoordinationDeviceManager::GetCoordinationDhids(const std::string &dhid, bool isRemote) const
{
    int32_t deviceId { -1 };
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (dev->GetDhid() == dhid && dev->IsRemote() == isRemote) {
            deviceId = id;
            break;
        }
    }
    return GetCoordinationDhids(deviceId);
}

std::string CoordinationDeviceManager::GetOriginNetworkId(int32_t id) const
{
    CALL_INFO_TRACE;
    auto devIter = devices_.find(id);
    if (devIter == devices_.end()) {
        FI_HILOGE("Cannot find the device, id:%{public}d", id);
        return {};
    }
    CHKPS(devIter->second);
    auto OriginNetworkId = devIter->second->GetNetworkId();
    if (OriginNetworkId.empty()) {
        OriginNetworkId = COORDINATION::GetLocalNetworkId();
    }
    return OriginNetworkId;
}

std::string CoordinationDeviceManager::GetOriginNetworkId(const std::string &dhid) const
{
    CALL_INFO_TRACE;
    if (dhid.empty()) {
        FI_HILOGE("Dhid is an empty string");
        return {};
    }
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            return dev->GetNetworkId();
        }
    }
    FI_HILOGE("Return an empty string as networkId of dhid:%{public}s", GetAnonyString(dhid).c_str());
    return {};
}

std::string CoordinationDeviceManager::GetDhid(int32_t deviceId) const
{
    CALL_DEBUG_ENTER;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->GetDhid();
        }
        FI_HILOGW("Cannot find the device");
    }
    return {};
}

bool CoordinationDeviceManager::HasLocalPointerDevice() const
{
    for (const auto &[id, dev] : devices_) {
        CHKPC(dev);
        if (!dev->IsRemote() && dev->IsPointerDevice()) {
            FI_HILOGD("Local pointer, it is currently a mouse device");
            return true;
        }
    }
    FI_HILOGD("Local pointer, not currently a mouse device");
    return false;
}

void CoordinationDeviceManager::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    CALL_DEBUG_ENTER;
    CHKPV(device);
    auto dev = std::make_shared<CoordinationDeviceManager::Device>(device);
    devices_.insert_or_assign(dev->GetId(), dev);
    if (dev->IsKeyboard()) {
        COOR_SM->OnKeyboardOnline(dev->GetDhid());
    }
    FI_HILOGD("Add device %{public}d:%{public}s, Dhid:\"%{public}s\", Network id:\"%{public}s\", "
        "local/remote:\"%{public}s\"", device->GetId(), device->GetDevPath().c_str(), dev->GetDhid().c_str(),
        GetAnonyString(dev->GetNetworkId()).c_str(), dev->IsRemote() ? "Remote Device" : "Local Device");
}

void CoordinationDeviceManager::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    CALL_DEBUG_ENTER;
    CHKPV(device);
    auto iter = devices_.find(device->GetId());
    if (iter == devices_.end()) {
        FI_HILOGE("Failed to find the device, current id:%{public}d", device->GetId());
        return;
    }
    std::shared_ptr<Device> dev = iter->second;
    CHKPV(dev);
    auto dhids = GetCoordinationDhids(dev->GetId());
    devices_.erase(iter);
    if (device->IsPointerDevice()) {
        COOR_SM->OnPointerOffline(dev->GetDhid(), dhids);
    } else if (device->IsKeyboard()) {
        if (!dev->IsRemote() && dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            COOR_SM->OnKeyboardOffline(dev->GetDhid());
        }
    }
}

CoordinationDeviceManager::DeviceObserver::DeviceObserver(CoordinationDeviceManager &cooDevMgr)
    : cooDevMgr_(cooDevMgr)
{}

void CoordinationDeviceManager::DeviceObserver::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    cooDevMgr_.OnDeviceAdded(device);
}

void CoordinationDeviceManager::DeviceObserver::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    cooDevMgr_.OnDeviceRemoved(device);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS