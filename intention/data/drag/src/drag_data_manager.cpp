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

#include "drag_data_manager.h"

#include "hitrace_meter.h"
#include "pointer_style.h"

#include "devicestatus_define.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragDataManager" };
constexpr int32_t DEFAULT_DISPLAY_ID { 0 };
} // namespace

DragDataManager::DragDataManager() = default;
DragDataManager::~DragDataManager() = default;

void DragDataManager::Init(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    dragData_ = dragData;
    if (dragData.displayId < DEFAULT_DISPLAY_ID) {
        dragData_.displayId = DEFAULT_DISPLAY_ID;
        FI_HILOGW("The value of displayId(%{public}d) is correcting to 0", dragData.displayId);
    }
    targetTid_ = -1;
    targetPid_ = -1;
}

DragData DragDataManager::GetDragData() const
{
    return dragData_;
}

void DragDataManager::ResetDragData()
{
    dragData_ = { };
    dragItemStyle_ = { };
}

bool DragDataManager::IsMotionDrag() const
{
    FI_HILOGD("isMotionDrag_:%{public}d", isMotionDrag_);
    return isMotionDrag_;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS