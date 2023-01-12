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
namespace DeviceStatus {
#define DEV_RET_IF_NULL_WITH_RET(cond, retval) if (cond) {return (retval);}
#define DEV_RET_IF_NULL(cond) if (cond) {return;}
#define DEV_RET_IF_NULL_WITH_LOG(cond, loginfo) \
    do { \
        if (cond) { \
            DEV_HILOGE(COMMON, "%{public}s "#loginfo" ", __func__); \
            return; \
        } \
    } while (0) \

#define WRITEBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).WriteBool(data)) { \
            DEV_HILOGE(COMMON, "WriteBool "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt32(data)) { \
            DEV_HILOGE(COMMON, "WriteInt32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEINT64(parcel, data, ...) \
    do { \
        if (!(parcel).WriteInt64(data)) { \
            DEV_HILOGE(COMMON, "WriteInt64 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUint32(data)) { \
            DEV_HILOGE(COMMON, "WriteUint32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEDOUBLE(parcel, data, ...) \
    do { \
        if (!(parcel).WriteDouble(data)) { \
            DEV_HILOGE(COMMON, "WriteDouble "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITESTRING(parcel, data, ...) \
    do { \
        if (!(parcel).WriteString(data)) { \
            DEV_HILOGE(COMMON, "WriteString "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITESTRING16(parcel, data, ...) \
    do { \
        if (!(parcel).WriteString16(data)) { \
            DEV_HILOGE(COMMON, "WriteString16 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define WRITEREMOTEOBJECT(parcel, data, ...) \
    do { \
        if (!(parcel).WriteRemoteObject(data)) { \
            DEV_HILOGE(COMMON, "WriteRemoteObject "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)
#define WRITEUInt8Vector(parcel, data, ...) \
    do { \
        if (!(parcel).WriteUInt8Vector(data)) { \
                DEV_HILOGE(COMMON, "WriteUInt8Vector "#data" failed"); \
                return __VA_ARGS__; \
            } \
    } while (0)
#define READBOOL(parcel, data, ...) \
    do { \
        if (!(parcel).ReadBool(data)) { \
            DEV_HILOGE(COMMON, "ReadBool "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt32(data)) { \
            DEV_HILOGE(COMMON, "ReadInt32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READINT64(parcel, data, ...) \
    do { \
        if (!(parcel).ReadInt64(data)) { \
            DEV_HILOGE(COMMON, "ReadInt64 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READUINT32(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUint32(data)) { \
            DEV_HILOGE(COMMON, "ReadUint32 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READDOUBLE(parcel, data, ...) \
    do { \
        if (!(parcel).ReadDouble(data)) { \
            DEV_HILOGE(COMMON, "ReadDouble "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READSTRING(parcel, data, ...) \
    do { \
        if (!(parcel).ReadString(data)) { \
            DEV_HILOGE(COMMON, "ReadString "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

#define READUInt8Vector(parcel, data, ...) \
    do { \
        if (!(parcel).ReadUInt8Vector(data)) { \
                DEV_HILOGE(COMMON, "ReadUInt8Vector "#data" failed"); \
                return __VA_ARGS__; \
            } \
    } while (0)
#define READSTRING16(parcel, data, ...) \
    do { \
        if (!(parcel).ReadString16(data)) { \
            DEV_HILOGE(COMMON, "ReadString16 "#data" failed"); \
            return __VA_ARGS__; \
        } \
    } while (0)

template<typename E>
constexpr auto DeviceStatusToUnderlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_COMMON_H
