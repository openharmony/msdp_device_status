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

#ifndef I_DEVICE_MANAGER_H
#define I_DEVICE_MANAGER_H

#include <memory>
#include <string>
#include <vector>

#include "i_device.h"
#include "i_device_observer.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDeviceManager {
public:
    IDeviceManager() = default;
    virtual ~IDeviceManager() = default;

    virtual std::shared_ptr<IDevice> GetDevice(int32_t id) const = 0;
    virtual int32_t AddDeviceObserver(std::shared_ptr<IDeviceObserver> observer) = 0;
    virtual void RemoveDeviceObserver(std::shared_ptr<IDeviceObserver> observer) = 0;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    virtual bool IsRemote(int32_t id) const = 0;
    virtual std::vector<std::string> GetCoordinationDhids(int32_t deviceId) const = 0;
    virtual std::vector<std::string> GetCoordinationDhids(const std::string &dhid) const = 0;
    virtual std::string GetOriginNetworkId(int32_t id) const = 0;
    virtual std::string GetOriginNetworkId(const std::string &dhid) const = 0;
    virtual std::string GetDhid(int32_t deviceId) const = 0;
    virtual bool HasLocalPointerDevice() const = 0;
#endif // OHOS_BUILD_ENABLE_COORDINATION
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_DEVICE_MANAGER_H