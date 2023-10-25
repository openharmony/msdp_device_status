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

#include <iostream>
#include <getopt.h>

#include "virtual_keyboard_builder.h"
#include "virtual_mouse_builder.h"
#include "virtual_touchscreen_builder.h"

using namespace ::OHOS::Msdp::DeviceStatus;

static void ShowMountHelp()
{
    std::cout << "Usage: vdevadm mount [-t <DEVICE TYPE>]" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
    std::cout << "              K   For virtual keyboard" << std::endl;
}

static void ShowUnmountHelp()
{
    std::cout << "Usage: vdevadm unmount [-t <DEVICE TYPE>]" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
    std::cout << "              K   For virtual keyboard" << std::endl;
}

static void ShowCloneHelp()
{
    std::cout << "Usage: vdevadm clone [-t <DEVICE TYPE>]" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
    std::cout << "              K   For virtual keyboard" << std::endl;
}

static void ShowMonitorHelp()
{
    std::cout << "Usage: vdevadm monitor [-t <DEVICE TYPE>]" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
}

static void ShowActHelp()
{
    std::cout << "Usage: vdevadm act [-t <DEVICE TYPE>]" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
    std::cout << "              K   For virtual keyboard" << std::endl;
}

static void ShowUsage()
{
    std::cout << "Usage: vdevadm [-h] [--help] <command> [args]" << std::endl;
    std::cout << std::endl;
    std::cout << "  Supported commands:" << std::endl;
    std::cout << "      mount       Mount a virtual device" << std::endl;
    std::cout << "      unmount     Unmount the virtual device" << std::endl;
    std::cout << "      clone       Clone a virtual device" << std::endl;
    std::cout << "      monitor     Monitor for current position of pointer" << std::endl;
    std::cout << "      act         Act on the virtual device" << std::endl;
    std::cout << std::endl;
    std::cout << "  Generally supported command args:" << std::endl;
    std::cout << "      -t <DEVICE TYPE>" << std::endl;
    std::cout << "                  Specify virtual device type" << std::endl;
    std::cout << "          <DEVICE TYPE> can be:" << std::endl;
    std::cout << "              M   For virtual mouse" << std::endl;
    std::cout << "              T   For virtual touchscreen" << std::endl;
    std::cout << "              K   For virtual keyboard" << std::endl;
    std::cout << std::endl;
    VirtualMouseBuilder::ShowUsage();
    std::cout << std::endl;
    VirtualTouchScreenBuilder::ShowUsage();
    std::cout << std::endl;
    VirtualKeyboardBuilder::ShowUsage();
    std::cout << std::endl;
}

static void Mount(int32_t argc, char *argv[])
{
    int32_t opt = getopt(argc, argv, "t:");
    if ((opt != 't') || (optarg == nullptr)) {
        std::cout << "vdevadm mount: missing or required option arguments are not provided" << std::endl;
        ShowMountHelp();
        return;
    }

    if (strcmp(optarg, "M") == 0) {
        std::cout << "Mount virtual mouse" << std::endl;
        VirtualMouseBuilder::Mount();
    } else if (strcmp(optarg, "T") == 0) {
        std::cout << "Mount virtual touchscreen" << std::endl;
        VirtualTouchScreenBuilder::Mount();
    } else if (strcmp(optarg, "K") == 0) {
        std::cout << "Mount virtual keyboard" << std::endl;
        VirtualKeyboardBuilder::Mount();
    } else {
        std::cout << "vdevadm mount: invalid argument for option \'-t\'" << std::endl;
        ShowMountHelp();
    }
}

static void Unmount(int32_t argc, char *argv[])
{
    int32_t opt = getopt(argc, argv, "t:");
    if ((opt != 't') || (optarg == nullptr)) {
        std::cout << "vdevadm unmount: missing or required option arguments are not provided" << std::endl;
        ShowUnmountHelp();
        return;
    }
    if (strcmp(optarg, "M") == 0) {
        std::cout << "Unmount virtual mouse" << std::endl;
        VirtualMouseBuilder::Unmount();
    } else if (strcmp(optarg, "T") == 0) {
        std::cout << "Unmount virtual touchscreen" << std::endl;
        VirtualTouchScreenBuilder::Unmount();
    } else if (strcmp(optarg, "K") == 0) {
        std::cout << "Unmount virtual keyboard" << std::endl;
        VirtualKeyboardBuilder::Unmount();
    } else {
        std::cout << "vdevadm unmount: invalid argument for option \'-t\'" << std::endl;
        ShowUnmountHelp();
    }
}

static void Clone(int32_t argc, char *argv[])
{
    int32_t opt = getopt(argc, argv, "t:");
    if ((opt != 't') || (optarg == nullptr)) {
        std::cout << "vdevadm clone: missing or required option arguments are not provided" << std::endl;
        ShowCloneHelp();
        return;
    }

    if (strcmp(optarg, "M") == 0) {
        std::cout << "Clone virtual mouse" << std::endl;
        VirtualMouseBuilder::Clone();
    } else if (strcmp(optarg, "T") == 0) {
        std::cout << "Clone virtual touchscreen" << std::endl;
        VirtualTouchScreenBuilder::Clone();
    } else if (strcmp(optarg, "K") == 0) {
        std::cout << "Clone virtual keyboard" << std::endl;
        VirtualKeyboardBuilder::Clone();
    } else {
        std::cout << "vdevadm clone: invalid argument for option \'-t\'" << std::endl;
        ShowCloneHelp();
    }
}

static void Monitor(int32_t argc, char *argv[])
{
    int32_t opt = getopt(argc, argv, "t:");
    if ((opt != 't') || (optarg == nullptr)) {
        std::cout << "vdevadm monitor: required option is missing" << std::endl;
        ShowMonitorHelp();
        return;
    }

    if (strcmp(optarg, "M") == 0) {
        std::cout << "Monitor for position of current pointer" << std::endl;
        VirtualMouseBuilder::Monitor();
    } else if (strcmp(optarg, "T") == 0) {
        std::cout << "Monitor for current touch position" << std::endl;
        VirtualTouchScreenBuilder::Monitor();
    } else {
        std::cout << "vdevadm clone: invalid argument for option \'-t\'" << std::endl;
        ShowMonitorHelp();
    }
}

static void Act(int32_t argc, char *argv[])
{
    int32_t opt = getopt(argc, argv, "t:");
    if ((opt != 't') || (optarg == nullptr)) {
        std::cout << "vdevadm act: missing or required option arguments are not provided" << std::endl;
        ShowActHelp();
        return;
    }

    if (strcmp(optarg, "M") == 0) {
        std::cout << "Operate virtual mouse" << std::endl;
        VirtualMouseBuilder::Act(argc, argv);
    } else if (strcmp(optarg, "T") == 0) {
        std::cout << "Operate virtual touchscreen" << std::endl;
        VirtualTouchScreenBuilder::Act(argc, argv);
    } else if (strcmp(optarg, "K") == 0) {
        std::cout << "Operate virtual keyboard" << std::endl;
        VirtualKeyboardBuilder::Act(argc, argv);
    } else {
        std::cout << "vdevadm act: invalid argument for option \'-t\'" << std::endl;
        ShowActHelp();
    }
}

int32_t main(int32_t argc, char *argv[])
{
    static const struct option options[] { { "help", no_argument, nullptr, 'h' }, {} };
    int32_t opt = getopt_long(argc, argv, "+h", options, nullptr);
    if (opt >= 0) {
        ShowUsage();
        return EXIT_SUCCESS;
    }
    const char *command = argv[optind++];
    if (command == nullptr) {
        std::cout << "vdevadm: command required" << std::endl;
        ShowUsage();
        return EXIT_FAILURE;
    }
    if (strcmp(command, "mount") == 0) {
        Mount(argc, argv);
    } else if (strcmp(command, "unmount") == 0) {
        Unmount(argc, argv);
    } else if (strcmp(command, "clone") == 0) {
        Clone(argc, argv);
    } else if (strcmp(command, "monitor") == 0) {
        Monitor(argc, argv);
    } else if (strcmp(command, "act") == 0) {
        Act(argc, argv);
    } else {
        std::cout << "vdevadm: invalid command \'" << command << "\'" << std::endl;
        ShowUsage();
    }
    return EXIT_SUCCESS;
}