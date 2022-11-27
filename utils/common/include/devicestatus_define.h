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

#ifndef DEVICESTATUS_DEFINE_H
#define DEVICESTATUS_DEFINE_H

#include "devicestatus_hilog_wrapper.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

#define CALL_INFO_TRACE
#define CALL_DEBUG_ENTER
#define ERROR_NULL_POINTER (-1)
#define MMI_DINPUT_PKG_NAME "ohos.multimodalinput.dinput"

#define CHKPL(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGW(module, "CHKPL(%{public}s) is null, do nothing", #cond); \
        } \
    } while (0)

#define CHKPV(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGE(module, "CHKPV(%{public}s) is null", #cond); \
            return; \
        } \
    } while (0)

#define CHKPF(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGE(module, "CHKPF(%{public}s) is null", #cond); \
            return false; \
        } \
    } while (0)

#define CHKPS(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGE(module, "CHKPS(%{public}s) is null", #cond); \
            return ""; \
        } \
    } while (0)

#define CHKPC(cond, module) \
    { \
        if ((cond) == nullptr) { \
            DEV_HILOGW(module, "CHKPC(%{public}s) is null, skip then continue", #cond); \
            continue; \
        } \
    }

#define CHKPB(cond, module) \
    { \
        if ((cond) == nullptr) { \
            DEV_HILOGW(module, "CHKPB(%{public}s) is null, skip then break", #cond); \
            break; \
        } \
    }

#define CHKPR(cond, module, r) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGE(module, "CHKPR(%{public}s) is null, return value is %{public}d", #cond, r); \
            return r; \
        } \
    } while (0)

#define CHKPP(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGE(module, "CHKPP(%{public}s) is null, return value is null", #cond); \
            return nullptr; \
        } \
    } while (0)

#define CHKPO(cond, module) \
    do { \
        if ((cond) == nullptr) { \
            DEV_HILOGW(module, "%{public}s, (%{public}d), CHKPO(%{public}s) is null, return object is null", \
                __FILE__, __LINE__, #cond); \
            return {}; \
        } \
    } while (0)

#define CK(cond, module, ec) \
    do { \
        if (!(cond)) { \
            DEV_HILOGE(module, "CK(%{public}s), errCode:%{public}d", #cond, ec); \
        } \
    } while (0)

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_COMMON_H