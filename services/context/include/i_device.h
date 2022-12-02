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

#ifndef OHOS_MSDP_DEVICE_STATUS_I_DEVICE_H
#define OHOS_MSDP_DEVICE_STATUS_I_DEVICE_H

#include <string>

#include "input_device.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDevice {
public:
    IDevice() = default;
    virtual ~IDevice() = default;

    virtual int32_t GetId() const = 0;
    virtual std::string GetName() const = 0;
    virtual int32_t GetType() const = 0;
    virtual int32_t GetBus() const = 0;
    virtual int32_t GetVersion() const = 0;
    virtual int32_t GetProduct() const = 0;
    virtual int32_t GetVendor() const = 0;
    virtual std::string GetPhys() const = 0;
    virtual std::string GetUniq() const = 0;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    virtual std::string GetDhid() const = 0;
    virtual std::string GetNetworkId() const = 0;
    virtual bool IsRemote() const = 0;
#endif // OHOS_BUILD_ENABLE_COORDINATION

    virtual ::OHOS::MMI::KeyboardType GetKeyboardType() const = 0;
    virtual bool IsPointerDevice() const = 0;
    virtual bool IsKeyboard() const = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_I_DEVICE_H