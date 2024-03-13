/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "devicestatus_hisysevent.h"
#include "hisysevent.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "DeviceStatusHiSysEvent"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

template<typename... Types>
static void WriteEvent(const std::string &packageName, Types ... args)
{
    int32_t ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::MSDP, packageName,
        HiviewDFX::HiSysEvent::EventType::STATISTIC, args...);
    if (ret != 0) {
        FI_HILOGE("Write event failed:%{public}s", packageName.c_str());
    }
}

void WriteSubscribeHiSysEvent(int32_t uid, const std::string &packageName, int32_t type)
{
    WriteEvent("SUBSCRIBE", "UID", uid, "PACKAGE_NAME", packageName, "TYPE", type);
}

void WriteUnSubscribeHiSysEvent(int32_t uid, const std::string &packageName, int32_t type)
{
    WriteEvent("UNSUBSCRIBE", "UID", uid, "PACKAGE_NAME", packageName, "TYPE", type);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
