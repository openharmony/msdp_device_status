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

#include "drag_data_packer.h"

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
    WRITESTRING(data, dragData.extraInfo, ERR_INVALID_VALUE);
    WRITESTRING(data, dragData.filterInfo, ERR_INVALID_VALUE);
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
    READSTRING(data, dragData.extraInfo, E_DEVICESTATUS_READ_PARCEL_ERROR);
    READSTRING(data, dragData.filterInfo, E_DEVICESTATUS_READ_PARCEL_ERROR);
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

int32_t ShadowPacker::Marshalling(const std::vector<ShadowInfo> &shadowInfos, Parcel &data)
{
    CALL_DEBUG_ENTER;
    if (shadowInfos.empty()) {
        FI_HILOGE("Invalid parameter shadowInfos");
        return ERR_INVALID_VALUE;
    }
    int32_t shadowNum = static_cast<int32_t>(shadowInfos.size());
    if (shadowNum > SHADOW_NUM_LIMIT) {
        FI_HILOGW("Only %{public}d shadowInfos allowed at most, now %{public}d", SHADOW_NUM_LIMIT, shadowNum);
        shadowNum = SHADOW_NUM_LIMIT;
    }
    WRITEINT32(data, shadowNum, ERR_INVALID_VALUE);
    for (int32_t i = 0; i < shadowNum; i++) {
        CHKPR(shadowInfos[i].pixelMap, RET_ERR);
        if (!shadowInfos[i].pixelMap->Marshalling(data)) {
            FI_HILOGE("Failed to marshalling pixelMap");
            return ERR_INVALID_VALUE;
        }
        WRITEINT32(data, shadowInfos[i].x, ERR_INVALID_VALUE);
        WRITEINT32(data, shadowInfos[i].y, ERR_INVALID_VALUE);
    }
    return RET_OK;
}

int32_t ShadowPacker::UnMarshalling(Parcel &data, std::vector<ShadowInfo> &shadowInfos)
{
    CALL_DEBUG_ENTER;
    int32_t shadowNum { 0 };
    READINT32(data, shadowNum, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (shadowNum <= 0 || shadowNum > SHADOW_NUM_LIMIT) {
        FI_HILOGE("Invalid shadowNum:%{public}d", shadowNum);
        return RET_ERR;
    }
    for (int32_t i = 0; i < shadowNum; i++) {
        ShadowInfo shadowInfo;
        auto pixelMap = OHOS::Media::PixelMap::Unmarshalling(data);
        CHKPR(pixelMap, RET_ERR);
        shadowInfo.pixelMap = std::shared_ptr<OHOS::Media::PixelMap>(pixelMap);
        READINT32(data, shadowInfo.x, E_DEVICESTATUS_READ_PARCEL_ERROR);
        READINT32(data, shadowInfo.y, E_DEVICESTATUS_READ_PARCEL_ERROR);
        shadowInfos.push_back(shadowInfo);
    }
    return RET_OK;
}

int32_t SummaryPacker::Marshalling(const SummaryMap &val, Parcel &parcel)
{
    WRITEINT32(parcel, static_cast<int32_t>(val.size()), ERR_INVALID_VALUE);
    for (auto const &[k, v] : val) {
        WRITESTRING(parcel, k, ERR_INVALID_VALUE);
        WRITEINT64(parcel, v, ERR_INVALID_VALUE);
    }
    return RET_OK;
}

int32_t SummaryPacker::UnMarshalling(Parcel &parcel, SummaryMap &val)
{
    int32_t size = 0;
    READINT32(parcel, size, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (size < 0) {
        FI_HILOGE("Invalid size:%{public}d", size);
        return RET_ERR;
    }
    size_t readAbleSize = parcel.GetReadableBytes();
    if ((static_cast<size_t>(size) > readAbleSize) || static_cast<size_t>(size) > val.max_size()) {
        FI_HILOGE("Invalid size:%{public}d", size);
        return RET_ERR;
    }
    for (int32_t i = 0; i < size; ++i) {
        std::string key;
        READSTRING(parcel, key, E_DEVICESTATUS_READ_PARCEL_ERROR);
        READINT64(parcel, val[key], E_DEVICESTATUS_READ_PARCEL_ERROR);
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
