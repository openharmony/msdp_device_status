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

#ifndef DRAG_ITEM_STYLE_PACKER_H
#define DRAG_ITEM_STYLE_PACKER_H

#include <message_parcel.h>

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragItemStylePacker {
private:
    static inline constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragItemStylePacker" };
public:
    static int32_t MarshallingDragItemStyle(const DragItemStyle &dragItemStyle, MessageParcel &data);
    static int32_t UnMarshallingDragItemStyle(MessageParcel &data, DragItemStyle &dragItemStyle);
private:
    static int32_t MarshallingAnimation(const Animation &animation, MessageParcel &data);
    static int32_t UnMarshallingAnimation(MessageParcel &data, Animation &animation);
    static int32_t MarshallingItemStyle(const ItemStyle &itemStyle, MessageParcel &data);
    static int32_t UnMarshallingItemStyle(MessageParcel &data, ItemStyle &itemStyle);
};

int32_t DragItemStylePacker::MarshallingDragItemStyle(const DragItemStyle &dragItemStyle, MessageParcel &data)
{
    CALL_DEBUG_ENTER;
    if (MarshallingAnimation(dragItemStyle.animation, data) != RET_OK) {
        FI_HILOGE("MarshallingAnimation failed");
        return RET_ERR;
    }
    if (MarshallingItemStyle(dragItemStyle.itemStyle, data) != RET_OK) {
        FI_HILOGE("MarshallingItemStyle failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragItemStylePacker::UnMarshallingDragItemStyle(MessageParcel &data, DragItemStyle &dragItemStyle)
{
    CALL_DEBUG_ENTER;
    if (UnMarshallingAnimation(data, dragItemStyle.animation) != RET_OK) {
        FI_HILOGE("UnMarshallingAnimation failed");
        return RET_ERR;
    }
    if (UnMarshallingItemStyle(data, dragItemStyle.itemStyle) != RET_OK) {
        FI_HILOGE("UnMarshallingItemStyle failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragItemStylePacker::MarshallingAnimation(const Animation &animation, MessageParcel &data)
{
    CALL_DEBUG_ENTER;
    WRITEINT32(data, animation.duration, ERR_INVALID_VALUE);
    WRITEINT32(data, animation.tempo, ERR_INVALID_VALUE);
    WRITEINT32(data, animation.delay, ERR_INVALID_VALUE);
    WRITEINT32(data, animation.iterations, ERR_INVALID_VALUE);
    WRITEINT32(data, animation.playMode, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t DragItemStylePacker::UnMarshallingAnimation(MessageParcel &data, Animation &animation)
{
    CALL_DEBUG_ENTER;
    READINT32(data, animation.duration, ERR_INVALID_VALUE);
    READINT32(data, animation.tempo, ERR_INVALID_VALUE);
    READINT32(data, animation.delay, ERR_INVALID_VALUE);
    READINT32(data, animation.iterations, ERR_INVALID_VALUE);
    READINT32(data, animation.playMode, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t DragItemStylePacker::MarshallingItemStyle(const ItemStyle &itemStyle, MessageParcel &data)
{
    CALL_DEBUG_ENTER;
    WRITEUINT32(data, itemStyle.foregroundColor, ERR_INVALID_VALUE);
    WRITEINT32(data, itemStyle.width, ERR_INVALID_VALUE);
    WRITEINT32(data, itemStyle.height, ERR_INVALID_VALUE);
    WRITEINT32(data, itemStyle.radius, ERR_INVALID_VALUE);
    WRITEINT32(data, itemStyle.border, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t DragItemStylePacker::UnMarshallingItemStyle(MessageParcel &data, ItemStyle &itemStyle)
{
    CALL_DEBUG_ENTER;
    READUINT32(data, itemStyle.foregroundColor, ERR_INVALID_VALUE);
    READINT32(data, itemStyle.width, ERR_INVALID_VALUE);
    READINT32(data, itemStyle.height, ERR_INVALID_VALUE);
    READINT32(data, itemStyle.radius, ERR_INVALID_VALUE);
    READINT32(data, itemStyle.border, ERR_INVALID_VALUE);
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_ITEM_STYLE_PACKER_H