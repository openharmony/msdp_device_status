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

#ifdef LOG_DOMAIN
#undef LOG_DOMAIN
#endif
#define LOG_DOMAIN 0XD002220

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
    if (HiLogIsLoggable(LOG_DOMAIN, LOG_TAG, LOG_DEBUG)) { \
        HILOG_DEBUG(LOG_CORE, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
    } \
} while (0)
#define FI_HILOGI(fmt, ...) do { \
    HILOG_INFO(LOG_CORE, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGW(fmt, ...) do { \
    HILOG_WARN(LOG_CORE, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGE(fmt, ...) do { \
    HILOG_ERROR(LOG_CORE, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)
#define FI_HILOGF(fmt, ...) do { \
    HILOG_FATAL(LOG_CORE, FI_FUNC_FMT fmt, FI_FUNC_INFO, ##__VA_ARGS__); \
} while (0)

namespace OHOS {
namespace Msdp {
class InnerFunctionDebugTracer {
public:
    InnerFunctionDebugTracer(const char* func)
        : func_ { func }
    {
        HILOG_DEBUG(LOG_CORE, "in %{public}s, enter", func_);
    }
    ~InnerFunctionDebugTracer()
    {
        HILOG_DEBUG(LOG_CORE, "in %{public}s, leave", func_);
    }
private:
    const char* func_ { nullptr };
};

class InnerFunctionInfoTracer {
public:
    InnerFunctionInfoTracer(const char* func)
        : func_ { func }
    {
        HILOG_INFO(LOG_CORE, "in %{public}s, enter", func_);
    }
    ~InnerFunctionInfoTracer()
    {
        HILOG_INFO(LOG_CORE, "in %{public}s, leave", func_);
    }
private:
    const char* func_ { nullptr };
};
} // namespace Msdp
} // namespace OHOS

#define CALL_DEBUG_ENTER InnerFunctionDebugTracer __innerFuncTracer_Debug___ { __FUNCTION__ }
#define CALL_INFO_TRACE InnerFunctionInfoTracer ___innerFuncTracer_Info___ { __FUNCTION__ }
#define CALL_TEST_DEBUG InnerFunctionInfoTracer ___innerFuncTracer_Info___ \
    { test_info_ == nullptr ? "TestBody" : test_info_->name() }
#endif // FI_LOG_H
