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

#ifndef DEVICE_STATUS_TYPE_H
#define DEVICE_STATUS_TYPE_H

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define NAME_MAX_LEN 48

/**
 * @brief Enumerates device status types.
 *
 * @since 5
 */
typedef enum DeviceStatusTypeId {
    DEVICE_STATUS_TYPE_ID_NONE = 0,                     /**< None */
    DEVICE_STATUS_TYPE_ID_STILL = 1,                    /**< Still */
    DEVICE_STATUS_TYPE_ID_HORIZONTAL_POSITION = 2,          /**< Horizontal */
    DEVICE_STATUS_TYPE_ID_VERTICAL_POSITION = 3,              /**< Vertical */
    DEVICE_STATUS_ID_MAX = 30,                         /**< Maximum number of Device status type IDs*/
}DeviceStatusTypeId;

/**
 * @brief define the reported status
 *
 */
typedef enum DeviceStatusReportResult {
    STATUS_INVALID = -1,
    STATUS_EXIT = 0,
    STATUS_ENTER = 1,
}DeviceStatusReportResult;

/**
 * @brief Defines the reported data.
 *
 */
typedef struct DeviceStatusEvent {
    int32_t deviceStatusTypeId;  /**< Device status type identifier */
    int32_t result;     /**< Device status result  {@link DeviceStatusReportResult}.*/
} DeviceStatusEvent;

/**
 * @brief Defines the callback for data reporting by the device status.
 *
 */
typedef void (*RecordDeviceStatusCallback)(DeviceStatusEvent *event);

/**
 * @brief Defines information about the device status subscriber.
 *
 */
typedef struct DeviceStatusUser {
    char name[NAME_MAX_LEN];  /**< Name of the device status subscriber */
    RecordDeviceStatusCallback callback;   /**< Callback for reporting device status result*/
} DeviceStatusUser;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif // DEVICE_STATUS_TYPE_H