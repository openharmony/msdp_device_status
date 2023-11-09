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

#ifndef VIRTUAL_TOUCHSCREEN_BUILDER_H
#define VIRTUAL_TOUCHSCREEN_BUILDER_H

#include <memory>

#include <nlohmann/json.hpp>

#include "virtual_device_builder.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class VirtualTouchScreenBuilder final : public VirtualDeviceBuilder {
public:
    VirtualTouchScreenBuilder();
    ~VirtualTouchScreenBuilder() = default;
    DISALLOW_COPY_AND_MOVE(VirtualTouchScreenBuilder);

    static std::string GetDeviceName();
    static void ShowUsage();
    static void Mount();
    static void Unmount();
    static void Clone();
    static void Monitor();
    static void Act(int32_t argc, char *argv[]);

private:
    static void ReadDownAction(int32_t argc, char *argv[]);
    static void ReadMoveAction(int32_t argc, char *argv[]);
    static void ReadUpAction();
    static void ReadMoveToAction(int32_t argc, char *argv[]);
    static void ReadDragToAction(int32_t argc, char *argv[]);
    static void ReadActions(const char *path);
    static void ReadModel(const nlohmann::json &model, int32_t level);
    static void ReadAction(const nlohmann::json &model);
    static void HandleDown(const nlohmann::json &model);
    static void HandleMove(const nlohmann::json &model);
    static void HandleUp(const nlohmann::json &model);
    static void HandleMoveTo(const nlohmann::json &model);
    static void HandleWait(const nlohmann::json &model);
    static void ReadRawInput(const char *path);
    static void ReadRawModel(const nlohmann::json &model, int32_t level);
    static void ReadRawData(const nlohmann::json &model);
    static int32_t GetModelValue(const nlohmann::json &model, const std::string &targetName, int32_t defaultValue);
    using InterfaceParameterLess = void(*)();
    using InterfaceParameterOne = void(*)(const char*);
    using InterfaceParameterTwo = void(*)(int32_t, char**);
    inline static std::map<const char, InterfaceParameterLess> ruleTscrnActions_ {
        { 'u', &ReadUpAction }
    };
    inline static std::map<const char, InterfaceParameterOne> readTscrnActions_ {
        { 'f', &ReadActions }, { 'r', &ReadRawInput}
    };
    inline static std::map<const char, InterfaceParameterTwo> moveTscrnActions_ {
        { 'd', &ReadDownAction }, { 'm', &ReadMoveAction },
        { 'D', &ReadDragToAction }, { 'M', &ReadMoveToAction }
    };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // VIRTUAL_TOUCHSCREEN_BUILDER_H