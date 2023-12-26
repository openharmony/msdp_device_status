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

#ifndef COOPERATE_EVENTS_H
#define COOPERATE_EVENTS_H

#include <string>
#include <variant>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
enum CooperateState : size_t {
    COOPERATE_STATE_FREE = 0,
    COOPERATE_STATE_OUT,
    COOPERATE_STATE_IN,
    NUM_COOPERATE_STATES,
};

enum class CooperateEventType {
    NOOP,
    QUIT,
    UPDATE_STATE,
    ENABLE,
    DISABLE,
    START,
    STOP,
    APP_CLOSED,
    PLUG_KEYBOARD,
    UNPLUG_KEYBOARD,
    UNPLUG_POINTER,
    POINTER_MOVE,
    PREPARE_DINPUT_RESULT,
    START_DINPUT_RESULT,
    DINPUT_CLOSED,
    DSOFTBUS_CLOSED,
    INTERCEPTOR,
    SESSION_OPEND,
};

struct UpdateStateEvent {
    CooperateState current;
};

struct StartCooperateEvent {
    int32_t userData;
    std::string remoteNetworkId;
    int32_t startDeviceId;
};

struct PrepareRemoteInputResult {
    std::string remoteNetworkId;
    std::string originNetworkId;
    int32_t startDeviceId;
    bool success;
};

using StartRemoteInputResult = PrepareRemoteInputResult;

struct HotplugEvent {
    std::string dhid;
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
        PrepareRemoteInputResult,
        HotplugEvent,
        PointerMoveEvent
    > event;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_EVENTS_H