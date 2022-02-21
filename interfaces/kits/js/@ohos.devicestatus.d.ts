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

declare namespace devicestatus {
    export enum DevicestatusType {
        TYPE_HIGH_STILL = 0,
        TYPE_FINE_STILL = 1,
        TYPE_CAR_BLUETOOTH = 2
    }

    export enum DevicestatusValue {
        VALUE_ENTER = 0,
        VALUE_EXIT
    }

    export interface DevicestatusResponse {
        devicestatusValue: DevicestatusValue
    }

    export interface HighStillResponse extends DevicestatusResponse {}
    export interface FineStillResponse extends DevicestatusResponse {}
    export interface CarBluetoothResponse extends DevicestatusResponse {}

    function on(type: DevicestatusType.TYPE_HIGH_STILL, callback: AsyncCallback<HighStillResponse>): void;
    function once(type: DevicestatusType.TYPE_HIGH_STILL, callback: AsyncCallback<HighStillResponse>): void;
    function off(type: DevicestatusType.TYPE_HIGH_STILL, callback: AsyncCallback<void>): void;

    function on(type: DevicestatusType.TYPE_FINE_STILL, callback: AsyncCallback<FineStillResponse>): void;
    function once(type: DevicestatusType.TYPE_FINE_STILL, callback: AsyncCallback<FineStillResponse>): void;
    function off(type: DevicestatusType.TYPE_FINE_STILL, callback: AsyncCallback<void>): void;

    function on(type: DevicestatusType.TYPE_CAR_BLUETOOTH, callback: AsyncCallback<CarBluetoothResponse>): void;
    function once(type: DevicestatusType.TYPE_CAR_BLUETOOTH, callback: AsyncCallback<CarBluetoothResponse>): void;
    function off(type: DevicestatusType.TYPE_CAR_BLUETOOTH, callback: AsyncCallback<void>): void;
}
export default devicestatus;