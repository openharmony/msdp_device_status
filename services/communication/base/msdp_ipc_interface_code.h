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

#ifndef MSDP_IPC_INTERFACE_CODE_H
#define MSDP_IPC_INTERFACE_CODE_H

/* SAID:2902 */
namespace OHOS {
namespace Msdp {
enum class DeviceInterfaceCode {
    DEVICESTATUS_SUBSCRIBE = 0,
    DEVICESTATUS_UNSUBSCRIBE,
    DEVICESTATUS_GETCACHE,
    REGISTER_COORDINATION_MONITOR = 10,
    UNREGISTER_COORDINATION_MONITOR,
    PREPARE_COORDINATION,
    UNPREPARE_COORDINATION,
    START_COORDINATION,
    STOP_COORDINATION,
    GET_COORDINATION_STATE,
    GET_COORDINATION_STATE_SYNC,
    REGISTER_COOPERATE_MONITOR,
    UNREGISTER_COOPERATE_MONITOR,
    PREPARE_COOPERATE,
    UNPREPARE_COOPERATE,
    START_COOPERATE,
    STOP_COOPERATE,
    GET_COOPERATE_STATE,
    UPDATED_DRAG_STYLE = 30,
    START_DRAG,
    STOP_DRAG,
    GET_DRAG_TARGET_PID,
    GET_DRAG_TARGET_UDKEY,
    REGISTER_DRAG_MONITOR,
    UNREGISTER_DRAG_MONITOR,
    SET_DRAG_WINDOW_VISIBLE,
    GET_SHADOW_OFFSET,
    UPDATE_SHADOW_PIC,
    GET_DRAG_DATA,
    REGISTER_SUBSCRIPT_MONITOR,
    UNREGISTER_SUBSCRIPT_MONITOR,
    GET_DRAG_STATE,
    GET_DRAG_SUMMARY,
    ENTER_TEXT_EDITOR_AREA,
    GET_DRAG_ACTION,
    GET_DRAG_EXTRAINFO,
    UPDATE_PREVIEW_STYLE,
    UPDATE_PREVIEW_STYLE_WITH_ANIMATION,
    ADD_PRIVILEGE,
    ALLOC_SOCKET_FD = 60,
    ADD_HOT_AREA_MONITOR = 70,
    REMOVE_HOT_AREA_MONITOR,
};
} // namespace Msdp
} // namespace OHOS
#endif // MSDP_IPC_INTERFACE_CODE_H
