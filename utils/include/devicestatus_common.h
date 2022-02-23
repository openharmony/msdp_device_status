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

#ifndef DEVICESTATUS_COMMON_H
#define DEVICESTATUS_COMMON_H

#include <cstdint>
#include <type_traits>

#include "devicestatus_hilog_wrapper.h"
#include "devicestatus_errors.h"

namespace OHOS {
namespace Msdp {
#define DEVICESTATUS_RETURN_IF_WITH_RET(cond, retval) if (cond) {return (retval);}
#define DEVICESTATUS_RETURN_IF(cond) if (cond) {return;}
#define DEVICESTATUS_RETURN_IF_WITH_LOG(cond, loginfo)                                  \
    do {                                                                                \
        if (cond) {                                                                     \
            DEV_HILOGE(COMMON, "%{public}s "#loginfo" ", __func__); \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define DEVICESTATUS_READ_PARCEL_NO_RET(parcel, type, out)                              \
    do {                                                                                \
        if (!(parcel).Read##type(out)) {                                                \
            DEV_HILOGE(COMMON, "%{public}s read "#out" failed", __func__); \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define DEVICESTATUS_WRITE_PARCEL_NO_RET(parcel, type, data)                            \
    do {                                                                                \
        if (!(parcel).Write##type(data)) {                                              \
            DEV_HILOGE(COMMON, "%{public}s write "#data" failed", __func__); \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define DEVICESTATUS_READ_PARCEL_WITH_RET(parcel, type, out, retval)                    \
    do {                                                                                \
        if (!(parcel).Read##type(out)) {                                                \
            DEV_HILOGE(COMMON, "%{public}s read "#out" failed", __func__); \
            return (retval);                                                            \
        }                                                                               \
    } while (0)                                                                         \

#define DEVICESTATUS_WRITE_PARCEL_WITH_RET(parcel, type, data, retval)                  \
    do {                                                                                \
        if (!(parcel).Write##type(data)) {                                              \
            DEV_HILOGE(COMMON, "%{public}s write "#data" failed", __func__); \
            return (retval);                                                            \
        }                                                                               \
    } while (0)

template<typename E>
constexpr auto DevicestatusToUnderlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_COMMON_H
