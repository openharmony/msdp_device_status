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

#include "drag_manager.h"

#include <iostream>
#include <sstream>

#include "drag_data_adapter.h"
#include "devicestatus_define.h"
#include "drag_data.h"
#include "extra_data.h"
#include "fi_log.h"
#include "hitrace_meter.h"
#include "input_manager.h"
#include "proto.h"
#include "pixel_map.h"
#include "pointer_style.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
} // namespace

int32_t DragManager::StartDrag(const DragData &dragData, int32_t pid)
{
    CALL_DEBUG_ENTER;
    int32_t pixelBytes = dragData.pixelMap->GetPixelBytes();
    int32_t byteCount = dragData.pixelMap->GetByteCount();
    FI_HILOGD("pixelBytes: %{public}d, byteCount: %{public}d, x: %{public}d, y: %{public}d",
        pixelBytes, byteCount, dragData.x, dragData.y);
    FI_HILOGD("bufferSize: %{public}d", dragData.buffer.size());
    FI_HILOGD("ByteCount of pixelMap: %{public}d", dragData.pixelMap->GetByteCount());
    (void) pid;
    (void) dragState_;
    (void) dragOutPid_;
    (void) monitorId_;
    (void) dragDrawing_;
    return RET_OK;
}

int32_t DragManager::StopDrag(int32_t &dragResult)
{
    CALL_DEBUG_ENTER;
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    (void) inputMgr;
    (void) dragTargetPid_;
    (void) monitorId_;
    (void) dragState_;
    return RET_OK;
}

int32_t DragManager::GetDragTargetPid()
{
    return dragTargetPid_;
}

int32_t DragManager::AddMonitor(int32_t pid)
{
    return RET_ERR;
}

int32_t DragManager::RemoveMonitor(int32_t monitorId)
{
    return RET_ERR;
}

int32_t DragManager::NotifyMonitor(DragState dragState)
{
    return RET_ERR;
}

MMI::ExtraData DragManager::ConstructExtraData(const DragData &dragData, bool appended)
{
    OHOS::MMI::ExtraData extraData;
    if (appended) {
        extraData.appended = true;
        extraData.buffer = dragData.buffer;
        extraData.sourceType = dragData.sourceType;
    }
    return extraData;
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS