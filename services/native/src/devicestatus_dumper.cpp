/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "devicestatus_dumper.h"

#include <cinttypes>
#include <csignal>
#include <cstring>
#include <getopt.h>
#include <iomanip>
#include <map>
#include <sstream>

#include "securec.h"
#include "string_ex.h"
#include "unique_fd.h"

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace {
    constexpr uint32_t MS_NS = 1000000;
}
void DevicestatusDumper::ParseCommand(int32_t fd, const std::vector<std::string> &args,
    const std::vector<DevicestatusDataUtils::DevicestatusData> &datas)
{
    int32_t optionIndex = 0;
    struct option dumpOptions[] = {
        {"help", no_argument, 0, 'h'},
        {"subscribe", no_argument, 0, 's'},
        {"list", no_argument, 0, 'l'},
        {"current", no_argument, 0, 'c'},
        {NULL, 0, 0, 0}
    };
    char **argv = new char *[args.size()];
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = new char[args[i].size() + 1];
        if (strcpy_s(argv[i], args[i].size() + 1, args[i].c_str()) != RET_OK) {
            DEV_HILOGE(SERVICE, "strcpy_s error");
            goto RELEASE_RES;
            return;
        }
    }
    optind = 1;
    int32_t c;
    while ((c = getopt_long(args.size(), argv, "hslc", dumpOptions, &optionIndex)) != -1) {
        switch (c) {
            case 'h': {
                DumpHelpInfo(fd);
                break;
            }
            case 's': {
                DumpDevicestatusSubscriber(fd);
                break;
            }
            case 'l': {
                DumpDevicestatusChanges(fd);
                break;
            }
            case 'c': {
                DumpDevicestatusCurrentStatus(fd, datas);
                break;
            }
            default: {
                dprintf(fd, "cmd param is error\n");
                DumpHelpInfo(fd);
                break;
            }
        }
    }
    RELEASE_RES:
    for (size_t i = 0; i < args.size(); ++i) {
        delete[] argv[i];
    }
    delete[] argv;
}

void DevicestatusDumper::DumpDevicestatusSubscriber(int32_t fd)
{
    DEV_HILOGI(SERVICE, "start");
    if (appInfoMap_.empty()) {
        DEV_HILOGI(SERVICE, "appInfoMap_ is empty");
        return;
    }
    std::string startTime;
    DumpCurrentTime(startTime);
    dprintf(fd, "Current time: %s \n", startTime.c_str());
    for (const auto &item : appInfoMap_) {
        for (auto appInfo : item.second) {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | type:%s | packageName:%s\n",
                appInfo->startTime.c_str(), appInfo->uid, appInfo->pid, GetStatusType(appInfo->type).c_str(),
                appInfo->packageName.c_str());
        }
    }
}

void DevicestatusDumper::DumpDevicestatusChanges(int32_t fd)
{
    DEV_HILOGI(SERVICE, "start");
    if (deviceStatusQueue_.empty()) {
        DEV_HILOGI(SERVICE, "deviceStatusQueue_ is empty");
        return;
    }
    std::string startTime;
    DumpCurrentTime(startTime);
    dprintf(fd, "Current time: %s \n", startTime.c_str());
    size_t length = deviceStatusQueue_.size() > MAX_DEVICE_STATUS_SIZE ? \
        MAX_DEVICE_STATUS_SIZE : deviceStatusQueue_.size();
    for (size_t i = 0; i < length; ++i) {
        auto record = deviceStatusQueue_.front();
        if (record == nullptr) {
            DEV_HILOGI(SERVICE, "deviceStatusQueue_ is is null");
            continue;
        }
        deviceStatusQueue_.push(record);
        deviceStatusQueue_.pop();
        dprintf(fd, "startTime:%s | type:%s | value:%s \n",
            record->startTime.c_str(), GetStatusType(record->data.type).c_str(),
            GetDeviceState(record->data.value).c_str());
    }
}

void DevicestatusDumper::DumpDevicestatusCurrentStatus(int32_t fd,
    const std::vector<DevicestatusDataUtils::DevicestatusData> &datas) const
{
    DEV_HILOGI(SERVICE, "start");
    std::string startTime;
    DumpCurrentTime(startTime);
    dprintf(fd, "Current time: %s \n", startTime.c_str());
    dprintf(fd, "Current device status: \n");
    if (datas.empty()) {
        dprintf(fd, "No device status available\n");
        return;
    }
    for (auto it = datas.begin(); it != datas.end(); ++it) {
        if (it->value == DevicestatusDataUtils::VALUE_INVALID) {
            continue;
        }
        dprintf(fd, "Device status DevicestatusType is %s , current type state is %s .\n",
            GetStatusType(it->type).c_str(), GetDeviceState(it->value).c_str());
    }
}

