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

#ifndef DEVICESTATUS_HILOG_WRAPPER_H
#define DEVICESTATUS_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace Msdp {
#define __FILENAME__            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

#ifdef DEVICESTATUS_HILOGF
#undef DEVICESTATUS_HILOGF
#endif

#ifdef DEVICESTATUS_HILOGE
#undef DEVICESTATUS_HILOGE
#endif

#ifdef DEVICESTATUS_HILOGW
#undef DEVICESTATUS_HILOGW
#endif

#ifdef DEVICESTATUS_HILOGI
#undef DEVICESTATUS_HILOGI
#endif

#ifdef DEVICESTATUS_HILOGD
#undef DEVICESTATUS_HILOGD
#endif

// param of log interface, such as DEVICESTATUS_HILOGF.
enum DevicestatusSubModule {
    DEVICESTATUS_MODULE_INNERKIT = 0,
    DEVICESTATUS_MODULE_SERVICE,
    DEVICESTATUS_MODULE_JS_NAPI,
    DEVICESTATUS_MODULE_COMMON,
    DEVICESTATUS_MODULE_BUTT,
};

// 0xD002900: subsystem:Msdp module:Devicestatus, 8 bits reserved.
static constexpr unsigned int BASE_MSDP_DOMAIN_ID = 0xD002900;

enum DevicestatusDomainId {
    DEVICESTATUS_INNERKIT_DOMAIN = BASE_MSDP_DOMAIN_ID + DEVICESTATUS_MODULE_INNERKIT,
    DEVICESTATUS_SERVICE_DOMAIN,
    DEVICESTATUS_JS_NAPI,
    DEVICESTATUS_COMMON,
    DEVICESTATUS_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel DEVICESTATUS_LABEL[DEVICESTATUS_MODULE_BUTT] = {
    {LOG_CORE, DEVICESTATUS_INNERKIT_DOMAIN, "DevicestatusClient"},
    {LOG_CORE, DEVICESTATUS_SERVICE_DOMAIN, "DevicestatusService"},
    {LOG_CORE, DEVICESTATUS_JS_NAPI, "DevicestatusJsNapi"},
    {LOG_CORE, DEVICESTATUS_COMMON, "DevicestatusCommon"},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than DEVICESTATUS_MODULE_BUTT.
#define DEVICESTATUS_HILOGF(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(DEVICESTATUS_LABEL[module], __FORMATED(__VA_ARGS__))
#define DEVICESTATUS_HILOGE(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(DEVICESTATUS_LABEL[module], __FORMATED(__VA_ARGS__))
#define DEVICESTATUS_HILOGW(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(DEVICESTATUS_LABEL[module], __FORMATED(__VA_ARGS__))
#define DEVICESTATUS_HILOGI(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(DEVICESTATUS_LABEL[module], __FORMATED(__VA_ARGS__))
#define DEVICESTATUS_HILOGD(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(DEVICESTATUS_LABEL[module], __FORMATED(__VA_ARGS__))
} // namespace Msdp
} // namespace OHOS

#else

#define DEVICESTATUS_HILOGF(...)
#define DEVICESTATUS_HILOGE(...)
#define DEVICESTATUS_HILOGW(...)
#define DEVICESTATUS_HILOGI(...)
#define DEVICESTATUS_HILOGD(...)

#endif // CONFIG_HILOG

#endif // DEVICESTATUS_HILOG_WRAPPER_H
