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

#ifndef I_PLUGIN_H
#define I_PLUGIN_H

#include "parcel.h"

#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IContext;
enum CommonAction : uint32_t {
    ENABLE,
    DISABLE,
    START,
    STOP,
    ADD_WATCH,
    REMOVE_WATCH,
    SET_PARAM,
    GET_PARAM,
    CONTROL
};

enum class Intention : uint32_t {
    DRAG,
    COOPERATE,
};

inline constexpr uint32_t PARAMBITS { 20U };
inline constexpr uint32_t PARAMMASK { (uint32_t(1U) << PARAMBITS) - uint32_t(1U) };
inline constexpr uint32_t INTENTIONSHIFT { PARAMBITS };
inline constexpr uint32_t INTENTIONBITS { 8U };
inline constexpr uint32_t INTENTIONMASK { (uint32_t(1U) << INTENTIONBITS) - uint32_t(1U) };
inline constexpr uint32_t ACTIONSHIFT { INTENTIONSHIFT + INTENTIONBITS };
inline constexpr uint32_t ACTIONBITS { 4U };
inline constexpr uint32_t ACTIONMASK { (uint32_t(1U) << ACTIONBITS) - uint32_t(1U) };

constexpr uint32_t PARAMID(uint32_t action, uint32_t intention, uint32_t param)
{
    return (
        ((action & ACTIONMASK) << ACTIONSHIFT) |
        ((intention & INTENTIONMASK) << INTENTIONSHIFT) |
        (param & PARAMMASK)
    );
}

constexpr uint32_t GACTION(uint32_t id)
{
    return ((id >> ACTIONSHIFT) & ACTIONMASK);
}

constexpr uint32_t GINTENTION(uint32_t id)
{
    return ((id >> INTENTIONSHIFT) & INTENTIONMASK);
}

constexpr uint32_t GPARAM(uint32_t id)
{
    return (id & PARAMMASK);
}

class ParamBase : public Parcelable {
public:
    virtual bool Unmarshalling(Parcel &parcel) = 0;
};

struct CallingContext {
    uint32_t tokenId { 0 };
    int32_t pid { -1 };
    int32_t tid { -1 };
    SessionPtr session { nullptr };
};

class IPlugin {
public:
    virtual ~IPlugin() = default;

    virtual int32_t Enable(CallingContext &context, Parcel &data, Parcel &reply) = 0;
    virtual int32_t Disable(CallingContext &context, Parcel &data, Parcel &reply) = 0;
    virtual int32_t Start(CallingContext &context, Parcel &data, Parcel &reply) = 0;
    virtual int32_t Stop(CallingContext &context, Parcel &data, Parcel &reply) = 0;
    virtual int32_t AddWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply) = 0;
    virtual int32_t RemoveWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply) = 0;
    virtual int32_t SetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply) = 0;
    virtual int32_t GetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply) = 0;
    virtual int32_t Control(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply) = 0;
};

using IntentionCreateInstance = IPlugin* (*)(IContext *context);
using IntentionDestroyInstance = void (*)(IPlugin *);
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_PLUGIN_H
