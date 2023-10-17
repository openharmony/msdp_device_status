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
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragParams" };
} // namespace
StartDragParam::StartDragParam()
{}

StartDragParam::StartDragParam(const DragData &dragData) : dragData(dragData)
{}

bool StartDragParam::Marshalling(Parcel &data) const
{
    return (
        data.WriteInt32(dragData.shadowInfo.x) &&
        data.WriteInt32(dragData.shadowInfo.y) &&
        data.WriteUInt8Vector(dragData.buffer) &&
        data.WriteString(dragData.udKey) &&
        data.WriteString(dragData.filterInfo) &&
        data.WriteString(dragData.extraInfo) &&
        data.WriteInt32(dragData.sourceType) &&
        data.WriteInt32(dragData.dragNum) &&
        data.WriteInt32(dragData.pointerId) &&
        data.WriteInt32(dragData.displayX) &&
        data.WriteInt32(dragData.displayY) &&
        data.WriteInt32(dragData.displayId) &&
        data.WriteBool(dragData.hasCanceledAnimation)
    );
}

bool StartDragParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(dragData.shadowInfo.x) &&
        data.ReadInt32(dragData.shadowInfo.y) &&
        data.ReadUInt8Vector(&dragData.buffer) &&
        data.ReadString(dragData.udKey) &&
        data.ReadString(dragData.filterInfo) &&
        data.ReadString(dragData.extraInfo) &&
        data.ReadInt32(dragData.sourceType) &&
        data.ReadInt32(dragData.dragNum) &&
        data.ReadInt32(dragData.pointerId) &&
        data.ReadInt32(dragData.displayX) &&
        data.ReadInt32(dragData.displayY) &&
        data.ReadInt32(dragData.displayId) &&
        data.ReadBool(dragData.hasCanceledAnimation)
    );
}

StopDragParam::StopDragParam(int32_t result, bool hasCustomAnimation)
    : result(result), hasCustomAnimation(hasCustomAnimation)
{}

StopDragParam::StopDragParam()
{}

bool StopDragParam::Marshalling(Parcel &data) const
{
    if (static_cast<DragResult>(result) < DragResult::DRAG_SUCCESS ||
        static_cast<DragResult>(result) > DragResult::DRAG_EXCEPTION) {
        FI_HILOGE("Invalid result:%{public}d", static_cast<int32_t>(result));
        return false;
    }
    return (
        data.WriteInt32(static_cast<int32_t>(result)) &&
        data.WriteBool(hasCustomAnimation)
    );
}

bool StopDragParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(result) &&
        data.ReadBool(hasCustomAnimation)
    );
}

DragStyleParam::DragStyleParam(int32_t style) : mouseStyle(style)
{
}

bool DragStyleParam::Marshalling(Parcel &data) const
{
    return data.WriteInt32(static_cast<int32_t>(mouseStyle));
}

bool DragStyleParam::Unmarshalling(Parcel &data)
{
    return data.ReadInt32(mouseStyle);
}

bool DragTargetPidParam::Marshalling(Parcel &data) const
{
    return true;
}

bool DragTargetPidParam::Unmarshalling(Parcel &data)
{
    return data.ReadInt32(pid);
}

GetUdKeyParam::GetUdKeyParam(std::string &udKey) : udKey(udKey)
{
}

bool GetUdKeyParam::Marshalling(Parcel &data) const
{
    return true;
}

bool GetUdKeyParam::Unmarshalling(Parcel &data)
{
    return data.ReadString(udKey);
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

SetDragWindowVisibleParam::SetDragWindowVisibleParam()
{
}

SetDragWindowVisibleParam::SetDragWindowVisibleParam(bool visible) : visible(visible)
{}

bool SetDragWindowVisibleParam::Marshalling(Parcel &data) const
{
    return data.WriteBool(visible);
}

bool SetDragWindowVisibleParam::Unmarshalling(Parcel &data)
{
    return data.ReadBool(visible);
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
    return (
        data.ReadInt32(offsetX) &&
        data.ReadInt32(offsetY) &&
        data.ReadInt32(width) &&
        data.ReadInt32(height)
    );
}

UpdateShadowPicParam::UpdateShadowPicParam()
{
}

UpdateShadowPicParam::UpdateShadowPicParam(ShadowInfo shadowInfo) : shadowInfo(shadowInfo)
{}

bool UpdateShadowPicParam::Marshalling(Parcel &data) const
{
    if (!shadowInfo.pixelMap->Marshalling(data)) {
        FI_HILOGE("Failed to marshalling pixelMap");
        return false;
    }
    return (
        data.WriteInt32(shadowInfo.x) &&
        data.WriteInt32(shadowInfo.y)
    );
}

bool UpdateShadowPicParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(shadowInfo.x) &&
        data.ReadInt32(shadowInfo.y)
    );
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