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

#include "virtual_mouse_builder.h"

#include <getopt.h>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <linux/input.h>

#include "input_manager.h"

#include "devicestatus_define.h"
#include "fi_log.h"
#include "utility.h"
#include "virtual_mouse.h"

#undef LOG_TAG
#define LOG_TAG "VirtualMouseBuilder"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t MAXIMUM_LEVEL_ALLOWED { 3 };
constexpr uint32_t IO_FLAG_WIDTH { 6 };
const std::unordered_map<std::string, int32_t> mouseBtns {
    { "BTN_LEFT", BTN_LEFT }, { "BTN_RIGHT", BTN_RIGHT },
    { "BTN_MIDDLE", BTN_MIDDLE }, { "BTN_SIDE", BTN_SIDE },
    { "BTN_EXTRA", BTN_EXTRA }, { "BTN_FORWARD", BTN_FORWARD },
    { "BTN_BACK", BTN_BACK }, { "BTN_TASK", BTN_TASK } };
} // namespace

VirtualMouseBuilder::VirtualMouseBuilder() : VirtualDeviceBuilder(GetDeviceName(), BUS_USB, 0x93a, 0x2510)
{
    eventTypes_ = { EV_KEY, EV_REL, EV_MSC };
    keys_ = { BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA, BTN_FORWARD, BTN_BACK, BTN_TASK };
    relBits_ = { REL_X, REL_Y, REL_WHEEL, REL_WHEEL_HI_RES };
    miscellaneous_ = { MSC_SCAN };
}

class MouseEventMonitor final : public MMI::IInputEventConsumer {
public:
    MouseEventMonitor() = default;
    ~MouseEventMonitor() = default;

    void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override {};
    void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
    void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override {};
};

void MouseEventMonitor::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    if (pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        return;
    }
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        return;
    }
    std::cout << "\rcurrent pointer position - x: " << std::setw(IO_FLAG_WIDTH) << std::left <<
        pointerItem.GetDisplayX() << "y: " << pointerItem.GetDisplayY() << "            ";
    std::cout.flush();
}

std::string VirtualMouseBuilder::GetDeviceName()
{
    return std::string("Virtual Mouse");
}

void VirtualMouseBuilder::ShowUsage()
{
    std::cout << "Usage: vdevadm act -t M [-d <mouse-button>] [-u <mouse-button>] [-s <dv>]" << std::endl;
    std::cout << "          [-m <dx> [<dy>]] [-M <x> <y>] [-w <ms>] [-f <FILE>] [-r <FILE>]" << std::endl;
    std::cout << "      -d <mouse-button>" << std::endl;
    std::cout << "                  Down the <mouse-button>" << std::endl;
    std::cout << "      -u <mouse-button>" << std::endl;
    std::cout << "                  Release the <mouse-button>" << std::endl;
    std::cout << "      -s <dy>     Scroll the mouse wheel" << std::endl;
    std::cout << "      -m <dx> [<dy>]" << std::endl;
    std::cout << "                  Move the mouse along <dx, dy>; if <dy> is missing, then set dy=dx" << std::endl;
    std::cout << "      -M <x> <y>  Move the pointer to <x, y>" << std::endl;
    std::cout << "      -D <SLOT> <sx> <sy> <tx> <ty> Drag the touch <SLOT> to (tx, ty)" << std::endl;
    std::cout << "      -w <ms>     Wait for <ms> milliseconds." << std::endl;
    std::cout << "      -f <FILE>   Read actions from <FILE>" << std::endl;
    std::cout << "      -r <FILE>   Read raw input data from <FILE>." << std::endl;
    std::cout << std::endl;
    std::cout << "          <mouse-button> can be:" << std::endl;
    std::cout << "              L   For left mouse button" << std::endl;
    std::cout << "              R   For right mouse button" << std::endl;
    std::cout << "              M   For middle mouse button" << std::endl;
}

void VirtualMouseBuilder::Mount()
{
    CALL_DEBUG_ENTER;
    std::cout << "Start to mount virtual mouse." << std::endl;
    if (VirtualMouse::GetDevice() != nullptr) {
        std::cout << "Virtual mouse has been mounted." << std::endl;
        return;
    }
    VirtualMouseBuilder vMouse;
    if (!vMouse.SetUp()) {
        std::cout << "Failed to mount virtual mouse." << std::endl;
        return;
    }

    int32_t nTries = 6;
    do {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
    } while ((nTries-- > 0) && (VirtualMouse::GetDevice() == nullptr));
    if (VirtualMouse::GetDevice() == nullptr) {
        std::cout << "Failed to mount virtual mouse." << std::endl;
        return;
    }

    std::cout << "Mount virtual mouse successfully." << std::endl;
    VirtualDeviceBuilder::Daemonize();

    for (;;) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void VirtualMouseBuilder::Unmount()
{
    CALL_DEBUG_ENTER;
    VirtualDeviceBuilder::Unmount("mouse", "M");
}

void VirtualMouseBuilder::Clone()
{
    CALL_DEBUG_ENTER;
    if (VirtualMouse::GetDevice() != nullptr) {
        std::cout << "Virtual mouse has been mounted." << std::endl;
        return;
    }

    std::vector<std::shared_ptr<VirtualDevice>> vDevs;
    int32_t ret = VirtualDeviceBuilder::ScanFor(
        [](std::shared_ptr<VirtualDevice> vDev) { return ((vDev != nullptr) && vDev->IsMouse()); }, vDevs);
    if (ret != RET_OK) {
        std::cout << "Failed while scanning for mouse." << std::endl;
        return;
    }
    auto vDev = VirtualDeviceBuilder::Select(vDevs, "mouse");
    CHKPV(vDev);

    std::cout << "Cloning \'" << vDev->GetName() << "\'." << std::endl;
    VirtualDeviceBuilder vBuilder(GetDeviceName(), vDev);
    if (!vBuilder.SetUp()) {
        std::cout << "Clone  \' " << vDev->GetName() << " \' is failed." << std::endl;
        return;
    }
    int32_t nTries = 3;
    do {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    } while ((nTries-- > 0) && (VirtualMouse::GetDevice() == nullptr));
    if (VirtualMouse::GetDevice() == nullptr) {
        std::cout << "Failed to clone \' " << vDev->GetName() << " \'." << std::endl;
        return;
    }

    std::cout << "Clone \'" << vDev->GetName() << "\' successfully." << std::endl;
    VirtualDeviceBuilder::Daemonize();
    for (;;) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void VirtualMouseBuilder::Monitor()
{
    CALL_DEBUG_ENTER;
    MMI::InputManager *inputMgr = MMI::InputManager::GetInstance();
    CHKPV(inputMgr);
    auto monitor = std::make_shared<MouseEventMonitor>();
    int32_t monitorId = inputMgr->AddMonitor(monitor);
    if (monitorId < 0) {
        std::cout << "Failed to add monitor." << std::endl;
        return;
    }
    for (;;) {
        std::this_thread::sleep_for(std::chrono::minutes(1));
    }
}

void VirtualMouseBuilder::Act(int32_t argc, char *argv[])
{
    CALL_DEBUG_ENTER;
    int32_t opt = getopt(argc, argv, "d:u:s:m:M:f:r:w:D:");
    if (opt < 0) {
        std::cout << "Vdevadm act: required option is missing" << std::endl;
        VirtualMouseBuilder::ShowUsage();
        return;
    }
    if (VirtualMouse::GetDevice() == nullptr) {
        std::cout << "No virtual mouse." << std::endl;
        return;
    }
    do {
        {
            auto action = ruleMouseActions_.find(opt);
            if (action != ruleMouseActions_.end()) {
                action->second();
                continue;
            }
        }
        {
            auto action = readMouseActions_.find(opt);
            if (action != readMouseActions_.end()) {
                action->second(optarg);
                continue;
            }
        }
        {
            auto action = moveMouseActions_.find(opt);
            if (action != moveMouseActions_.end()) {
                action->second(argc, argv);
                continue;
            }
        }
        if (opt == 'w') {
            VirtualDeviceBuilder::WaitFor(optarg, "mouse");
        } else {
            ShowUsage();
        }
    } while ((opt = getopt(argc, argv, "d:u:s:m:M:f:r:w:D:")) >= 0);
}

void VirtualMouseBuilder::ReadDownAction()
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);

    if (strcmp(optarg, "L") == 0) {
        std::cout << "[mouse] down button: BTN_LEFT" << std::endl;
        VirtualMouse::GetDevice()->DownButton(BTN_LEFT);
    } else if (strcmp(optarg, "M") == 0) {
        std::cout << "[mouse] down button: BTN_MIDDLE" << std::endl;
        VirtualMouse::GetDevice()->DownButton(BTN_MIDDLE);
    } else if (strcmp(optarg, "R") == 0) {
        std::cout << "[mouse] down button: BTN_RIGHT" << std::endl;
        VirtualMouse::GetDevice()->DownButton(BTN_RIGHT);
    } else {
        std::cout << "Invalid argument for option \'-d\'." << std::endl;
        ShowUsage();
    }
}

void VirtualMouseBuilder::ReadMoveAction(int32_t argc, char *argv[])
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);
    if (!Utility::IsInteger(std::string(optarg)) || (optind < 0) || (optind >= argc) ||
        !Utility::IsInteger(argv[optind])) {
        std::cout << "Invalid arguments for Option \'-m\'." << std::endl;
        ShowUsage();
        return;
    }
    int32_t dx = std::atoi(optarg);
    int32_t dy = dx;

    if ((argv[optind] != nullptr) && Utility::IsInteger(std::string(argv[optind]))) {
        dy = std::atoi(argv[optind++]);
    }
    std::cout << "[mouse] move: (" << dx << "," << dy << ")" << std::endl;
    VirtualMouse::GetDevice()->MoveProcess(dx, dy);
}

void VirtualMouseBuilder::ReadMoveToAction(int32_t argc, char *argv[])
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);

    if (!Utility::IsInteger(optarg) || (optind < 0) || (optind >= argc) || !Utility::IsInteger(argv[optind])) {
        std::cout << "Invalid arguments for Option \'-M\'." << std::endl;
        ShowUsage();
        return;
    }
    int32_t x = std::atoi(optarg);
    int32_t y = std::atoi(argv[optind]);
    std::cout << "[mouse] move-to (" << x << "," << y << ")" << std::endl;
    VirtualMouse::GetDevice()->MoveTo(x, y);
    while ((optind < argc) && Utility::IsInteger(argv[optind])) {
        optind++;
    }
}

void VirtualMouseBuilder::ReadDragToAction(int32_t argc, char *argv[])
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);
    if (!Utility::IsInteger(optarg) || (optind < 0) || (optind >= argc) || !Utility::IsInteger(argv[optind])) {
        std::cout << "Invalid arguments for Option \'-D\'." << std::endl;
        ShowUsage();
        return;
    }
    int32_t x = std::atoi(optarg);
    int32_t y = std::atoi(argv[optind]);

    std::cout << "[mouse] drag-to (" << x << "," << y << ")" << std::endl;
    VirtualMouse::GetDevice()->DownButton(BTN_LEFT);
    VirtualDeviceBuilder::WaitFor("mouse", SLEEP_TIME);
    VirtualMouse::GetDevice()->MoveTo(x, y);
    VirtualMouse::GetDevice()->UpButton(BTN_LEFT);
    while ((optind < argc) && Utility::IsInteger(argv[optind])) {
        optind++;
    }
}

void VirtualMouseBuilder::ReadUpAction()
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);

    if (strcmp(optarg, "L") == 0) {
        std::cout << "[mouse] release button: BTN_LEFT" << std::endl;
        VirtualMouse::GetDevice()->UpButton(BTN_LEFT);
    } else if (strcmp(optarg, "M") == 0) {
        std::cout << "[mouse] release button: BTN_MIDDLE" << std::endl;
        VirtualMouse::GetDevice()->UpButton(BTN_MIDDLE);
    } else if (strcmp(optarg, "R") == 0) {
        std::cout << "[mouse] release button: BTN_RIGHT" << std::endl;
        VirtualMouse::GetDevice()->UpButton(BTN_RIGHT);
    } else {
        std::cout << "Invalid argument for option \'-u\'." << std::endl;
        ShowUsage();
    }
}

void VirtualMouseBuilder::ReadScrollAction()
{
    CALL_DEBUG_ENTER;
    CHKPV(optarg);
    if (!Utility::IsInteger(std::string(optarg))) {
        std::cout << "Invalid arguments for Option \'-s\'." << std::endl;
        ShowUsage();
        return;
    }
    int32_t dy = std::atoi(optarg);
    std::cout << "[mouse] scroll: " << dy << std::endl;
    VirtualMouse::GetDevice()->Scroll(dy);
}

void VirtualMouseBuilder::ReadActions(const char *path)
{
    CALL_DEBUG_ENTER;
    json model;
    int32_t result = VirtualDeviceBuilder::ReadFile(path, model);
    if (result == RET_ERR) {
        FI_HILOGE("Failed to read mouse data from the files");
        return;
    }
    ReadModel(model, MAXIMUM_LEVEL_ALLOWED);
}

void VirtualMouseBuilder::ReadModel(const nlohmann::json &model, int32_t level)
{
    CALL_DEBUG_ENTER;
    if (model.is_object()) {
        auto tIter = model.find("actions");
        if (tIter != model.cend() && tIter->is_array()) {
            std::for_each(tIter->cbegin(), tIter->cend(), [](const auto &item) { ReadAction(item); });
        }
    } else if (model.is_array() && level > 0) {
        for (const auto &m : model) {
            ReadModel(m, level - 1);
        }
    }
}

void VirtualMouseBuilder::ReadAction(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object()) {
        FI_HILOGD("Not an object");
        return;
    }
    auto it = model.find("action");
    if (it != model.cend() && it->is_string()) {
        static const std::unordered_map<std::string, std::function<void(const nlohmann::json &model)>> actions {
            { "down", &HandleDown },
            { "move", &HandleMove },
            { "up", &HandleUp },
            { "scroll", &HandleScroll },
            { "wait", &HandleWait }
        };
        auto actionItr = actions.find(it.value());
        if (actionItr != actions.cend()) {
            actionItr->second(model);
        }
    }
}

void VirtualMouseBuilder::HandleDown(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("button");
    if (it != model.cend() && it->is_string()) {
        auto tIter = mouseBtns.find(it.value());
        if (tIter != mouseBtns.cend()) {
            std::cout << "[mouse] down button: " << tIter->first << std::endl;
            VirtualMouse::GetDevice()->DownButton(tIter->second);
        }
    }
}

void VirtualMouseBuilder::HandleMove(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    int32_t dx = 0;
    int32_t dy = 0;

    auto it = model.find("dx");
    if (it != model.cend() && it->is_number_integer()) {
        dx = it.value();
    }
    it = model.find("dy");
    if (it != model.cend() && it->is_number_integer()) {
        dy = it.value();
    }
    std::cout << "[mouse] move: (" << dx << "," << dy << ")" << std::endl;
    VirtualMouse::GetDevice()->Move(dx, dy);
}

void VirtualMouseBuilder::HandleUp(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("button");
    if (it != model.cend() && it->is_string()) {
        auto tIter = mouseBtns.find(it.value());
        if (tIter != mouseBtns.cend()) {
            std::cout << "[mouse] release button: " << tIter->first << std::endl;
            VirtualMouse::GetDevice()->UpButton(tIter->second);
        }
    }
}

void VirtualMouseBuilder::HandleScroll(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("dy");
    if (it != model.cend() && it->is_number_integer()) {
        int32_t dy = it.value();
        std::cout << "[mouse] scroll: " << dy << std::endl;
        VirtualMouse::GetDevice()->Scroll(dy);
    }
}

void VirtualMouseBuilder::HandleWait(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    auto it = model.find("duration");
    if (it != model.cend() && it->is_number_integer()) {
        int32_t waitTime = it.value();
        std::cout << "[mouse] wait for " << waitTime << " milliseconds" << std::endl;
        VirtualDeviceBuilder::WaitFor("mouse", waitTime);
    }
}

void VirtualMouseBuilder::ReadRawInput(const char *path)
{
    CALL_DEBUG_ENTER;
    json model;
    int32_t result = VirtualDeviceBuilder::ReadFile(path, model);
    if (result == RET_ERR) {
        FI_HILOGE("Failed to read the raw mouse data");
        return;
    }
    ReadRawModel(model, MAXIMUM_LEVEL_ALLOWED);
}

void VirtualMouseBuilder::ReadRawModel(const nlohmann::json &model, int32_t level)
{
    CALL_DEBUG_ENTER;
    if (model.is_object()) {
        auto typeIter = model.find("type");
        if (typeIter == model.cend() || !typeIter->is_string() || (std::string(typeIter.value()).compare("raw") != 0)) {
            std::cout << "Expect raw input data." << std::endl;
            return;
        }
        auto actionIter = model.find("actions");
        if (actionIter != model.cend() && actionIter->is_array()) {
            std::for_each(actionIter->cbegin(), actionIter->cend(), [](const auto &item) { ReadRawData(item); });
        }
    } else if (model.is_array() && level > 0) {
        for (const auto &m : model) {
            ReadRawModel(m, level - 1);
        }
    }
}

void VirtualMouseBuilder::ReadRawData(const nlohmann::json &model)
{
    CALL_DEBUG_ENTER;
    if (!model.is_object()) {
        FI_HILOGD("Not an object");
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
    auto valueIter = model.find("value");
    if (valueIter == model.cend() || !valueIter->is_number_integer()) {
        return;
    }
    std::cout << "[virtual mouse] raw input: [" << typeIter.value() << ", " << codeIter.value() << ", " <<
        valueIter.value() << "]" << std::endl;
    VirtualMouse::GetDevice()->SendEvent(typeIter.value(), codeIter.value(), valueIter.value());
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS