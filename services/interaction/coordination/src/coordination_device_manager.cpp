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

#include <regex>
#include <openssl/sha.h>

#include "coordination_event_manager.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "devicestatus_define.h"
#include "napi_constants.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationDeviceManager" };
const std::string INPUT_VIRTUAL_DEVICE_NAME { "DistributedInput " };
const std::string SPLIT_SYMBOL { "|" };
const std::string DH_ID_PREFIX { "Input_" };
const std::string CONFIG_ITEM_KEYBOARD_TYPE { "Key.keyboard.type" };
} // namespace

CoordinationDeviceManager::Device::Device(std::shared_ptr<IDevice> dev)
    : device_(dev)
{
    Populate();
}

void CoordinationDeviceManager::Device::Populate()
{
    CALL_DEBUG_ENTER;
    if (IsRemote()) {
        networkId_ = MakeNetworkId(GetPhys());
    }
    dhid_ = GenerateDescriptor();
}

bool CoordinationDeviceManager::Device::IsRemote() const
{
    return (GetName().find(INPUT_VIRTUAL_DEVICE_NAME) != std::string::npos);
}

std::string CoordinationDeviceManager::Device::MakeNetworkId(const std::string &phys) const
{
    std::vector<std::string> strList;
    StringSplit(phys, SPLIT_SYMBOL, strList);
    if (strList.size() == 3) {
        return strList[1];
    }
    return EMPTYSTR;
}

std::string CoordinationDeviceManager::Device::GenerateDescriptor() const
{
    const std::string phys = GetPhys();
    std::string descriptor;
    if (IsRemote() && !phys.empty()) {
        FI_HILOGI("physicalPath:%{public}s", phys.c_str());
        std::vector<std::string> strList;
        StringSplit(phys.c_str(), SPLIT_SYMBOL, strList);
        if (strList.size() == 3) {
            descriptor = strList[2];
        }
        return descriptor;
    }

    int32_t vendor = GetVendor();
    const std::string name = GetName();
    const std::string uniq = GetUniq();
    int32_t product = GetProduct();
    std::string rawDescriptor = StringPrintf(":%04x:%04x:", vendor, product);

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
    FI_HILOGI("Created descriptor raw: %{public}s", rawDescriptor.c_str());
    return descriptor;
}

std::string CoordinationDeviceManager::Device::Sha256(const std::string &in) const
{
    unsigned char out[SHA256_DIGEST_LENGTH * 2 + 1] = {0};
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, in.data(), in.size());
    SHA256_Final(&out[SHA256_DIGEST_LENGTH], &ctx);
    
    constexpr int32_t WIDTH = 4;
    constexpr unsigned char MASK = 0x0F;
    const char* hexCode = "0123456789abcdef";
    constexpr int32_t DOUBLE_TIMES = 2;
    for (int32_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        unsigned char value = out[SHA256_DIGEST_LENGTH + i];
        out[i * DOUBLE_TIMES] = hexCode[(value >> WIDTH) & MASK];
        out[i * DOUBLE_TIMES + 1] = hexCode[value & MASK];
    }
    out[SHA256_DIGEST_LENGTH * DOUBLE_TIMES] = 0;
    return reinterpret_cast<char*>(out);
}

CoordinationDeviceManager::CoordinationDeviceManager() {}
CoordinationDeviceManager::~CoordinationDeviceManager() {}

void CoordinationDeviceManager::Init()
{
    CALL_INFO_TRACE;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    devObserver_ = std::make_shared<DeviceObserver>(*this);
    context->GetDeviceManager().AddDeviceObserver(devObserver_);
    context->GetDeviceManager().RetriggerHotplug(devObserver_);
}

bool CoordinationDeviceManager::IsRemote(int32_t id) const
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(id); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->IsRemote();
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return false;
}

std::vector<std::string> CoordinationDeviceManager::GetCoordinationDhids(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    std::vector<std::string> dhids;
    auto devIter = devices_.find(deviceId);
    if (devIter == devices_.end()) {
        FI_HILOGI("Find pointer id failed");
        return dhids;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Device is unsynchronized");
        return dhids;
    }
    std::shared_ptr<Device> dev = devIter->second;
    if (!dev->IsPointerDevice()) {
        FI_HILOGI("Not pointer device");
        return dhids;
    }
    dhids.push_back(dev->GetDhid());
    FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "pointer");

    const std::string localNetworkId { COORDINATION::GetLocalDeviceId() };
    const auto pointerNetworkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };

    for (const auto &[id, dev]: devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        const auto networkId { dev->IsRemote() ? dev->GetNetworkId() : localNetworkId };
        if (networkId != pointerNetworkId) {
            continue;
        }
        if (dev->GetKeyboardType() == IDevice::KEYBOARD_TYPE_ALPHABETICKEYBOARD) {
            dhids.push_back(dev->GetDhid());
            FI_HILOGI("unq: %{public}s, type:%{public}s", dhids.back().c_str(), "supportkey");
        }
    }
    return dhids;
}

std::vector<std::string> CoordinationDeviceManager::GetCoordinationDhids(const std::string &dhid) const
{
    int32_t deviceId { -1 };
    for (const auto &[id, dev] : devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        if (dev->GetDhid() == dhid) {
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
        FI_HILOGE("Failed to search for the device: id %{public}d", id);
        return EMPTYSTR;
    }
    if (devIter->second == nullptr) {
        FI_HILOGW("Device is unsynchronized");
        return EMPTYSTR;
    }
    auto networkId = devIter->second->GetNetworkId();
    if (networkId.empty()) {
        networkId = COORDINATION::GetLocalDeviceId();
    }
    return networkId;
}

std::string CoordinationDeviceManager::GetOriginNetworkId(const std::string &dhid) const
{
    CALL_INFO_TRACE;
    if (dhid.empty()) {
        return EMPTYSTR;
    }
    for (const auto &[id, dev] : devices_) {
        if (dev == nullptr) {
            FI_HILOGW("Device is unsynchronized");
            continue;
        }
        if (dev->IsRemote() && dev->GetDhid() == dhid) {
            return dev->GetNetworkId();
        }
    }
    return EMPTYSTR;
}

std::string CoordinationDeviceManager::GetDhid(int32_t deviceId) const
{
    CALL_INFO_TRACE;
    if (auto devIter = devices_.find(deviceId); devIter != devices_.end()) {
        if (devIter->second != nullptr) {
            return devIter->second->GetDhid();
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return EMPTYSTR;
}

bool CoordinationDeviceManager::HasLocalPointerDevice() const
{
    for (const auto &[id, dev] : devices_) {
        if (dev != nullptr) {
            if (!dev->IsRemote() && dev->IsPointerDevice()) {
                return true;
            }
        } else {
            FI_HILOGW("Device is unsynchronized");
        }
    }
    return false;
}

void CoordinationDeviceManager::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    CHKPV(device);
    auto dev = std::make_shared<CoordinationDeviceManager::Device>(device);
    devices_.insert_or_assign(dev->GetId(), dev);
    
    if (dev->IsKeyboard()) {
        CooSM->OnKeyboardOnline(dev->GetDhid());
    }
}

void CoordinationDeviceManager::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    CALL_INFO_TRACE;
    CHKPV(device);
    auto dev = std::make_shared<CoordinationDeviceManager::Device>(device);
    auto tIter = devices_.find(dev->GetId());
    if (tIter != devices_.end()) {
        devices_.erase(tIter);
    }

    if (device->IsPointerDevice()) {
        CooSM->OnPointerOffline(dev->GetDhid(), dev->GetNetworkId(), CooDevMgr->GetCoordinationDhids(dev->GetId()));
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