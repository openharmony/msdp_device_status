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

#ifndef OHOS_MSDP_DEVICE_STATUS_DEVICE_H
#define OHOS_MSDP_DEVICE_STATUS_DEVICE_H

#include <string>
#include <vector>

#include "input_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Device final {
public:
    Device(std::shared_ptr<::OHOS::MMI::InputDevice> inputDev);
    ~Device() = default;
    DISALLOW_COPY_AND_MOVE(Device);

    int32_t GetId() const;
    std::string GetName() const;
    int32_t GetType() const;
    int32_t GetBus() const;
    int32_t GetVersion() const;
    int32_t GetProduct() const;
    int32_t GetVendor() const;
    std::string GetPhys() const;
    std::string GetUniq() const;

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::string GetDhid() const;
    std::string GetNetworkId() const;
    bool IsRemote() const;
#endif // OHOS_BUILD_ENABLE_COORDINATION

    ::OHOS::MMI::KeyboardType GetKeyboardType() const;
    bool IsPointerDevice() const;
    bool IsKeyboard() const;

private:
    void Populate();
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::string MakeNetworkId(const std::string &phys) const;
    std::string GenerateDescriptor() const;
    std::string Sha256(const std::string &in) const;
#endif // OHOS_BUILD_ENABLE_COORDINATION
    void onKeyboardTypeObtained(int32_t keyboardType);

private:
    std::shared_ptr<::OHOS::MMI::InputDevice> inputDev_;
    std::string dhid_;
    std::string networkId_;
    ::OHOS::MMI::KeyboardType keyboardType_ { ::OHOS::MMI::KEYBOARD_TYPE_NONE };
};

inline int32_t Device::GetId() const
{
    return inputDev_->GetId();
}

inline std::string Device::GetName() const
{
    return inputDev_->GetName();
}

inline int32_t Device::GetType() const
{
    return inputDev_->GetType();
}

inline int32_t Device::GetBus() const
{
    return inputDev_->GetBus();
}

inline int32_t Device::GetVersion() const
{
    return inputDev_->GetVersion();
}

inline int32_t Device::GetProduct() const
{
    return inputDev_->GetProduct();
}

inline int32_t Device::GetVendor() const
{
    return inputDev_->GetVendor();
}

inline std::string Device::GetPhys() const
{
    return inputDev_->GetPhys();
}

inline std::string Device::GetUniq() const
{
    return inputDev_->GetUniq();
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION

inline std::string Device::GetDhid() const
{
    return dhid_;
}

inline std::string Device::GetNetworkId() const
{
    return networkId_;
}

#endif // OHOS_BUILD_ENABLE_COORDINATION

inline ::OHOS::MMI::KeyboardType Device::GetKeyboardType() const
{
    return (IsKeyboard() ? keyboardType_ : ::OHOS::MMI::KEYBOARD_TYPE_NONE);
}

inline bool Device::IsPointerDevice() const
{
    return inputDev_->HasCapability(::OHOS::MMI::INPUT_DEV_CAP_POINTER);
}

inline bool Device::IsKeyboard() const
{
    return inputDev_->HasCapability(::OHOS::MMI::INPUT_DEV_CAP_KEYBOARD);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_DEVICE_H