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

#include <string>
#include <unordered_map>
#include <vector>

#include "singleton.h"

#include "device.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceManager final : public ::OHOS::MMI::IInputDeviceListener,
                            public std::enable_shared_from_this<DeviceManager> {
    DECLARE_DELAYED_SINGLETON(DeviceManager);

public:
    DISALLOW_COPY_AND_MOVE(DeviceManager);
    int32_t Enable();
    void Disable();
    std::shared_ptr<Device> GetDevice(int32_t id) const;
    void Dump(int32_t fd, const std::vector<std::string> &args);

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    bool IsRemote(int32_t id) const;
    std::vector<std::string> GetCooperateDhids(int32_t deviceId);
    std::vector<std::string> GetCooperateDhids(const std::string &dhid);
    std::string GetOriginNetworkId(int32_t id);
    std::string GetOriginNetworkId(const std::string &dhid);
    std::string GetDhid(int32_t deviceId) const;
    bool HasLocalPointerDevice() const;
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    void OnDeviceAdded(int32_t deviceId, const std::string &type) override;
    void OnDeviceRemoved(int32_t deviceId, const std::string &type) override;
    void OnDeviceInfoObtained(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);

private:
    std::unordered_map<int32_t, std::shared_ptr<Device>> devices_;
};

#define DevMgr ::OHOS::DelayedSingleton<DeviceManager>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_DEVICE_MANAGER_H
