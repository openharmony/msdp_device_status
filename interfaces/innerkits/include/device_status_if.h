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

#ifndef DEVICE_STATUS_IF_H
#define DEVICE_STATUS_IF_H

#include "device_status_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief Get the device status event in the system.
 *
 * @param deviceStatusTypeId Indicates the device status event type.
 * @param event Indicates the device status event
 * @return Returns <b>0</b> if the event is obtained; returns a non-zero value otherwise.
 */
int32_t GetDeviceStatusEvent(int32_t deviceStatusTypeId, DeviceStatusEvent* event);

/**
 * @brief Subscribes to device status event. The system will report the obtained device status event to the subscriber.
 * 
 * @param deviceStatusTypeId Indicates the ID of a device status type. For details, see {@link DeviceStatusTypeId}.
 * @param user Indicates the pointer to the device status subscriber that requests device status event. For details,
 * see {@link DeviceStatusUser}.
 * @return Returns <b>0</b> if the event is obtained; returns a non-zero value otherwise.
 */
int32_t SubscribeDeviceStatusEvent(int32_t deviceStatusTypeId, const DeviceStatusUser* user);

/**
 * @brief Unsubscribes to device status event
 *
 * @param deviceStatusTypeId Indicates the ID of a device status type. For details, see {@link DeviceStatusTypeId}.
 * @param user Indicates the pointer to the device status subscriber that requests device status event. For details,
 * see {@link DeviceStatusUser}.
 * @return Returns <b>0</b> if the event is obtained; returns a non-zero value otherwise.
 */
int32_t UnsubscribeDeviceStatusEvent(int32_t deviceStatusTypeId, const DeviceStatusUser *user);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* DEVICE_STATUS_IF_H */