std::string DevicestatusDumper::GetDeviceState(const DevicestatusDataUtils::DevicestatusValue &value) const
{
    std::string state;
    switch (value) {
        case DevicestatusDataUtils::VALUE_ENTER: {
            state = "enter";
            break;
        }
        case DevicestatusDataUtils::VALUE_EXIT: {
            state = "exit";
            break;
        }
        case DevicestatusDataUtils::VALUE_INVALID: {
            state = "invalid";
            break;
        }
        default: {
            state = "unknown";
            break;
        }
    }
    return state;
}

std::string DevicestatusDumper::GetStatusType(const DevicestatusDataUtils::DevicestatusType &type) const
{
    std::string stateType;
    switch (type) {
        case DevicestatusDataUtils::TYPE_STILL: {
            stateType = "high still";
            break;
        }
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            stateType = "fine still";
            break;
        }
        case DevicestatusDataUtils::TYPE_CAR_BLUETOOTH: {
            stateType = "car bluetooth";
            break;
        }
        case DevicestatusDataUtils::TYPE_LID_OPEN: {
            stateType = "lid open";
            break;
        }
        default: {
            stateType = "unknown";
            break;
        }
    }
    return stateType;
}

void DevicestatusDumper::DumpCurrentTime(std::string &startTime) const
{
    timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    if (timeinfo == nullptr) {
        DEV_HILOGI(SERVICE, "get localtime failed");
        return;
    }
    startTime.append(std::to_string(timeinfo->tm_year + BASE_YEAR)).append("-")
        .append(std::to_string(timeinfo->tm_mon + BASE_MON)).append("-").append(std::to_string(timeinfo->tm_mday))
        .append(" ").append(std::to_string(timeinfo->tm_hour)).append(":").append(std::to_string(timeinfo->tm_min))
        .append(":").append(std::to_string(timeinfo->tm_sec)).append(".")
        .append(std::to_string(curTime.tv_nsec / MS_NS));
}

void DevicestatusDumper::DumpHelpInfo(int32_t fd) const
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h: dump help\n");
    dprintf(fd, "      -s: dump the device_status subscribers\n");
    dprintf(fd, "      -l: dump the last 10 device status change\n");
    dprintf(fd, "      -c: dump the device_status current device status\n");
}

void DevicestatusDumper::SaveAppInfo(std::shared_ptr<AppInfo> appInfo)
{
    DEV_HILOGI(SERVICE, "Enter");
    DumpCurrentTime(appInfo->startTime);
    std::set<std::shared_ptr<AppInfo>> appInfos;
    auto iter = appInfoMap_.find(appInfo->type);
    if (iter == appInfoMap_.end()) {
        if (appInfos.insert(appInfo).second) {
            appInfoMap_.insert(std::make_pair(appInfo->type, appInfos));
        }
    } else {
        if (!appInfoMap_[iter->first].insert(appInfo).second) {
            DEV_HILOGW(SERVICE, "duplicated app info");
            return;
        }
    }
}

void DevicestatusDumper::RemoveAppInfo(std::shared_ptr<AppInfo> appInfo)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (appInfo->callback == nullptr) {
        DEV_HILOGW(SERVICE, "callback is null");
        return;
    }
    DEV_HILOGI(SERVICE, "appInfoMap_ size=%{public}zu", appInfoMap_.size());

    auto appInfoSetIter = appInfoMap_.find(appInfo->type);
    if (appInfoSetIter == appInfoMap_.end()) {
        DEV_HILOGE(SERVICE, "not exist %d type appInfo", appInfo->type);
        return;
    }
    DEV_HILOGI(SERVICE, "callbacklist type=%d size=%{public}zu",
        appInfo->type, appInfoMap_[appInfoSetIter->first].size());
    auto iter = appInfoMap_.find(appInfo->type);
    if (iter == appInfoMap_.end()) {
        DEV_HILOGW(SERVICE, "Remove app info is not exists");
        return;
    }
    for (const auto &item : iter->second) {
        if (item->pid == appInfo->pid) {
            iter->second.erase(item);
            break;
        }
    }
}

void DevicestatusDumper::pushDeviceStatus(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::unique_lock lock(mutex_);
    auto record = std::make_shared<DeviceStatusRecord>();
    DumpCurrentTime(record->startTime);
    record->data = data;
    deviceStatusQueue_.push(record);
    if (deviceStatusQueue_.size() > MAX_DEVICE_STATUS_SIZE) {
        deviceStatusQueue_.pop();
    }
}
} // namespace Msdp
} // namespace OHOS