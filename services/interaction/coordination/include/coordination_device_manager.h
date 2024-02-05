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

#ifndef COORDINATION_DEVICE_MANAGER_H
#define COORDINATION_DEVICE_MANAGER_H

#include <string>
#include <unordered_map>

#include "singleton.h"

#include "i_device_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationDeviceManager {
    DECLARE_DELAYED_SINGLETON(CoordinationDeviceManager);
public:
    class Device {
    public:
        explicit Device(std::shared_ptr<IDevice> dev);
        int32_t GetId() const;
        std::string GetDhid() const;
        std::string GetName() const;
        std::string GetNetworkId() const;
        bool IsRemote();
        int32_t GetProduct() const;
        int32_t GetVendor() const;
        std::string GetPhys() const;
        std::string GetUniq() const;
        bool IsKeyboard() const;
        bool IsPointerDevice() const;
        IDevice::KeyboardType GetKeyboardType() const;

    private:
        void Populate();
        std::string MakeNetworkId(const std::string &phys) const;
        std::string GenerateDescriptor();
        std::string Sha256(const std::string &in) const;
        std::shared_ptr<IDevice> device_ { nullptr };
        std::string networkId_;
        std::string dhid_;
    };

private:
    class DeviceObserver final : public IDeviceObserver {
    public:
        explicit DeviceObserver(CoordinationDeviceManager &cooDevMgr);
        ~DeviceObserver() = default;
        void OnDeviceAdded(std::shared_ptr<IDevice> device) override;
        void OnDeviceRemoved(std::shared_ptr<IDevice> device) override;

    private:
        CoordinationDeviceManager &cooDevMgr_;
    };

public:
    void Init();
    std::shared_ptr<CoordinationDeviceManager::Device> GetDevice(int32_t id) const;
    bool IsRemote(int32_t id);
    bool HasLocalPointerDevice() const;
    std::vector<std::string> GetCoordinationDhids(int32_t deviceId) const;
    std::vector<std::string> GetCoordinationDhids(const std::string &dhid, bool isRemote) const;
    std::string GetDhid(int32_t deviceId) const;
    std::string GetOriginNetworkId(int32_t id) const;
    std::string GetOriginNetworkId(const std::string &dhid) const;

private:
    void OnDeviceAdded(std::shared_ptr<IDevice> device);
    void OnDeviceRemoved(std::shared_ptr<IDevice> device);

    std::unordered_map<int32_t, std::shared_ptr<Device>> devices_;
    std::shared_ptr<DeviceObserver> devObserver_ { nullptr };
};
#define COOR_DEV_MGR OHOS::DelayedSingleton<CoordinationDeviceManager>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_DEVICE_MANAGER_H