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

#ifndef COOPERATE_DEVICE_MANAGER_H
#define COOPERATE_DEVICE_MANAGER_H

#include <string>
#include <unordered_map>

#include "nocopyable.h"

#include "channel.h"
#include "cooperate_events.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class InputDeviceManager final : public std::enable_shared_from_this<InputDeviceManager> {
public:
    class Device {
    public:
        explicit Device(std::shared_ptr<IDevice> dev);
        int32_t GetId() const;
        std::string GetName() const;
        std::string GetDhid() const;
        std::string GetNetworkId() const;
        bool IsRemote();
        int32_t GetProduct() const;
        int32_t GetVendor() const;
        std::string GetPhys() const;
        std::string GetUniq() const;
        bool IsPointerDevice() const;
        bool IsKeyboard() const;
        IDevice::KeyboardType GetKeyboardType() const;

    private:
        void Populate();
        std::string MakeNetworkId(const std::string &phys) const;
        std::string GenerateDescriptor();
        std::string Sha256(const std::string &in) const;
        std::shared_ptr<IDevice> device_ { nullptr };
        std::string dhid_;
        std::string networkId_;
    };

    InputDeviceManager(IContext *env);
    ~InputDeviceManager();
    DISALLOW_COPY_AND_MOVE(InputDeviceManager);

    void AttachSender(Channel<CooperateEvent>::Sender sender);
    bool Enable();
    void Disable();

    std::shared_ptr<InputDeviceManager::Device> GetDevice(int32_t id) const;
    bool IsRemote(int32_t id);
    std::vector<std::string> GetCooperateDhids(int32_t deviceId) const;
    std::vector<std::string> GetCooperateDhids(const std::string &dhid, bool isRemote) const;
    std::string GetOriginNetworkId(int32_t id) const;
    std::string GetOriginNetworkId(const std::string &dhid) const;
    std::string GetDhid(int32_t deviceId) const;
    bool HasLocalPointerDevice() const;

private:
    class DeviceObserver final : public IDeviceObserver {
    public:
        explicit DeviceObserver(std::shared_ptr<InputDeviceManager> devMgr);
        ~DeviceObserver() = default;

        void OnDeviceAdded(std::shared_ptr<IDevice> device) override;
        void OnDeviceRemoved(std::shared_ptr<IDevice> device) override;

    private:
        std::weak_ptr<InputDeviceManager> devMgr_;
    };

    void OnDeviceAdded(std::shared_ptr<IDevice> device);
    void OnDeviceRemoved(std::shared_ptr<IDevice> device);

    IContext *env_ { nullptr };
    Channel<CooperateEvent>::Sender sender_;
    std::shared_ptr<IDeviceObserver> observer_ { nullptr };
    std::unordered_map<int32_t, std::shared_ptr<Device>> devices_;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_DEVICE_MANAGER_H