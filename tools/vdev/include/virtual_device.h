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

#ifndef VIRTUAL_DEVICE_H
#define VIRTUAL_DEVICE_H

#include <map>
#include <memory>
#include <vector>

#include <linux/input.h>

#include "nocopyable.h"

#include "v_input_device.h"
#include "virtual_device_defines.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class VirtualDevice {
public:
    explicit VirtualDevice(const std::string &node);
    virtual ~VirtualDevice() = default;
    DISALLOW_COPY_AND_MOVE(VirtualDevice);

    bool IsActive() const;
    bool IsMouse() const;
    bool IsKeyboard() const;
    bool IsTouchscreen() const;
    bool SupportEventType(size_t ev) const;
    bool SupportKey(size_t key) const;
    bool SupportAbs(size_t abs) const;
    bool SupportRel(size_t rel) const;
    bool SupportMsc(size_t msc) const;
    bool SupportLed(size_t led) const;
    bool SupportRep(size_t rep) const;
    bool SupportProperty(size_t prop) const;
    bool QueryAbsInfo(size_t abs, struct input_absinfo &absInfo);
    std::string GetName() const;
    std::string GetPhys() const;
    struct input_id GetInputId() const;
    void SetName(const std::string &name);
    int32_t SendEvent(uint16_t type, uint16_t code, int32_t value);

protected:
    static bool FindDeviceNode(const std::string &name, std::string &node);
    void SetMinimumInterval(int32_t interval);

private:
    static void Execute(std::vector<std::string> &results);
    static void GetInputDeviceNodes(std::map<std::string, std::string> &nodes);

private:
    int32_t minimumInterval_ { 0 };
    std::unique_ptr<VInputDevice> inputDev_ { nullptr };
    std::string name_;
};

inline bool VirtualDevice::IsActive() const
{
    return ((inputDev_ != nullptr) && inputDev_->IsActive());
}

inline bool VirtualDevice::IsMouse() const
{
    return ((inputDev_ != nullptr) && inputDev_->IsMouse());
}

inline bool VirtualDevice::IsKeyboard() const
{
    return ((inputDev_ != nullptr) && inputDev_->IsKeyboard());
}

inline bool VirtualDevice::IsTouchscreen() const
{
    return ((inputDev_ != nullptr) && inputDev_->IsTouchscreen());
}

inline bool VirtualDevice::SupportEventType(size_t ev) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportEventType(ev));
}

inline bool VirtualDevice::SupportKey(size_t key) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportKey(key));
}

inline bool VirtualDevice::SupportAbs(size_t abs) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportAbs(abs));
}

inline bool VirtualDevice::SupportRel(size_t rel) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportRel(rel));
}

inline bool VirtualDevice::SupportProperty(size_t prop) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportProperty(prop));
}

inline bool VirtualDevice::SupportMsc(size_t msc) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportMsc(msc));
}

inline bool VirtualDevice::SupportLed(size_t led) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportLed(led));
}

inline bool VirtualDevice::SupportRep(size_t rep) const
{
    return ((inputDev_ != nullptr) && inputDev_->SupportRep(rep));
}

inline bool VirtualDevice::QueryAbsInfo(size_t abs, struct input_absinfo &absInfo)
{
    return ((inputDev_ != nullptr) && inputDev_->QueryAbsInfo(abs, absInfo));
}

inline std::string VirtualDevice::GetPhys() const
{
    if (inputDev_ != nullptr) {
        return inputDev_->GetPhys();
    }
    return {};
}

inline struct input_id VirtualDevice::GetInputId() const
{
    if (inputDev_ != nullptr) {
        return inputDev_->GetInputId();
    }
    return {};
}

inline void VirtualDevice::SetName(const std::string &name)
{
    name_ = name;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_DEVICE_H