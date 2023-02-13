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
const std::string ARG_DUMP_HELP = "-h";
const std::string ARG_DUMP_DEVICESTATUS_SUBSCRIBER = "-s";
const std::string ARG_DUMP_DEVICESTATUS_CHANGES = "-l";
const std::string ARG_DUMP_DEVICESTATUS_CURRENT_STATE = "-c";
constexpr uint32_t MAX_DEVICE_STATUS_SIZE = 10;
constexpr uint32_t BASE_YEAR = 1900;
constexpr uint32_t BASE_MON = 1;
struct AppInfo {
    std::string startTime;
    int32_t uid = 0;
    int32_t pid = 0;
    Security::AccessToken::AccessTokenID tokenId;
    std::string packageName;
    DevicestatusDataUtils::DevicestatusType type;
    sptr<IdevicestatusCallback> callback;
};
struct DeviceStatusRecord {
    std::string startTime;
    DevicestatusDataUtils::DevicestatusData data;
};
class DevicestatusDumper final : public RefBase,
    public Singleton<DevicestatusDumper> {
public:
    DevicestatusDumper() = default;
    ~DevicestatusDumper() = default;
    void ParseCommand(int32_t fd, const std::vector<std::string> &args,
        const std::vector<DevicestatusDataUtils::DevicestatusData> &datas);
    void DumpHelpInfo(int32_t fd) const;
    void DumpDevicestatusSubscriber(int32_t fd);
    void DumpDevicestatusChanges(int32_t fd);
    void DumpDevicestatusCurrentStatus(int32_t fd,
        const std::vector<DevicestatusDataUtils::DevicestatusData> &datas) const;
    void SaveAppInfo(std::shared_ptr<AppInfo> appInfo);
    void RemoveAppInfo(std::shared_ptr<AppInfo> appInfo);
    void pushDeviceStatus(const DevicestatusDataUtils::DevicestatusData& data);
private:
    DISALLOW_COPY_AND_MOVE(DevicestatusDumper);
    void DumpCurrentTime(std::string &startTime) const;
    std::string GetStatusType(const DevicestatusDataUtils::DevicestatusType &type) const;
    std::string GetDeviceState(const DevicestatusDataUtils::DevicestatusValue &type) const;
    std::map<DevicestatusDataUtils::DevicestatusType, std::set<std::shared_ptr<AppInfo>>> \
        appInfoMap_;
    std::queue<std::shared_ptr<DeviceStatusRecord>> deviceStatusQueue_;
    std::mutex mutex_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DUMPER_H