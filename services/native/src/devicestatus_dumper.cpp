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
    const std::string ARG_DUMP_HELP = "-h";
    const std::string ARG_DUMP_DEVICESTATUS_INFO = "-i";
}

int DevicestatusDumper::Dump(int fd, const std::vector<std::u16string>& args) const
{
    DEV_HILOGI(SERVICE, "Dump begin fd: %{public}d", fd);
    if (fd < 0) {
        return -1;
    }
    (void) signal(SIGPIPE, SIG_IGN); // ignore SIGPIPE crash
    UniqueFd ufd = UniqueFd(fd); // auto close
    fd = ufd.Get();
    std::vector<std::string> params;
    for (auto& arg : args) {
        params.emplace_back(Str16ToStr8(arg));
    }

    std::string dumpInfo;
    if (params.empty()) {
        ShowIllegalArgsInfo(dumpInfo);
    } else if (params[0] == ARG_DUMP_HELP) {
        ShowHelpInfo(dumpInfo);
    } else {
        DumpDevicestatusInfo(params, dumpInfo);
    }
    int ret = dprintf(fd, "%s\n", dumpInfo.c_str());
    if (ret < 0) {
        DEV_HILOGE(SERVICE, "dprintf error");
        return -1;
    }
    DEV_HILOGI(SERVICE, "Dump end");
    return 0;
}

int DevicestatusDumper::DumpDevicestatusInfo(const std::vector<std::string>& args, std::string& dumpInfo) const
{
    if (args.empty()) {
        return -1;
    }
    DEV_HILOGI(SERVICE, "DumpDevicestatusInfo start");
    return 0;
}

void DevicestatusDumper::DumpCurrentTime(int32_t fd)
{
    timespec curTime = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    CHKPV(timeinfo);
    dprintf(fd, "Current time: %02d:%02d:%02d.%03d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            int32_t { (curTime.tv_nsec / MS_NS) });
}

void DevicestatusDumper::ShowIllegalArgsInfo(std::string& dumpInfo) const
{
    dumpInfo.append("The arguments are illegal and you can enter '-h' for help.");
}

void DevicestatusDumper::ShowHelpInfo(std::string& dumpInfo) const
{
    dumpInfo.append("Usage:\n")
        .append(" -h                    ")
        .append("|help text for the tool\n")
        .append(" -i                    ")
        .append("|dump accessibility window info in the system\n");
}
}
}