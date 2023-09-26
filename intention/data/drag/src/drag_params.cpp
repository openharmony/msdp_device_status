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

#include "drag_params.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
StartDragParam::StartDragParam(const DragData &dragData) : dragData(dragData)
{}

bool StartDragParam::Marshalling(Parcel &data) const
{
    return {
        WRITEINT32(data, dragData.shadowInfo.x) &&
        WRITEINT32(data, dragData.shadowInfo.y) &&
        WRITEUINT8VECTOR(data, dragData.buffer) &&
        WRITESTRING(data, dragData.udKey) &&
        WRITESTRING(data, dragData.filterInfo) &&
        WRITESTRING(data, dragData.extraInfo) &&
        WRITEINT32(data, dragData.sourceType) &&
        WRITEINT32(data, dragData.dragNum) &&
        WRITEINT32(data, dragData.pointerId) &&
        WRITEINT32(data, dragData.displayX) &&
        WRITEINT32(data, dragData.displayY) &&
        WRITEINT32(data, dragData.displayId) &&
        WRITEBOOL(data, dragData.hasCanceledAnimation)
    };
}

bool StartDragParam::Unmarshalling(Parcel &data)
{
    return {
        READINT32(data, dragData.shadowInfo.x) &&
        READINT32(data, dragData.shadowInfo.y) &&
        READUINT8VECTOR(data, dragData.buffer) &&
        READSTRING(data, dragData.udKey) &&
        READSTRING(data, dragData.filterInfo) &&
        READSTRING(data, dragData.extraInfo) &&
        READINT32(data, dragData.sourceType) &&
        READINT32(data, dragData.dragNum) &&
        READINT32(data, dragData.pointerId) &&
        READINT32(data, dragData.displayX) &&
        READINT32(data, dragData.displayY) &&
        READINT32(data, dragData.displayId) &&
        READBOOL(data, dragData.hasCanceledAnimation)
    };
}

StopDragParam::StopDragParam(int32_t result, bool hasCustomAnimation)
    : result(result), hasCustomAnimation(hasCustomAnimation)
{}

bool StopDragParam::Marshalling(Parcel &data) const
{
    if (result < DragResult::DRAG_SUCCESS || result > DragResult::DRAG_EXCEPTION) {
        FI_HILOGE("Invalid result:%{public}d", static_cast<int32_t>(result));
        return false;
    }
    return {
        WRITEINT32(data, static_cast<int32_t>(result));
        WRITEBOOL(data, hasCustomAnimation);
    };
}

bool StopDragParam::Unmarshalling(Parcel &data)
{
    return {
        READINT32(data, result) &&
        READBOOL(data, hasCustomAnimation)
    };
}

DragStyleParam::DragStyleParam(int32_t style) : mouseStyle(style)
{
}

bool DragStyleParam::Marshalling(Parcel &data) const
{
    return WRITEINT32(data, static_cast<int32_t>(mouseStyle));
}

bool DragStyleParam::Unmarshalling(Parcel &data)
{
    return READINT32(data, mouseStyle);
}

bool DragTargetPidParam::Marshalling(Parcel &data) const
{
    return true;
}

bool DragTargetPidParam::Unmarshalling(Parcel &data)
{
    return READINT32(data, pid);
}

bool GetUdKeyParam::Marshalling(Parcel &data) const
{
    return true;
}

bool GetUdKeyParam::Unmarshalling(Parcel &data)
{
    return READSTRING(data, udKey);
}

bool AddDragListenerParam::Marshalling(Parcel &data) const
{
    return true;
}

bool AddDragListenerParam::Unmarshalling(Parcel &data)
{
    return true;
}

bool RemoveDragListenerParam::Marshalling(Parcel &data) const
{
    return true;
}

bool RemoveDragListenerParam::Unmarshalling(Parcel &data)
{
    return true;
}

SetDragWindowVisibleParam::SetDragWindowVisibleParam(bool visible) : visible(visible)
{}

bool SetDragWindowVisibleParam::Marshalling(Parcel &data) const
{
    return WRITEBOOL(data, visible);
}

bool SetDragWindowVisibleParam::Unmarshalling(Parcel &data)
{
    return READBOOL(data, visible);
}

GetShadowOffsetParam::GetShadowOffsetParam(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height)
    : offsetX(offsetX),
      offsetY(offsetY),
      width(width),
      height(height)
{}

bool GetShadowOffsetParam::Marshalling(Parcel &data) const
{
    return true;
}

bool GetShadowOffsetParam::Unmarshalling(Parcel &data)
{
    return {
        READINT32(data, offsetX) &&
        READINT32(data, offsetY) &&
        READINT32(data, width) &&
        READINT32(data, height)
    };
}

UpdateShadowPicParam::UpdateShadowPicParam(ShadowInfo shadowInfo) : shadowInfo(shadowInfo)
{}

bool UpdateShadowPicParam::Marshalling(Parcel &data) const
{
    if (!shadowInfo.pixelMap->Marshalling(data)) {
        FI_HILOGE("Failed to marshalling pixelMap");
        return false;
    }
    return {
        WRITEINT32(data, shadowInfo.x);
        WRITEINT32(data, shadowInfo.y);
    };
}

bool UpdateShadowPicParam::Unmarshalling(Parcel &data)
{
    return {
        READINT32(data, shadowInfo.x) &&
        READINT32(data, shadowInfo.y)
    };
}

bool DefaultDragReply::Marshalling(Parcel &data) const
{
    return true;
}

bool DefaultDragReply::Unmarshalling(Parcel &data)
{
    return true;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS