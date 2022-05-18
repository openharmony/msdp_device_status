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
#include <string>

namespace OHOS {
namespace Msdp {

class DevicestatusDumper : public RefBase {
public:
    DevicestatusDumper() {}
    int Dump(int fd, const std::vector<std::u16string>& args) const;

private:
    int DumpDevicestatusInfo(const std::vector<std::string>& args, std::string& dumpInfo) const;
    void DumpCurrentTime(int32_t fd);
    void ShowHelpInfo(std::string& dumpInfo) const;
    void ShowIllegalArgsInfo(std::string& dumpInfo) const;
};
}
}
#endif // DEVICESTATUS_DUMPER_H