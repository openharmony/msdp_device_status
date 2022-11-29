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

#ifndef DEVICESTATUS_ERRORS_H
#define DEVICESTATUS_ERRORS_H

#include <errors.h>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum {
    /**
     *  Module type: Devicestatus Service
     */
    DEVICESTATUS_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: Devicestatus Kit
     */
    DEVICESTATUS_MODULE_TYPE_KIT = 1,
    DEVICESTATUS_MODULE_TYPE_CLIENT = 2
};

// offset of devicestatus error, only be used in this file.
constexpr ErrCode DEVICESTATUS_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_MSDP, DEVICESTATUS_MODULE_TYPE_SERVICE);

enum {
    E_DEVICESTATUS_WRITE_PARCEL_ERROR = DEVICESTATUS_SERVICE_ERR_OFFSET,
    E_DEVICESTATUS_READ_PARCEL_ERROR,
    E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED,
    E_DEVICESTATUS_GET_SERVICE_FAILED,
    E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED,
    E_DEVICESTATUS_INNER_ERR,
    ETASKS_INIT_FAIL,
    ETASKS_WAIT_TIMEOUT,
    ETASKS_WAIT_DEFERRED,
    ETASKS_POST_SYNCTASK_FAIL,
    ETASKS_POST_ASYNCTASK_FAIL,
    EPOLL_CREATE_FAIL,
};

enum {
    // 文件打开失败
    FILE_OPEN_FAIL = ErrCodeOffset(SUBSYS_MSDP, DEVICESTATUS_MODULE_TYPE_KIT),
    // 流缓冲读取失败
    STREAM_BUF_READ_FAIL,
    // 事件注册失败
    EVENT_REG_FAIL,
    // 参数注入失败
    PARAM_INPUT_FAIL
};

enum {
    NON_STD_EVENT = ErrCodeOffset(SUBSYS_MSDP, DEVICESTATUS_MODULE_TYPE_CLIENT),
    VAL_NOT_EXP,
    ERROR_UNSUPPORT,
    ERROR_NULL_POINTER,
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_ERRORS_H