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

#ifndef VIRTUAL_KEYBOARD_BUILDER_H
#define VIRTUAL_KEYBOARD_BUILDER_H

#include <nlohmann/json.hpp>

#include "virtual_device_builder.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class VirtualKeyboardBuilder final : public VirtualDeviceBuilder {
public:
    VirtualKeyboardBuilder();
    ~VirtualKeyboardBuilder() = default;
    DISALLOW_COPY_AND_MOVE(VirtualKeyboardBuilder);

    static std::string GetDeviceName();
    static void ShowUsage();
    static void Mount();
    static void Unmount();
    static void Clone();
    static void Act(int32_t argc, char *argv[]);

private:
    static void ReadDownAction();
    static void ReadUpAction();
    static void ReadActions(const char *path);
    static void ReadModel(const nlohmann::json &model, int32_t level);
    static void ReadAction(const nlohmann::json &model);
    static void HandleDown(const nlohmann::json &model);
    static void HandleUp(const nlohmann::json &model);
    static void HandleWait(const nlohmann::json &model);
    static void ReadRawInput(const char *path);
    static void ReadRawModel(const nlohmann::json &model, int32_t level);
    static void ReadRawData(const nlohmann::json &model);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif //VIRTUAL_KEYBOARD_BUILDER_H