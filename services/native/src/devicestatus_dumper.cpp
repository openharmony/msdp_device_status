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

#include <cinttypes>
#include <csignal>
#include <cstring>
#include <getopt.h>
#include <iomanip>
#include <map>
#include <sstream>

#include <ipc_skeleton.h>
#include "securec.h"
#include "string_ex.h"
#include "unique_fd.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_sm.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_dumper.h"
#include "drag_manager.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusDumper" };
}

int32_t DeviceStatusDumper::Init(IContext *context)
{
    CALL_DEBUG_ENTER;
    CHKPR(context, RET_ERR);
    context_ = context;
    return RET_OK;
}

void DeviceStatusDumper::ParseCommand(int32_t fd, const std::vector<std::string> &args, const std::vector<Data> &datas)
{
    ParseLong(fd, args, datas);
}

void DeviceStatusDumper::ParseLong(int32_t fd, const std::vector<std::string> &args, const std::vector<Data> &datas)
{
    if (args.empty()) {
        DEV_HILOGE(SERVICE, "args is empty");
        return;
    }
    int32_t c;
    optind = 1;
    int32_t optionIndex = 0;
    struct option dumpOptions[] = {
        { "help", no_argument, 0, 'h' },
        { "subscribe", no_argument, 0, 's' },
        { "list", no_argument, 0, 'l' },
        { "current", no_argument, 0, 'c' },
        { "coordination", no_argument, 0, 'o' },
        { "drag", no_argument, 0, 'd' },
        { NULL, 0, 0, 0 }
    };
    char **argv = new (std::nothrow) char *[args.size()];
    if (argv == nullptr) {
        DEV_HILOGE(SERVICE, "argv is nullptr");
        return;
    }
    if (memset_s(argv, args.size() * sizeof(char*), 0, args.size() * sizeof(char*)) != EOK) {
        DEV_HILOGE(SERVICE, "call memset_s failed");
        delete[] argv;
        return;
    }
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i] = new (std::nothrow) char[args[i].size() + 1];
        if (argv[i] == nullptr) {
            DEV_HILOGE(SERVICE, "failed to allocate memory");
            goto RELEASE_RES;
        }
        if (strcpy_s(argv[i], args[i].size() + 1, args[i].c_str()) != RET_OK) {
            DEV_HILOGE(SERVICE, "strcpy_s error");
            goto RELEASE_RES;
        }
    }
    while ((c = getopt_long(args.size(), argv, "hslcod", dumpOptions, &optionIndex)) != -1) {
        ExecutDump(fd, datas, c);
    }
    RELEASE_RES:
    for (size_t i = 0; i < args.size(); ++i) {
        if (argv[i] != nullptr) {
            delete[] argv[i];
        }
    }
    delete[] argv;
}

void DeviceStatusDumper::ExecutDump(int32_t fd, const std::vector<Data> &datas, int32_t info)
{
    switch (info) {
        case 'h': {
            DumpHelpInfo(fd);
            break;
        }
        case 's': {
            DumpDeviceStatusSubscriber(fd);
            break;
        }
        case 'l': {
            DumpDeviceStatusChanges(fd);
            break;
        }
        case 'c': {
            DumpDeviceStatusCurrentStatus(fd, datas);
            break;
        }
        case 'o': {
#ifdef OHOS_BUILD_ENABLE_COORDINATION
            COOR_SM->Dump(fd);
#else
            dprintf(fd, "device coordination is not supported\n");
#endif // OHOS_BUILD_ENABLE_COORDINATION
            break;
        }
        case 'd': {
            CHKPV(context_);
            context_->GetDragManager().Dump(fd);
            break;
        }
        default: {
            dprintf(fd, "cmd param is error\n");
            DumpHelpInfo(fd);
            break;
        }
    }
}

void DeviceStatusDumper::DumpDeviceStatusSubscriber(int32_t fd)
{
    DEV_HILOGD(SERVICE, "start");
    if (appInfoMap_.empty()) {
        DEV_HILOGE(SERVICE, "appInfoMap_ is empty");
        return;
    }
    std::string startTime;
    GetTimeStamp(startTime);
    dprintf(fd, "Current time: %s \n", startTime.c_str());
    for (const auto &item : appInfoMap_) {
        for (auto appInfo : item.second) {
            dprintf(fd, "startTime:%s | uid:%d | pid:%d | type:%s | packageName:%s\n",
                appInfo->startTime.c_str(), appInfo->uid, appInfo->pid, GetStatusType(appInfo->type).c_str(),
                appInfo->packageName.c_str());
        }
    }
}

void DeviceStatusDumper::DumpDeviceStatusChanges(int32_t fd)
{
    DEV_HILOGD(SERVICE, "start");
    if (deviceStatusQueue_.empty()) {
        DEV_HILOGI(SERVICE, "deviceStatusQueue_ is empty");
        return;
    }
    std::string startTime;
    GetTimeStamp(startTime);
    dprintf(fd, "Current time:%s\n", startTime.c_str());
    size_t length = deviceStatusQueue_.size() > MAX_DEVICE_STATUS_SIZE ? \
        MAX_DEVICE_STATUS_SIZE : deviceStatusQueue_.size();
    for (size_t i = 0; i < length; ++i) {
        auto record = deviceStatusQueue_.front();
        if (record == nullptr) {
            DEV_HILOGE(SERVICE, "deviceStatusQueue is nullptr");
            continue;
        }
        deviceStatusQueue_.push(record);
        deviceStatusQueue_.pop();
        dprintf(fd, "startTime:%s | type:%s | value:%s\n",
            record->startTime.c_str(), GetStatusType(record->data.type).c_str(),
            GetDeviceState(record->data.value).c_str());
    }
}

