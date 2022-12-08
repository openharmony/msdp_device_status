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

#include <singleton.h>

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
    int32_t uid {};
    int32_t pid {};
    Security::AccessToken::AccessTokenID tokenId;
    std::string packageName;
    Type type;
    sptr<IRemoteDevStaCallback> callback { nullptr };
};
struct DeviceStatusRecord {
    std::string startTime;
    Data data;
};
class DeviceStatusDumper final : public RefBase,
    public Singleton<DeviceStatusDumper> {
public:
    DeviceStatusDumper() = default;
    ~DeviceStatusDumper() = default;
    void ParseCommand(int32_t fd, const std::vector<std::string> &args, const std::vector<Data> &datas);
    void ParseLong(int32_t fd, const std::vector<std::string> &args, const std::vector<Data> &datas);
    void ExecutDump(int32_t fd, const std::vector<Data> &datas, int32_t info);
    void DumpHelpInfo(int32_t fd) const;
    void DumpDeviceStatusSubscriber(int32_t fd);
    void DumpDeviceStatusChanges(int32_t fd);
    void DumpDeviceStatusCurrentStatus(int32_t fd, const std::vector<Data> &datas) const;
    void SaveAppInfo(Type type, sptr<IRemoteDevStaCallback> callback);
    void RemoveAppInfo(std::shared_ptr<AppInfo> appInfo);
    void PushDeviceStatus(const Data &data);
    std::string GetPackageName(Security::AccessToken::AccessTokenID tokenId);
private:
    DISALLOW_COPY_AND_MOVE(DeviceStatusDumper);
    void DumpCurrentTime(std::string &startTime) const;
    std::string GetStatusType(Type type) const;
    std::string GetDeviceState(OnChangedValue type) const;

    std::map<Type, std::set<std::shared_ptr<AppInfo>>> appInfoMap_;
    std::queue<std::shared_ptr<DeviceStatusRecord>> deviceStatusQueue_;
    std::mutex mutex_;
    std::shared_ptr<AppInfo> appInfo_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_DUMPER_H
