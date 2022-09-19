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

#ifndef DEVICESTATUS_DUMP_H
#define DEVICESTATUS_DUMP_H

#include <vector>
#include <cinttypes>
#include <ctime>
#include <queue>

#include "singleton.h"
#include "devicestatus_hilog_wrapper.h"

namespace OHOS {
namespace Msdp {
class DevicestatusDump : public Singleton<DevicestatusDump> {
public:
    DevicestatusDump() = default;
    virtual ~DevicestatusDump() = default;
    bool DumpDevicestatusHelp(int32_t fd, const std::vector<std::u16string> &args);
    void DumpHelp(int32_t fd);

private:
    void DumpCurrentTime(int32_t fd);
};
}  // namespace Msdp
}  // namespace OHOS
#endif  // DEVICESTATUS_DUMP_H
