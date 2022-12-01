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
class DeviceManager final : public IDeviceManager,
                            public ::OHOS::MMI::IInputDeviceListener,
                            public std::enable_shared_from_this<DeviceManager> {
public:
    DeviceManager() = default;
    ~DeviceManager() = default;
    DISALLOW_COPY_AND_MOVE(DeviceManager);

    int32_t Enable(IContext *context);
    void Disable();
    std::shared_ptr<IDevice> GetDevice(int32_t id) const override;
    void Dump(int32_t fd, const std::vector<std::string> &args);

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
    void OnDeviceAdded(int32_t deviceId, const std::string &type) override;
    void OnDeviceRemoved(int32_t deviceId, const std::string &type) override;
    void OnDeviceIdsObtained(std::vector<int32_t> &deviceIds);
    void OnDeviceInfoObtained(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);
    int32_t AddDevice(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);
    int32_t RemoveDevice(int32_t deviceId);

private:
    IContext *context_ { nullptr };
    std::set<std::shared_ptr<IDeviceObserver>> observers_;
    std::unordered_map<int32_t, std::shared_ptr<Device>> devices_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_DEVICE_MANAGER_H
