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

#ifndef OHOS_MSDP_DEVICE_STATUS_DEVICE_MANAGER_H
#define OHOS_MSDP_DEVICE_STATUS_DEVICE_MANAGER_H

#include <future>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "i_input_device_listener.h"
#include "nocopyable.h"

#include "device.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceManager final : public IDeviceManager {
public:
    class InputDeviceListener : public ::OHOS::MMI::IInputDeviceListener {
    public:
        InputDeviceListener(DeviceManager &devMgr);
        void OnDeviceAdded(int32_t deviceId, const std::string &type) override;
        void OnDeviceRemoved(int32_t deviceId, const std::string &type) override;

    private:
        DeviceManager &devMgr_;
    };

public:
    DeviceManager() = default;
    ~DeviceManager() = default;
    DISALLOW_COPY_AND_MOVE(DeviceManager);

    int32_t Init(IContext *context);
    int32_t Enable();
    void Disable();
    std::shared_ptr<IDevice> GetDevice(int32_t id) const override;

    int32_t AddDeviceObserver(std::shared_ptr<IDeviceObserver> observer) override;
    void RemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer) override;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    bool IsRemote(int32_t id) const override;
    std::vector<std::string> GetCooperateDhids(int32_t deviceId) const override;
    std::vector<std::string> GetCooperateDhids(const std::string &dhid) const override;
    std::string GetOriginNetworkId(int32_t id) const override;
    std::string GetOriginNetworkId(const std::string &dhid) const override;
    std::string GetDhid(int32_t deviceId) const override;
    bool HasLocalPointerDevice() const override;
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    int32_t OnInit(IContext *context);
    int32_t OnEnable();
    int32_t OnDisable();
    void OnDeviceAdded(int32_t deviceId, const std::string &type);
    void OnDeviceRemoved(int32_t deviceId, const std::string &type);

    int32_t GetDeviceAsync(int32_t deviceId);
    void AddDevice(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);
    int32_t OnAddDevice(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);
    int32_t OnRemoveDevice(int32_t deviceId);

    void OnGetDeviceIds(std::vector<int32_t> &deviceIds);
    int32_t Synchronize();

    int32_t OnAddDeviceObserver(std::shared_ptr<IDeviceObserver> observer);
    int32_t OnRemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer);

    std::shared_ptr<IDevice> OnGetDevice(int32_t id) const;
    int32_t RunGetDevice(std::packaged_task<std::shared_ptr<IDevice>(int32_t)> &task, int32_t id) const;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    bool OnIsRemote(int32_t id) const;
    int32_t RunIsRemote(std::packaged_task<bool(int32_t)> &task, int32_t id) const;

    std::vector<std::string> OnGetCoopDhids(int32_t deviceId) const;
    int32_t RunGetGetCoopDhids(std::packaged_task<std::vector<std::string>(int32_t)> &task,
                               int32_t deviceId) const;

    std::vector<std::string> OnGetCooperateDhids(const std::string &dhid) const;
    int32_t RunGetCooperateDhids(std::packaged_task<std::vector<std::string>(const std::string &)> &task,
                                 const std::string &dhid) const;

    std::string OnGetOriginNetId(int32_t id) const;
    int32_t RunGetOriginNetId(std::packaged_task<std::string(int32_t)> &task,
                              int32_t id) const;

    std::string OnGetOriginNetworkId(const std::string &dhid) const;
    int32_t RunGetOriginNetworkId(std::packaged_task<std::string(const std::string &)> &task,
                                  const std::string &dhid) const;

    std::string OnGetDhid(int32_t deviceId) const;
    int32_t RunGetDhid(std::packaged_task<std::string(int32_t)> &task, int32_t deviceId) const;

    bool OnHasLocalPointerDevice() const;
    int32_t RunHasLocalPointerDevice(std::packaged_task<bool()> &task) const;
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    IContext *context_ { nullptr };
    std::shared_ptr<::OHOS::MMI::IInputDeviceListener> inputDevListener_ { nullptr };
    std::set<std::shared_ptr<IDeviceObserver>> observers_;
    std::unordered_map<int32_t, std::shared_ptr<Device>> devices_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_DEVICE_MANAGER_H
