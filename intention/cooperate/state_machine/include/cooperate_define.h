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

#ifndef COOPERATE_DEFINE_H
#define COOPERATE_DEFINE_H

#include <string>
#include <tuple>
#include <variant>

#include "channel.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class CooperateEventType {
    NOOP,
    QUIT,
    UPDATE_STATE,
    ENABLE,
    DISABLE,
    START,
    STOP,
    APP_CLOSED,
    UNPLUG_POINTER,
    PLUG_KEYBOARD,
    POINTER_MOVE,
    PREPARE_DINPUT_RESULT,
    START_DINPUT_RESULT,
    DINPUT_CLOSED,
    DSOFTBUS_CLOSED,
    INTERCEPTOR,
    SESSION_OPEND,
};

struct UpdateStateEvent {
    size_t current;
};

struct StartCooperateEvent {
    int32_t userData;
    std::string remoteNetworkId;
    int32_t startDeviceId;
};

struct StartRemoteInputResult {
    std::string source;
    std::string sink;
    int32_t startDeviceId;
    bool success;
};

struct PointerMoveEvent {
    int32_t deviceId;
};

struct SessionOpened {
    int32_t sessionId;
    int32_t result;
};

struct CooperateEvent {
    CooperateEvent() : type(CooperateEventType::QUIT) {}

    explicit CooperateEvent(CooperateEventType ty) : type(ty) {}

    template<typename Event>
    CooperateEvent(CooperateEventType ty, Event ev) : type(ty), event(ev) {}

    CooperateEventType type;
    std::variant<
        UpdateStateEvent,
        StartCooperateEvent,
        StartRemoteInputResult,
        PointerMoveEvent
    > event;
};

std::ostream& operator<<(std::ostream &oss, CooperateEvent &event)
{
    oss << "CooperateEvent(id:";
    switch (event.type) {
        case CooperateEventType::POINTER_MOVE : {
            PointerMoveEvent e = std::get<PointerMoveEvent>(event.event);
            oss << "pointer move, pointer:" << e.deviceId;
            break;
        }
        case CooperateEventType::START: {
            StartCooperateEvent e = std::get<StartCooperateEvent>(event.event);
            oss << "start cooperate, remote:" << e.remoteNetworkId << ", startDeviceId:" << e.startDeviceId;
            break;
        }
        case CooperateEventType::PREPARE_DINPUT_RESULT : {
            StartRemoteInputResult e = std::get<StartRemoteInputResult>(event.event);
            oss << "prepare remote input result, source:" << e.source << ", sink:" << e.sink
                << ", startDeviceId:" << e.startDeviceId << ", isSuccess:" << std::boolalpha << e.success;
            break;
        }
        case CooperateEventType::START_DINPUT_RESULT : {
            StartRemoteInputResult e = std::get<StartRemoteInputResult>(event.event);
            oss << "start remote input result, source:" << e.source << ", sink:" << e.sink
                << ", startDeviceId:" << e.startDeviceId << ", isSuccess:" << std::boolalpha << e.success;
            break;
        }
        default : {
            oss << static_cast<int32_t>(event.type);
            break;
        }
    }
    oss << ")";
    return oss;
}

class DeviceManager {
public:
    std::string GetDhid(int32_t deviceId);
};

std::string DeviceManager::GetDhid(int32_t deviceId)
{
    return std::to_string(deviceId);
}

class Context {
public:
    DeviceManager devMgr_;
    Channel<CooperateEvent>::Sender sender;
    std::string cooperated_;
    std::string startDeviceId_;
    bool isUnchain_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_DEFINE_H