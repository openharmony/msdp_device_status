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
    N_COOPERATE_STATES,
};

enum class CooperateEventType {
    NOOP,
    QUIT,
    UPDATE_STATE,
    REGISTER_LISTENER,
    UNREGISTER_LISTENER,
    REGISTER_HOTAREA_LISTENER,
    UNREGISTER_HOTAREA_LISTENER,
    ENABLE,
    DISABLE,
    START,
    STOP,
    GET_COOPERATE_STATE,
    DUMP,
    APP_CLOSED,
    DDM_BOARD_ONLINE,
    DDM_BOARD_OFFLINE,
    DDP_COOPERATE_SWITCH_CHANGED,
    INPUT_PLUG_KEYBOARD,
    INPUT_UNPLUG_KEYBOARD,
    INPUT_UNPLUG_POINTER,
    INPUT_POINTER_MOVE,
    DINPUT_PREPARE_RESULT,
    DINPUT_START_RESULT,
    DINPUT_SESSION_CLOSED,
    DINPUT_STOP_RESULT,
    DSOFTBUS_SESSION_OPEND,
    DSOFTBUS_SESSION_CLOSED,
    DSOFTBUS_START_COOPERATE,
    DSOFTBUS_START_COOPERATE_RESPONSE,
    DSOFTBUS_START_COOPERATE_FINISHED,
};

struct UpdateStateEvent {
    CooperateState current;
};

struct RegisterListenerEvent {
    int32_t pid;
};

using UnregisterListenerEvent = RegisterListenerEvent;
using RegisterHotareaListenerEvent = RegisterListenerEvent;
using UnregisterHotareaListenerEvent = RegisterListenerEvent;
using EnableCooperateEvent = RegisterListenerEvent;
using DisableCooperateEvent = RegisterListenerEvent;

struct StartCooperateEvent {
    int32_t pid;
    int32_t userData;
    std::string remoteNetworkId;
    int32_t startDeviceId;
};

struct StopCooperateEvent {
    int32_t pid;
    int32_t userData;
    bool isUnchained;
};

struct GetCooperateStateEvent {
    int32_t pid;
    int32_t userData;
    std::string networkId;
};

struct DumpEvent {
    int32_t fd;
};

struct DDMBoardOnlineEvent {
    std::string networkId;
};

using DDMBoardOfflineEvent = DDMBoardOnlineEvent;

struct DDPCooperateSwitchChanged {
    std::string networkId;
    bool status;
};

struct InputHotplugEvent {
    std::string dhid;
};

struct DInputPrepareResult {
    std::string remoteNetworkId;
    std::string originNetworkId;
    int32_t startDeviceId;
    bool success;
};

struct DInputStopResult {
    std::string originNetworkId;
    int32_t startDeviceId;
    bool success;
};

using DInputStartResult = DInputPrepareResult;

struct DSoftbusSessionOpened {
    int32_t sessionId;
    int32_t result;
};

struct DSoftbusStartCooperate {
    std::string networkId;
};

struct DSoftbusStartCooperateResponse {
    std::string networkId;
    bool normal;
};

struct Coordinate {
    int32_t x;
    int32_t y;
};

struct DSoftbusStartCooperateFinished {
    std::string networkId;
    std::string startDeviceDhid;
    bool success;
    Coordinate cursorPos;
};

struct CooperateEvent {
    CooperateEvent() : type(CooperateEventType::QUIT) {}

    explicit CooperateEvent(CooperateEventType ty) : type(ty) {}

    template<typename Event>
    CooperateEvent(CooperateEventType ty, Event ev) : type(ty), event(ev) {}

    CooperateEventType type;
    std::variant<
        UpdateStateEvent,
        RegisterListenerEvent,
        StartCooperateEvent,
        StopCooperateEvent,
        GetCooperateStateEvent,
        DumpEvent,
        DDMBoardOnlineEvent,
        DDPCooperateSwitchChanged,
        DInputPrepareResult,
        DInputStopResult,
        InputHotplugEvent,
        DSoftbusStartCooperate,
        DSoftbusStartCooperateResponse,
        DSoftbusStartCooperateFinished
    > event;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_EVENTS_H
