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

#ifndef FI_LOG_H
#define FI_LOG_H

#include <cinttypes>
#include <functional>
#include <future>
#include <string>
#include <sstream>

#include "hilog/log.h"

namespace OHOS {
namespace Msdp {
inline constexpr uint32_t MSDP_DOMAIN_ID { 0xD002220 };
} // namespace Msdp
} // namespace OHOS

#ifndef FI_FUNC_FMT
#define FI_FUNC_FMT "in %{public}s, "
#endif

#ifndef FI_FUNC_INFO
#define FI_FUNC_INFO __FUNCTION__
#endif

#ifndef FI_FILE_NAME
#define FI_FILE_NAME (strrchr((__FILE__), '/') ? strrchr((__FILE__), '/') + 1 : (__FILE__))
#endif

#ifndef FI_LINE_INFO
#define FI_LINE_INFO FI_FILE_NAME, __LINE__
#endif

#define FI_HILOGD(fmt, ...) do { \
    if (HiLogIsLoggable(OHOS::Msdp::MSDP_DOMAIN_ID, LABEL.tag, LOG_DEBUG)) { \
        OHOS::HiviewDFX::HiLog::Debug(LABEL, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
    } \
} while (0)
#define FI_HILOGI(fmt, ...) do { \
    OHOS::HiviewDFX::HiLog::Info(LABEL, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGW(fmt, ...) do { \
    OHOS::HiviewDFX::HiLog::Warn(LABEL, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGE(fmt, ...) do { \
    OHOS::HiviewDFX::HiLog::Error(LABEL, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGF(fmt, ...) do { \
    OHOS::HiviewDFX::HiLog::Fatal(LABEL, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)

namespace OHOS {
namespace Msdp {
class InnerFunctionTracer {
public:
    using HilogFunc = std::function<int32_t(const char *)>;

public:
    InnerFunctionTracer(HilogFunc logfn, const char* tag, LogLevel level)
        : logfunc_ { logfn }, tag_ { tag }, level_ { level }
    {
        if (HiLogIsLoggable(OHOS::Msdp::MSDP_DOMAIN_ID, tag_, level_)) {
            if (logfunc_ != nullptr) {
                logfunc_("in %{public}s, enter");
            }
        }
    }
    ~InnerFunctionTracer()
    {
        if (HiLogIsLoggable(OHOS::Msdp::MSDP_DOMAIN_ID, tag_, level_)) {
            if (logfunc_ != nullptr) {
                logfunc_("in %{public}s, leave");
            }
        }
    }
private:
    HilogFunc logfunc_ { nullptr };
    const char* tag_ { nullptr };
    LogLevel level_ { LOG_LEVEL_MIN };
};
} // namespace Msdp
} // namespace OHOS

#define CALL_DEBUG_ENTER OHOS::Msdp::InnerFunctionTracer ___innerFuncTracer_Debug___ \
    { std::bind(&OHOS::HiviewDFX::HiLog::Debug, LABEL, std::placeholders::_1, __FUNCTION__), LABEL.tag, LOG_DEBUG }

#define CALL_INFO_TRACE OHOS::Msdp::InnerFunctionTracer ___innerFuncTracer_Info___ \
    { std::bind(&OHOS::HiviewDFX::HiLog::Info, LABEL, std::placeholders::_1, __FUNCTION__), LABEL.tag, LOG_INFO }

#define CALL_TEST_DEBUG OHOS::Msdp::InnerFunctionTracer ___innerFuncTracer_Info___ \
    { std::bind(&OHOS::HiviewDFX::HiLog::Info, LABEL, std::placeholders::_1,     \
    (test_info_ == nullptr ? "TestBody" : test_info_->name())), LABEL.tag, LOG_DEBUG }
#endif // FI_LOG_H
