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

#include "virtual_keyboard_builder.h"

#include <getopt.h>
#include <fstream>
#include <iostream>

#include "devicestatus_define.h"
#include "fi_log.h"
#include "utility.h"
#include "virtual_keyboard.h"

#undef LOG_TAG
#define LOG_TAG "VirtualKeyboardBuilder"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t MAXIMUM_LEVEL_ALLOWED { 3 };
constexpr ssize_t MAXIMUM_FILESIZE_ALLOWED { 0x100000 };
} // namespace

VirtualKeyboardBuilder::VirtualKeyboardBuilder() : VirtualDeviceBuilder(GetDeviceName(), BUS_USB, 0x24ae, 0x4035)
{
    eventTypes_ = { EV_KEY, EV_MSC, EV_LED, EV_REP };
    miscellaneous_ = { MSC_SCAN };
    leds_ = { LED_NUML, LED_CAPSL, LED_SCROLLL, LED_COMPOSE, LED_KANA };
    repeats_ = { REP_DELAY, REP_PERIOD };
    keys_ = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
        61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80,
        81, 82, 83, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 102,
        103, 104, 105, 106, 107, 108, 109, 110, 111, 113, 114, 115, 116, 117, 119, 121, 122, 123, 124, 125,
        126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 140, 142, 150, 152, 158, 159, 161,
        163, 164, 165, 166, 173, 176, 177, 178, 179, 180, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192,
        193, 194, 240, 211, 213, 214, 215, 218, 220, 221, 222, 223, 226, 227, 231, 232, 233, 236, 237, 238,
        239, 242, 243, 245, 246, 247, 248, 464, 522, 523, 141, 145, 146, 147, 148, 149, 151, 153, 154, 157,
        160, 162, 170, 175, 182, 200, 201, 202, 203, 204, 205, 101, 112, 118, 120 };
}

std::string VirtualKeyboardBuilder::GetDeviceName()
{
    return std::string("Virtual Keyboard");
}

void VirtualKeyboardBuilder::ShowUsage()
{
    std::cout << "Usage: vdevadm act -t K [-d <key>] [-u <key>] [-w <ms>] [-f <FILE>] [-r <FILE>]" << std::endl;
    std::cout << "      -d <key>    Down <key>" << std::endl;
    std::cout << "      -u <key>    Release <key>" << std::endl;
    std::cout << "      -w <ms>     Wait for <ms> milliseconds." << std::endl;
    std::cout << "      -f <FILE>   Read actions from <FILE>" << std::endl;
    std::cout << "      -r <FILE>   Read raw input data from <FILE>." << std::endl;
    std::cout << std::endl;
}

void VirtualKeyboardBuilder::Mount()
{
    CALL_DEBUG_ENTER;
    std::cout << "Start to mount virtual keyboard." << std::endl;
    if (VirtualKeyboard::GetDevice() != nullptr) {
        std::cout << "Virtual keyboard has been mounted." << std::endl;
        return;
    }
    VirtualKeyboardBuilder vKeyboard;
    if (!vKeyboard.SetUp()) {
        std::cout << "Failed to mount virtual keyboard." << std::endl;
        return;
    }

    int32_t nTries = 6;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    } while ((nTries-- > 0) && (VirtualKeyboard::GetDevice() == nullptr));
    if (VirtualKeyboard::GetDevice() == nullptr) {
        std::cout << "Failed to mount virtual keyboard." << std::endl;
        return;
    }

    std::cout << "Mount virtual keyboard successfully." << std::endl;
    VirtualDeviceBuilder::Daemonize();

    for (;;) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void VirtualKeyboardBuilder::Unmount()
{
    CALL_DEBUG_ENTER;
    VirtualDeviceBuilder::Unmount("keyboard", "K");
}

void VirtualKeyboardBuilder::Clone()
{
    CALL_DEBUG_ENTER;
    if (VirtualKeyboard::GetDevice() != nullptr) {
        std::cout << "Virtual keyboard has been mounted" << std::endl;
        return;
    }

    std::vector<std::shared_ptr<VirtualDevice>> vDevs;
    int32_t ret = VirtualDeviceBuilder::ScanFor(
        [](std::shared_ptr<VirtualDevice> vDev) { return ((vDev != nullptr) && vDev->IsKeyboard()); }, vDevs);
    if (ret != RET_OK) {
        std::cout << "Failed while scanning for keyboard" << std::endl;
        return;
    }
    auto vDev = VirtualDeviceBuilder::Select(vDevs, "keyboard");
    CHKPV(vDev);
    std::cout << "Cloning \'" << vDev->GetName() << "\'." << std::endl;
    VirtualDeviceBuilder vBuilder(GetDeviceName(), vDev);
    if (!vBuilder.SetUp()) {
        std::cout << "Failed to clone \' " << vDev->GetName() << " \'." << std::endl;
        return;
    }

    int32_t nTries = 3;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while ((nTries-- > 0) && (VirtualKeyboard::GetDevice() == nullptr));
    if (VirtualKeyboard::GetDevice() == nullptr) {
        std::cout << "Failed to clone \' " << vDev->GetName() << " \'." << std::endl;
        return;
    }

    std::cout << "Clone \'" << vDev->GetName() << "\' successfully" << std::endl;
    VirtualDeviceBuilder::Daemonize();
    for (;;) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void VirtualKeyboardBuilder::Act(int32_t argc, char *argv[])
{
    CALL_DEBUG_ENTER;
    int32_t opt = getopt(argc, argv, "d:u:f:r:w:");
    if (opt < 0) {
        std::cout << "Vdevadm act: required option is missing" << std::endl;
        ShowUsage();
        return;
    }
    if (VirtualKeyboard::GetDevice() == nullptr) {
        std::cout << "No virtual keyboard." << std::endl;
        return;
    }
    do {
        switch (opt) {
            case 'd': {
                ReadDownAction();
                break;
            }
            case 'u': {
                ReadUpAction();
                break;
            }
            case 'f': {
                ReadActions(optarg);
                break;
            }
            case 'r': {
                ReadRawInput(optarg);
                break;
            }
            case 'w': {
                VirtualDeviceBuilder::WaitFor(optarg, "keyboard");
                break;
            }
            default: {
                ShowUsage();
                break;
            }
        }
    } while ((opt = getopt(argc, argv, "d:u:f:r:w:")) >= 0);
}

void VirtualKeyboardBuilder::ReadDownAction()
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);
    if (!Utility::IsInteger(optarg)) {
        std::cout << "Require arguments for Option \'-d\'." << std::endl;
        ShowUsage();
        return;
    }

    int32_t key = std::atoi(optarg);
    std::cout << "[keyboard] down key: [" << key << "]" << std::endl;
    VirtualKeyboard::GetDevice()->Down(key);
}

void VirtualKeyboardBuilder::ReadUpAction()
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);
    if (!Utility::IsInteger(optarg)) {
        std::cout << "Require arguments for Option \'-u\'." << std::endl;
        ShowUsage();
        return;
    }

    int32_t key = std::atoi(optarg);
    std::cout << "[keyboard] release key: [" << key << "]" << std::endl;
    VirtualKeyboard::GetDevice()->Up(key);
}

void VirtualKeyboardBuilder::ReadActions(const char *path)
{
    CALL_DEBUG_ENTER;
    CHKPV(path);
    char realPath[PATH_MAX] {};
    if (realpath(path, realPath) == nullptr) {
        std::cout << "[keyboard] an invalid path: " << path << std::endl;
        return;
    }
    if (Utility::GetFileSize(realPath) > MAXIMUM_FILESIZE_ALLOWED) {
        std::cout << "[keyboard] the file size is too large" << std::endl;
        return;
    }
    json model;
    int32_t ret = VirtualDeviceBuilder::ReadFile(realPath, model);
    if (ret == RET_ERR) {
        FI_HILOGE("Failed to read the file");
        return;
    }
    ReadModel(model, MAXIMUM_LEVEL_ALLOWED);
}

void VirtualKeyboardBuilder::ReadModel(const nlohmann::json &model, int32_t level)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object() && !model.is_array()) {
        FI_HILOGE("model is not an array or object");
        return;
    }
    if (model.is_object()) {
        auto tIter = model.find("actions");
        if (tIter != model.cend() && tIter->is_array()) {
            std::for_each(tIter->cbegin(), tIter->cend(), [](const auto &item) { ReadAction(item); });
        }
    }
    if (model.is_array() && level > 0) {
        for (const auto &m : model) {
            ReadModel(m, level - 1);
        }
    }
}

void VirtualKeyboardBuilder::ReadAction(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object()) {
        FI_HILOGD("Not an object");
        return;
    }
    auto it = model.find("action");
    if (it != model.cend() && it->is_string()) {
        static const std::unordered_map<std::string, std::function<void(const nlohmann::json &model)>> actions {
            { "down", &VirtualKeyboardBuilder::HandleDown },
            { "up", &VirtualKeyboardBuilder::HandleUp },
            { "wait", &VirtualKeyboardBuilder::HandleWait }
        };
        auto actionItr = actions.find(it.value());
        if (actionItr != actions.cend()) {
            actionItr->second(model);
        }
    }
}

void VirtualKeyboardBuilder::HandleDown(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("key");
    if (it != model.cend() && it->is_number_integer()) {
        std::cout << "[virtual keyboard] down key: " << it.value() << std::endl;
        VirtualKeyboard::GetDevice()->Down(it.value());
    }
}

void VirtualKeyboardBuilder::HandleUp(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("key");
    if (it != model.cend() && it->is_number_integer()) {
        std::cout << "[virtual keyboard] release key: " << it.value() << std::endl;
        VirtualKeyboard::GetDevice()->Up(it.value());
    }
}

void VirtualKeyboardBuilder::HandleWait(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("duration");
    if (it != model.cend() && it->is_number_integer()) {
        int32_t waitTime = it.value();
        std::cout << "[virtual keyboard] wait for " << waitTime << " milliseconds" << std::endl;
        VirtualDeviceBuilder::WaitFor("virtual keyboard", waitTime);
    }
}

void VirtualKeyboardBuilder::ReadRawInput(const char *path)
{
    CALL_DEBUG_ENTER;
    CHKPV(path);
    char realPath[PATH_MAX] {};

    if (realpath(path, realPath) == nullptr) {
        std::cout << "[keyboard] invalid path: " << path << std::endl;
        return;
    }
    if (Utility::GetFileSize(realPath) > MAXIMUM_FILESIZE_ALLOWED) {
        std::cout << "[keyboard] file is too large" << std::endl;
        return;
    }
    json model;

    int32_t ret = VirtualDeviceBuilder::ReadFile(realPath, model);
    if (ret == RET_ERR) {
        FI_HILOGE("Failed to read raw input data");
        return;
    }
    ReadRawModel(model, MAXIMUM_LEVEL_ALLOWED);
}

void VirtualKeyboardBuilder::ReadRawModel(const nlohmann::json &model, int32_t level)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object() && !model.is_array()) {
        FI_HILOGE("model is not an array or object");
        return;
    }
    if (model.is_object()) {
        auto typeIter = model.find("type");
        if (typeIter == model.cend() || !typeIter->is_string() || (std::string(typeIter.value()).compare("raw") != 0)) {
            std::cout << "Expect raw input data" << std::endl;
            return;
        }
        auto actionIter = model.find("actions");
        if (actionIter != model.cend() && actionIter->is_array()) {
            std::for_each(actionIter->cbegin(), actionIter->cend(), [](const auto &item) { ReadRawData(item); });
        }
    }
    if (model.is_array() && level > 0) {
        for (const auto &m : model) {
            ReadRawModel(m, level - 1);
        }
    }
}

void VirtualKeyboardBuilder::ReadRawData(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object()) {
        FI_HILOGE("model is not an object");
        return;
    }
    auto valueIter = model.find("value");
    if (valueIter == model.cend() || !valueIter->is_number_integer()) {
        return;
    }
    auto codeIter = model.find("code");
    if (codeIter == model.cend() || !codeIter->is_number_integer()) {
        return;
    }
    auto typeIter = model.find("type");
    if (typeIter == model.cend() || !typeIter->is_number_integer()) {
        return;
    }
    std::cout << "[virtual keyboard] raw input: [" << typeIter.value() << ", " << codeIter.value() << ", " <<
        valueIter.value() << "]" << std::endl;
    VirtualKeyboard::GetDevice()->SendEvent(typeIter.value(), codeIter.value(), valueIter.value());
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS