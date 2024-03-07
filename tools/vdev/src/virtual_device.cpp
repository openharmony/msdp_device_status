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

#include "virtual_device.h"

#include <iostream>
#include <map>

#include <unistd.h>

#include "devicestatus_define.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "VirtualDevice"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr size_t DEFAULT_BUF_SIZE { 1024 };
constexpr int32_t MINIMUM_INTERVAL_ALLOWED { 1 };
constexpr int32_t MAXIMUM_INTERVAL_ALLOWED { 100 };
} // namespace

VirtualDevice::VirtualDevice(const std::string &node)
{
    inputDev_ = std::make_unique<VInputDevice>(node);
    inputDev_->Open();
}

bool VirtualDevice::FindDeviceNode(const std::string &name, std::string &node)
{
    CALL_DEBUG_ENTER;
    std::map<std::string, std::string> nodes;
    GetInputDeviceNodes(nodes);
    FI_HILOGD("There are %{public}zu device nodes", nodes.size());

    std::map<std::string, std::string>::const_iterator cItr = nodes.find(name);
    if (cItr == nodes.cend()) {
        FI_HILOGE("No virtual stylus were found");
        return false;
    }
    FI_HILOGD("Node name : \'%{public}s\'", cItr->second.c_str());
    std::ostringstream ss;
    ss << "/dev/input/" << cItr->second;
    node = ss.str();
    return true;
}

void VirtualDevice::Execute(std::vector<std::string> &results)
{
    CALL_DEBUG_ENTER;
    char buffer[DEFAULT_BUF_SIZE] {};
    FILE *pin = popen("cat /proc/bus/input/devices", "r");
    if (pin == nullptr) {
        FI_HILOGE("Failed to popen command");
        return;
    }
    while (!feof(pin)) {
        if (fgets(buffer, sizeof(buffer), pin) != nullptr) {
            results.push_back(buffer);
        }
    }
    FI_HILOGD("Close phandle");
    pclose(pin);
}

void VirtualDevice::GetInputDeviceNodes(std::map<std::string, std::string> &nodes)
{
    CALL_DEBUG_ENTER;
    std::vector<std::string> results;
    Execute(results);
    if (results.empty()) {
        FI_HILOGE("Failed to list devices");
        return;
    }
    const std::string kname { "Name=\"" };
    const std::string kevent { "event" };
    std::string name;
    for (const auto &res : results) {
        FI_HILOGD("res:%{public}s", res.c_str());
        if (res[0] == 'N') {
            std::string::size_type spos = res.find(kname);
            if (spos != std::string::npos) {
                spos += kname.size();
                std::string::size_type tpos = res.find("\"", spos);
                if (tpos != std::string::npos) {
                    name = res.substr(spos, tpos - spos);
                }
            }
        } else if (!name.empty() && (res[0] == 'H')) {
            std::string::size_type spos = res.find(kevent);
            if (spos != std::string::npos) {
                std::map<std::string, std::string>::const_iterator cItr = nodes.find(name);
                if (cItr != nodes.end()) {
                    nodes.erase(cItr);
                }
                std::string::size_type tpos = spos + kevent.size();
                while (std::isalnum(res[tpos])) {
                    ++tpos;
                }
                auto [_, ret] = nodes.emplace(name, res.substr(spos, tpos - spos));
                if (!ret) {
                    FI_HILOGW("name is duplicated");
                }
                name.clear();
            }
        }
    }
}

int32_t VirtualDevice::SendEvent(uint16_t type, uint16_t code, int32_t value)
{
    CALL_DEBUG_ENTER;
    CHKPR(inputDev_, RET_ERR);
    inputDev_->SendEvent(type, code, value);

    if ((type == EV_SYN) && (code == SYN_REPORT)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(minimumInterval_));
    }
    return RET_OK;
}

std::string VirtualDevice::GetName() const
{
    if (!name_.empty()) {
        return name_;
    }
    if (inputDev_ != nullptr) {
        return inputDev_->GetName();
    }
    return {};
}

void VirtualDevice::SetMinimumInterval(int32_t interval)
{
    minimumInterval_ = std::max(MINIMUM_INTERVAL_ALLOWED, std::min(interval, MAXIMUM_INTERVAL_ALLOWED));
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS