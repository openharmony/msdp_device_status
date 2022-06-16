/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <iomanip>
#include <map>
#include <sstream>

#include "string_ex.h"
#include "unique_fd.h"

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace {
    constexpr uint32_t MS_NS = 1000000;
}
void DevicestatusDumper::DumpDevicestatusSubscriber(int32_t fd)
{
    DEV_HILOGI(SERVICE, "DumpDevicestatusSubscriber start");

    if (appInfoMap_.empty()) {
        DEV_HILOGI(SERVICE, "appInfoMap_ is empty");
        return;
    }
    std::string startTime;
    DumpCurrentTime(startTime);
    dprintf(fd, "Current time: %s \n", startTime.c_str());
    
    for (auto it = appInfoMap_.begin(); it != appInfoMap_.end(); ++it) {
        auto appInfos = it->second;
        for (auto appInfo : appInfos) {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | type:%s | packageName:%s\n",
                appInfo->startTime.c_str(), appInfo->uid, appInfo->pid, GetStatusType(appInfo->type).c_str(),
                appInfo->packageName.c_str());
        }
    }
}

void DevicestatusDumper::DumpDevicestatusChanges(int32_t fd)
{
    DEV_HILOGI(SERVICE, "DumpDevicestatusChanges start");

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
            DEV_HILOGI(SERVICE, "deviceStatusQueue_ is empty");
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
    DEV_HILOGI(SERVICE, "DumpDevicestatusCurrentStatus start");
    std::string startTime;
    DumpCurrentTime(startTime);
    dprintf(fd, "Current time: %s", startTime.c_str());
    dprintf(fd, "Current device status: \n");
    if (datas.size() == 0) {
        dprintf(fd, "No device status available\n");
    }
    int32_t num = 0;
    for (auto it = datas.begin(); it != datas.end(); ++it) {
        if (it->value == DevicestatusDataUtils::VALUE_INVALID) {
            continue;
        }
        num ++;
        dprintf(fd, "Device status Type is %s , current type state is %s .\n",
            GetStatusType(it->type).c_str(), GetDeviceState(it->value).c_str());
    }
    if (num == 0) {
        dprintf(fd, "No device status available\n");
    }
}

std::string DevicestatusDumper::GetDeviceState(const DevicestatusDataUtils::DevicestatusValue &value) const
{
    std::string valueString;
    switch (value) {
        case DevicestatusDataUtils::VALUE_ENTER: {
            valueString = "enter";
            break;
        }
        case DevicestatusDataUtils::VALUE_EXIT: {
            valueString = "exit";
            break;
        }
        case DevicestatusDataUtils::VALUE_INVALID: {
            valueString = "invalid";
            break;
        }
        default: {
            valueString = "unknown";
            break;
        }
    }
    return valueString;
}

std::string DevicestatusDumper::GetStatusType(const DevicestatusDataUtils::DevicestatusType &type) const
{
    std::string typeString;
    switch (type) {
        case DevicestatusDataUtils::TYPE_HIGH_STILL: {
            typeString = "high still";
            break;
        }
        case DevicestatusDataUtils::TYPE_FINE_STILL: {
            typeString = "fine still";
            break;
        }
        case DevicestatusDataUtils::TYPE_CAR_BLUETOOTH: {
            typeString = "car bluetooth";
            break;
        }
        case DevicestatusDataUtils::TYPE_LID_OPEN: {
            typeString = "lid open";
            break;
        }
        default: {
            typeString = "unknown";
            break;
        }
    }
    return typeString;
}

void DevicestatusDumper::DumpCurrentTime(std::string &startTime) const
{
    timespec curTime;
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    if (timeinfo == nullptr) {
        DEV_HILOGI(SERVICE, "DumpCurrentTime get localtime failed");
        return;
    }
    startTime.append(std::to_string(timeinfo->tm_year + BASE_YEAR)).append("-")
        .append(std::to_string(timeinfo->tm_mon + BASE_MON)).append("-").append(std::to_string(timeinfo->tm_mday))
        .append(" ").append(std::to_string(timeinfo->tm_hour)).append(":").append(std::to_string(timeinfo->tm_min))
        .append(":").append(std::to_string(timeinfo->tm_sec)).append(".")
        .append(std::to_string(curTime.tv_nsec / MS_NS));
}

