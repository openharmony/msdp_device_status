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

#ifndef VIRTUAL_TOUCHSCREEN_H
#define VIRTUAL_TOUCHSCREEN_H

#include <vector>

#include "virtual_device.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class VirtualTouchScreen final : public VirtualDevice {
public:
    static VirtualTouchScreen *GetDevice();
    ~VirtualTouchScreen() = default;
    DISALLOW_COPY_AND_MOVE(VirtualTouchScreen);

    int32_t DownButton(int32_t slot, int32_t x, int32_t y);
    int32_t UpButton(int32_t slot);
    int32_t Move(int32_t slot, int32_t dx, int32_t dy);
    int32_t MoveTo(int32_t slot, int32_t x, int32_t y);
    void SendTouchEvent();

private:
    struct Slot {
        bool active { false };
        Coordinate coord {};
    };

    explicit VirtualTouchScreen(const std::string &name);
    void QueryScreenSize();

private:
    static VirtualTouchScreen *device_;
    std::vector<Slot> slots_;
    int32_t screenWidth_ { std::numeric_limits<int32_t>::max() };
    int32_t screenHeight_ { std::numeric_limits<int32_t>::max() };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_TOUCHSCREEN_H