void DeviceStatusDumper::DumpDeviceStatusCurrentStatus(int32_t fd, const std::vector<Data> &datas) const
{
    DEV_HILOGI(SERVICE, "start");
    std::string startTime;
    GetTimeStamp(startTime);
    dprintf(fd, "Current time:%s\n", startTime.c_str());
    dprintf(fd, "Current device status:\n");
    if (datas.empty()) {
        dprintf(fd, "No device status available\n");
        return;
    }
    for (auto it = datas.begin(); it != datas.end(); ++it) {
        if (it->value == VALUE_INVALID) {
            continue;
        }
        dprintf(fd, "type:%s | state:%s\n",
            GetStatusType(it->type).c_str(), GetDeviceState(it->value).c_str());
    }
}

std::string DeviceStatusDumper::GetDeviceState(OnChangedValue value) const
{
    std::string state;
    switch (value) {
        case VALUE_ENTER: {
            state = "enter";
            break;
        }
        case VALUE_EXIT: {
            state = "exit";
            break;
        }
        case VALUE_INVALID: {
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

std::string DeviceStatusDumper::GetStatusType(Type type) const
{
    std::string stateType;
    switch (type) {
        case TYPE_ABSOLUTE_STILL: {
            stateType = "absolute still";
            break;
        }
        case TYPE_HORIZONTAL_POSITION: {
            stateType = "horizontal position";
            break;
        }
        case TYPE_VERTICAL_POSITION: {
            stateType = "vertical position";
            break;
        }
        case TYPE_LID_OPEN: {
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

void DeviceStatusDumper::DumpHelpInfo(int32_t fd) const
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h: dump help\n");
    dprintf(fd, "      -s: dump the subscribers\n");
    dprintf(fd, "      -l: dump the last 10 device status change\n");
    dprintf(fd, "      -c: dump the current device status\n");
    dprintf(fd, "      -o: dump the coordination status\n");
    dprintf(fd, "      -d: dump the drag status\n");
}

void DeviceStatusDumper::SaveAppInfo(std::shared_ptr<AppInfo> appInfo)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (appInfo == nullptr) {
        DEV_HILOGE(SERVICE, "appInfo is nullptr");
        return;
    }
    GetTimeStamp(appInfo->startTime);
    std::set<std::shared_ptr<AppInfo>> appInfos;
    auto iter = appInfoMap_.find(appInfo->type);
    if (iter == appInfoMap_.end()) {
        if (appInfos.insert(appInfo).second) {
            appInfoMap_.insert(std::make_pair(appInfo->type, appInfos));
        }
    } else {
        if (!appInfoMap_[iter->first].insert(appInfo).second) {
            DEV_HILOGW(SERVICE, "duplicated app info");
        }
    }
}

void DeviceStatusDumper::RemoveAppInfo(std::shared_ptr<AppInfo> appInfo)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (appInfo->callback == nullptr) {
        DEV_HILOGW(SERVICE, "callback is nullptr");
        return;
    }
    DEV_HILOGI(SERVICE, "appInfoMap size:%{public}zu", appInfoMap_.size());

    auto appInfoSetIter = appInfoMap_.find(appInfo->type);
    if (appInfoSetIter == appInfoMap_.end()) {
        DEV_HILOGE(SERVICE, "not exist %{public}d type appInfo", appInfo->type);
        return;
    }
    DEV_HILOGI(SERVICE, "callbacklist type:%{public}d, size:%{public}zu",
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

void DeviceStatusDumper::PushDeviceStatus(const Data& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    std::unique_lock lock(mutex_);
    auto record = std::make_shared<DeviceStatusRecord>();
    GetTimeStamp(record->startTime);
    record->data = data;
    deviceStatusQueue_.push(record);
    if (deviceStatusQueue_.size() > MAX_DEVICE_STATUS_SIZE) {
        deviceStatusQueue_.pop();
    }
}

std::string DeviceStatusDumper::GetPackageName(Security::AccessToken::AccessTokenID tokenId)
{
    DEV_HILOGD(SERVICE, "Enter");
    std::string packageName = "unknown";
    int32_t tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE: {
            Security::AccessToken::NativeTokenInfo tokenInfo;
            if (Security::AccessToken::AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                DEV_HILOGE(SERVICE, "get native token info fail");
                return packageName;
            }
            packageName = tokenInfo.processName;
            break;
        }
        case Security::AccessToken::ATokenTypeEnum::TOKEN_HAP: {
            Security::AccessToken::HapTokenInfo hapInfo;
            if (Security::AccessToken::AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                DEV_HILOGE(SERVICE, "get hap token info fail");
                return packageName;
            }
            packageName = hapInfo.bundleName;
            break;
        }
        default: {
            DEV_HILOGW(SERVICE, "token type not match");
            break;
        }
    }
    return packageName;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