void DevicestatusDumper::DumpIllegalArgsInfo(int32_t fd) const
{
    dprintf(fd, "The arguments are illegal and you can enter '-h' for help.");
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

    auto appInfoSetIter = appInfoMap_.find(appInfo->type);
    if (appInfoSetIter == appInfoMap_.end() && appInfos.insert(appInfo).second) {
        DEV_HILOGI(SERVICE, "no found app info set list of type, insert success");
        appInfoMap_.insert(std::make_pair(appInfo->type, appInfos));
    } else {
        auto iter = appInfoMap_[appInfoSetIter->first].find(appInfo);
        if (iter != appInfoMap_[appInfoSetIter->first].end()) {
            return;
        } else if (appInfoMap_[appInfoSetIter->first].insert(appInfo).second) {
            DEV_HILOGI(SERVICE, "found app info set list of type, insert success");
        }
    }
}

void DevicestatusDumper::RemoveAppInfo(std::shared_ptr<AppInfo> appInfo)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (appInfo->callback == nullptr) {
        return;
    }
    DEV_HILOGI(SERVICE, "appInfoMap_.size=%{public}zu", appInfoMap_.size());

    auto appInfoSetIter = appInfoMap_.find(appInfo->type);
    if (appInfoSetIter == appInfoMap_.end()) {
        return;
    }
    DEV_HILOGI(SERVICE, "callbacklist.size=%{public}zu",
        appInfoMap_[appInfoSetIter->first].size());
    //auto iter = appInfoMap_[appInfoSetIter->first].find(appInfo);
    std::set<std::shared_ptr<AppInfo>>::const_iterator iter;
    for (iter = appInfoMap_[appInfoSetIter->first].begin();
        iter != appInfoMap_[appInfoSetIter->first].end(); iter++) {
        if (iter->get()->pid == appInfo->pid) {
            DEV_HILOGI(SERVICE, "iterato pid is %d", iter->get()->pid);
            DEV_HILOGI(SERVICE, "appinfo pid is %d", appInfo->pid);
            DEV_HILOGI(SERVICE, "find same pid");
            break;
        }
    }

    if (iter != appInfoMap_[appInfoSetIter->first].end()) {
        appInfoMap_[appInfoSetIter->first].erase(iter);
        return;
    }
}

void DevicestatusDumper::pushDeviceStatus(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    auto record = std::make_shared<DeviceStatusRecord>();
    DumpCurrentTime(record->startTime);
    record->data = data;
    deviceStatusQueue_.push(record);
    if (deviceStatusQueue_.size() > MAX_DEVICE_STATUS_SIZE) {
        deviceStatusQueue_.pop();
    }
}

std::string DevicestatusDumper::GetPackageName(Security::AccessToken::AccessTokenID tokenId)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::string packageName = "unknown";
    int32_t tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE: {
            Security::AccessToken::NativeTokenInfo tokenInfo;
            if (Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                DEV_HILOGI(SERVICE, "get native token info fail");
                return packageName;
            }
            packageName = tokenInfo.processName;
            break;
        }
        case Security::AccessToken::ATokenTypeEnum::TOKEN_HAP: {
            Security::AccessToken::HapTokenInfo hapInfo;
            if (Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                DEV_HILOGI(SERVICE, "get hap token info fail");
                return packageName;
            }
            packageName = hapInfo.bundleName;
            break;
        }
        default: {
            DEV_HILOGI(SERVICE, "token type not match");
            break;
        }
    }
    return packageName;
}
} // namespace Msdp
} // namespace OHOS