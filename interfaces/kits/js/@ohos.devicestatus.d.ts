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

import { AsyncCallback } from "./basic";

/**
 * 订阅用户设备状态通知
 *
 * @since 9
 * @syscap SystemCapability.Msdp.DeviceStatus
 * @import import sensor from '@ohos.DeviceStatus'
 * @permission N/A
 */
declare namespace DeviceStatus {
    /**
     * 行为识别数据。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export interface ActivityResponse {
        eventType: EventType
    }
	
    /**
     * 绝对静止的数据。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export interface StillResponse extends ActivityResponse {}
    
    /**
     * 相对静止的数据。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export interface RelativeStillResponse extends ActivityResponse {}
    
    /**
     * 水平位置的数据。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export interface VerticalPositionResponse extends ActivityResponse {}
    
    /**
     * 垂直位置的数据。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export interface HorizontalPositionResponse extends ActivityResponse {}
	
    /**
     * 行为识别类型。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export enum ActivityType {
        TYPE_STILL = "still",
        TYPE_RELATIVE_STILL = "relativeStill",
        TYPE_VERTICAL_POSITION = "verticalPosition",
        TYPE_HORIZONTAL_POSITION = "horizontalPosition"
    }

    /**
     * 事件类型。
     * @syscap SystemCapability.Msdp.DeviceStatus
     */
    export enum EventType {
        ENTER = 1,
        EXIT = 2,
        ENTER_EXIT = 3
    }

    /**	
     * 订阅绝对静止。
     *
     * @since 9
     * @param type 订阅绝对静止, {@code type: ActivityType.TYPE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function on(type: ActivityType.TYPE_STILL, eventType: EventType, reportLatencyNs: number, callback: AsyncCallback<StillResponse>): void;
	
    /**
     * 订阅相对静止。
     *
     * @since 9
     * @param type 订阅相对静止, {@code type: ActivityType.TYPE_RELATIVE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function on(type: ActivityType.TYPE_RELATIVE_STILL, eventType: EventType, reportLatencyNs: number, callback: AsyncCallback<RelativeStillResponse>): void;
	
    /**
     * 订阅水平位置。
     *
     * @since 9
     * @param type 订阅水平位置, {@code type: ActivityType.TYPE_VERTICAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function on(type: ActivityType.TYPE_VERTICAL_POSITION, eventType: EventType, reportLatencyNs: number, callback: AsyncCallback<VerticalPositionResponse>): void;
	
    /**
     * 订阅垂直位置。
     *
     * @since 9
     * @param type 订阅垂直位置, {@code type: ActivityType.TYPE_HORIZONTAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function on(type: ActivityType.TYPE_HORIZONTAL_POSITION, eventType: EventType, reportLatencyNs: number, callback: AsyncCallback<HorizontalPositionResponse>): void;
     
    /**
     * 查询是否绝对静止。
     *
     * @since 9
     * @param type 查询是否绝对静止, {@code type: ActivityType.TYPE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function once(type: ActivityType.TYPE_STILL, callback: AsyncCallback<StillResponse>): void;
	
    /**
     * 查询是否相对静止。
     *
     * @since 9
     * @param type 查询是否相对静止, {@code type: ActivityType.TYPE_RELATIVE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function once(type: ActivityType.TYPE_RELATIVE_STILL, callback: AsyncCallback<RelativeStillResponse>): void;
	
    /**
     * 查询是否水平位置。
     *
     * @since 9
     * @param type 查询是否水平位置, {@code type: ActivityType.TYPE_VERTICAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function once(type: ActivityType.TYPE_VERTICAL_POSITION, callback: AsyncCallback<VerticalPositionResponse>): void;
	
    /**
     * 查询是否垂直位置。
     *
     * @since 9
     * @param type 查询是否垂直位置, {@code type: ActivityType.TYPE_HORIZONTAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function once(type: ActivityType.TYPE_HORIZONTAL_POSITION, callback: AsyncCallback<HorizontalPositionResponse>): void;
	
    /**
     * 取消订阅绝对静止。
     *
     * @since 9
     * @param type 查询是否绝对静止, {@code type: ActivityType.TYPE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function off(type: ActivityType.TYPE_STILL, eventType: EventType, callback?: AsyncCallback<void>): void;
    
    /**
     * 取消订阅相对静止。
     *
     * @since 9
     * @param type 查询是否相对静止, {@code type: ActivityType.TYPE_RELATIVE_STILL}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function off(type: ActivityType.TYPE_RELATIVE_STILL, eventType: EventType, callback?: AsyncCallback<void>): void;
    
    /**
     * 取消订阅水平位置。
     *
     * @since 9
     * @param type 查询是否水平位置, {@code type: ActivityType.TYPE_VERTICAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function off(type: ActivityType.TYPE_VERTICAL_POSITION, eventType: EventType, callback?: AsyncCallback<void>): void;
    
    /**
     * 取消订阅垂直位置。
     *
     * @since 9
     * @param type 查询是否垂直位置, {@code type: ActivityType.TYPE_HORIZONTAL_POSITION}.
     * @param eventType enter and exit event.
     * @param reportLatencyNs report event latency.
     * @param callback callback function, receive reported data.
     */
    function off(type: ActivityType.TYPE_HORIZONTAL_POSITION, eventType: EventType, callback?: AsyncCallback<void>): void;
}
export default DeviceStatus;
