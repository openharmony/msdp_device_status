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

#ifndef DEVICESTATUS_DUMPER_H
#define DEVICESTATUS_DUMPER_H

#include <refbase.h>
#include <singleton.h>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "accesstoken_kit.h"
#include "devicestatus_data_utils.h"
#include "idevicestatus_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
const std::string ARG_DUMP_HELP = "-h";
const std::string ARG_DUMP_DEVICESTATUS_SUBSCRIBER = "-s";
const std::string ARG_DUMP_DEVICESTATUS_CHANGES = "-l";
const std::string ARG_DUMP_DEVICESTATUS_CURRENT_STATE = "-c";
constexpr int32_t RET_NG = -1;
constexpr uint32_t MAX_DEVICE_STATUS_SIZE = 10;
constexpr uint32_t BASE_YEAR = 1900;
constexpr uint32_t BASE_MON = 1;
struct AppInfo {
    std::string startTime;
    int32_t uid = 0;
    int32_t pid = 0;
    Security::AccessToken::AccessTokenID tokenId;
    std::string packageName;
    DeviceStatusDataUtils::DeviceStatusType type;
    sptr<IdevicestatusCallback> callback;
};
struct DeviceStatusRecord {
    std::string startTime;
    DeviceStatusDataUtils::DeviceStatusData data;
};
class DeviceStatusDumper final : public RefBase,
    public Singleton<DeviceStatusDumper> {
public:
    DeviceStatusDumper() = default;
    ~DeviceStatusDumper() = default;
    void ParseCommand(int32_t fd, const std::vector<std::string> &args,
        const std::vector<DeviceStatusDataUtils::DeviceStatusData> &datas);
    void DumpHelpInfo(int32_t fd) const;
    void DumpDeviceStatusSubscriber(int32_t fd);
    void DumpDeviceStatusChanges(int32_t fd);
    void DumpDeviceStatusCurrentStatus(int32_t fd,
        const std::vector<DeviceStatusDataUtils::DeviceStatusData> &datas) const;
    void SaveAppInfo(std::shared_ptr<AppInfo> appInfo);
    void RemoveAppInfo(std::shared_ptr<AppInfo> appInfo);
    void pushDeviceStatus(const DeviceStatusDataUtils::DeviceStatusData& data);
private:
    DISALLOW_COPY_AND_MOVE(DeviceStatusDumper);
    void DumpCurrentTime(std::string &startTime) const;
    std::string GetStatusType(const DeviceStatusDataUtils::DeviceStatusType &type) const;
    std::string GetDeviceState(const DeviceStatusDataUtils::DeviceStatusValue &type) const;
    std::map<DeviceStatusDataUtils::DeviceStatusType, std::set<std::shared_ptr<AppInfo>>> \
        appInfoMap_;
    std::queue<std::shared_ptr<DeviceStatusRecord>> deviceStatusQueue_;
    std::mutex mutex_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DUMPER_H