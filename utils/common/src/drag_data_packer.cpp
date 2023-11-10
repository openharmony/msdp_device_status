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

#include "drag_data_packer.h"

#include "shadow_packer.h"
#include "summary_packer.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"

namespace OHOS {
namespace Msdp {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragDataPacker" };
} // namespace

namespace DeviceStatus {

int32_t DragDataPacker::Marshalling(const DragData &dragData, Parcel &data)
{
    if (ShadowPacker::Marshalling(dragData.shadowInfos, data) != RET_OK) {
        FI_HILOGE("Failed to marshalling shadowInfos");
        return RET_ERR;
    }
    WRITEUINT8VECTOR(data, dragData.buffer, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.udKey, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.filterInfo, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.extraInfo, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.sourceType, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.dragNum, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.pointerId, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayX, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayY, ERR_INVALID_VALUE);
    WRITEINT32(data, dragData.displayId, ERR_INVALID_VALUE);
    WRITEBOOL(data, dragData.hasCanceledAnimation, ERR_INVALID_VALUE);
    if (SummaryPacker::Marshalling(dragData.summarys, data) != RET_OK) {
        FI_HILOGE("Failed to summarys marshalling");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragDataPacker::UnMarshalling(Parcel &data, DragData &dragData)
{
    if (ShadowPacker::UnMarshalling(data, dragData.shadowInfos) != RET_OK) {
        FI_HILOGE("UnMarshallingShadowInfos failed");
        return RET_ERR;
    }
    READUINT8VECTOR(data, dragData.buffer, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(data, dragData.udKey, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(data, dragData.filterInfo, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(data, dragData.extraInfo, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.sourceType, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.dragNum, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.pointerId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayX, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayY, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READINT32(data, dragData.displayId, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READBOOL(data, dragData.hasCanceledAnimation, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (SummaryPacker::UnMarshalling(data, dragData.summarys) != RET_OK) {
        FI_HILOGE("Failed to summarys unmarshalling");
        return RET_ERR;
    }
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
