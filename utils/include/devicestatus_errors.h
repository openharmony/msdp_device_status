/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
enum {
    /**
     *  Module type: Devicestatus Service
     */
    DEVICESTATUS_MODULE_TYPE_SERVICE = 0,
    /**
     *  Module type: Devicestatus Kit
     */
    DEVICESTATUS_MODULE_TYPE_KIT = 1
};

// offset of devicestatus error, only be used in this file.
constexpr ErrCode DEVICESTATUS_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_MSDP, DEVICESTATUS_MODULE_TYPE_SERVICE);

enum {
    E_DEVICESTATUS_WRITE_PARCEL_ERROR = DEVICESTATUS_SERVICE_ERR_OFFSET,
    E_DEVICESTATUS_READ_PARCEL_ERROR,
    E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED,
    E_DEVICESTATUS_GET_SERVICE_FAILED,
    E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED,
    E_DEVICESTATUS_INNER_ERR
};

enum {
    RET_OK = 0,
    RET_ERR = -1
};
} // namespace Msdp
} // namespace OHOS

#endif // DEVICESTATUS_ERRORS_H