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

#include "drag_data_adapter.h"

#include "hitrace_meter.h"
#include "pointer_style.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDataAdapter" };
} // namespace

DragDataAdapter::DragDataAdapter() = default;
DragDataAdapter::~DragDataAdapter() = default;

void DragDataAdapter::Init(const DragData &dragData, const MMI::PointerStyle &pointerStyle)
{
    CALL_DEBUG_ENTER;
    extraData_.buffer = dragData.buffer;
    extraData_.sourceType = dragData.sourceType;
    extraData_.appended = false;
    coordinate_ = std::make_pair(dragData.x, dragData.y);
    pixelMap_ = dragData.pixelMap;
    dragNum_ = dragData.dragNum;
    pointerStyle_ = pointerStyle;
}

OHOS::MMI::ExtraData DragDataAdapter::GetExtraData() const
{
    return extraData_;
}
std::pair<int, int> DragDataAdapter::GetCoordinate() const
{
    return coordinate_;
}

std::shared_ptr<OHOS::Media::PixelMap> DragDataAdapter::GetPixelMap() const
{
    return pixelMap_;
}

int32_t DragDataAdapter::GetDragNum() const
{
    return dragNum_;
}

int32_t DragDataAdapter::GetDragStyle() const
{
    return dragStyle_;
}

std::u16string DragDataAdapter::GetDragMessage() const
{
    return dragMessage_;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS