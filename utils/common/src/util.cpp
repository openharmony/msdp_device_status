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

#include "util.h"

#include <unistd.h>

#include <string>

#include <sys/syscall.h>
#include <sys/prctl.h>
#include "securec.h"

#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "Util" };
constexpr size_t BUF_TID_SIZE = 10;
} // namespace

int32_t GetPid()
{
    return static_cast<int32_t>(getpid());
}

static std::string GetThisThreadIdOfString()
{
    thread_local std::string threadLocalId;
    if (threadLocalId.empty()) {
        long tid = syscall(SYS_gettid);
        char buf[BUF_TID_SIZE] = {};
        const int32_t ret = sprintf_s(buf, BUF_TID_SIZE, "%06d", tid);
        if (ret < 0) {
            FI_HILOGE("call sprintf_s failed, ret = %{public}d.", ret);
            return threadLocalId;
        }
        buf[BUF_TID_SIZE - 1] = '\0';
        threadLocalId = buf;
    }

    return threadLocalId;
}

uint64_t GetThisThreadId()
{
    std::string stid = GetThisThreadIdOfString();
    auto tid = std::stoull(stid);
    return tid;
}

int64_t GetMillisTime()
{
    auto timeNow = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow.time_since_epoch());
    return tmp.count();
}

void SetThreadName(const std::string &name)
{
    prctl(PR_SET_NAME, name.c_str());
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS