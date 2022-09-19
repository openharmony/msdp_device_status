/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "devicestatus_dump.h"

namespace OHOS {
namespace Msdp {
namespace {
constexpr uint32_t MS_NS = 1000000;
}

bool DevicestatusDump::DumpDevicestatusHelp(int32_t fd, const std::vector<std::u16string> &args)
{
    if ((args.empty()) || (args[0].compare(u"-h") != 0)) {
        DEV_HILOGD(SERVICE, "args cannot be empty or invalid");
        return false;
    }
    DumpHelp(fd);
    return true;
}

void DevicestatusDump::DumpHelp(int32_t fd)
{
    dprintf(fd, "Usage:\n");
    dprintf(fd, "      -h: devicestatus dump help\n");
}

void DevicestatusDump::DumpCurrentTime(int32_t fd)
{
    timespec curTime = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &curTime);
    struct tm *timeinfo = localtime(&(curTime.tv_sec));
    if (timeinfo == nullptr) {
        DEV_HILOGE(SERVICE, "timeinfo cannot be null");
        return;
    }
    dprintf(fd, "Current time: %02d:%02d:%02d.%03d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec,
            int32_t { (curTime.tv_nsec / MS_NS) });
}
}  // namespace Msdp
}  // namespace OHOS
