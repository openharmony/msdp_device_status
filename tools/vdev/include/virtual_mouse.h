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

#ifndef VIRTUAL_MOUSE_H
#define VIRTUAL_MOUSE_H

#include <bitset>

#include "virtual_device.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class VirtualMouse final : public VirtualDevice {
public:
    static VirtualMouse *GetDevice();
    ~VirtualMouse() = default;
    DISALLOW_COPY_AND_MOVE(VirtualMouse);

    int32_t DownButton(int32_t button);
    int32_t UpButton(int32_t button);
    void Scroll(int32_t dy);
    void Move(int32_t dx, int32_t dy);
    int32_t MoveTo(int32_t x, int32_t y);
    void MoveProcess(int32_t dx, int32_t dy);

private:
    explicit VirtualMouse(const std::string &name);

private:
    static VirtualMouse *device_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_MOUSE_H