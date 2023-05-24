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

#ifndef DEVICE_STATUS_HILOG_WRAPPER_H
#define DEVICE_STATUS_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG

#include "hilog/log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
#define FILENAME            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define FORMATTED(fmt, ...)    "[%{public}s] %{public}s# " fmt, FILENAME, __FUNCTION__, ##__VA_ARGS__

#ifdef DEV_HILOGF
#undef DEV_HILOGF
#endif

#ifdef DEV_HILOGE
#undef DEV_HILOGE
#endif

#ifdef DEV_HILOGW
#undef DEV_HILOGW
#endif

#ifdef DEV_HILOGI
#undef DEV_HILOGI
#endif

#ifdef DEV_HILOGD
#undef DEV_HILOGD
#endif

#ifndef RET_OK
    #define RET_OK (0)
#endif

#ifndef RET_ERR
    #define RET_ERR (-1)
#endif

// param of log interface, such as DEV_HILOGF.
enum DeviceStatusSubModule {
    INNERKIT = 0,
    SERVICE,
    JS_NAPI,
    COMMON,
    BUTT
};

// 0xD002220: subsystem:Msdp module:DeviceStatus, 8 bits reserved.
static constexpr unsigned int BASE_MSDP_DOMAIN_ID = 0xD002220;

enum DeviceStatusDomainId {
    DEVICESTATUS_INNERKIT_DOMAIN = BASE_MSDP_DOMAIN_ID + INNERKIT,
    DEVICESTATUS_SERVICE_DOMAIN,
    DEVICESTATUS_JS_NAPI,
    DEVICESTATUS_COMMON,
    DEVICESTATUS_BUTT
};

static constexpr OHOS::HiviewDFX::HiLogLabel DEVICESTATUS_LABEL[BUTT] = {
    {LOG_CORE, DEVICESTATUS_INNERKIT_DOMAIN, "DeviceStatusClient"},
    {LOG_CORE, DEVICESTATUS_SERVICE_DOMAIN, "DeviceStatusService"},
    {LOG_CORE, DEVICESTATUS_JS_NAPI, "DeviceStatusJsNapi"},
    {LOG_CORE, DEVICESTATUS_COMMON, "DeviceStatusCommon"}
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than BUTT.
#define DEV_HILOGF(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(DEVICESTATUS_LABEL[module], FORMATTED(__VA_ARGS__))
#define DEV_HILOGE(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(DEVICESTATUS_LABEL[module], FORMATTED(__VA_ARGS__))
#define DEV_HILOGW(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(DEVICESTATUS_LABEL[module], FORMATTED(__VA_ARGS__))
#define DEV_HILOGI(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(DEVICESTATUS_LABEL[module], FORMATTED(__VA_ARGS__))
#define DEV_HILOGD(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(DEVICESTATUS_LABEL[module], FORMATTED(__VA_ARGS__))
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#else

#define DEV_HILOGF(...)
#define DEV_HILOGE(...)
#define DEV_HILOGW(...)
#define DEV_HILOGI(...)
#define DEV_HILOGD(...)

#endif // CONFIG_HILOG

#endif // DEVICE_STATUS_HILOG_WRAPPER_H
