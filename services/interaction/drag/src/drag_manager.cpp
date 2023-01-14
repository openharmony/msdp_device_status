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

#include "hitrace_meter.h"
#include "drag_data_adapter.h"
#include "proto.h"
#include "input_manager.h"
#include "devicestatus_define.h"
#include "pixel_map.h"
#include "drag_data.h"
#include "extra_data.h"
#include "pointer_style.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
} // namespace

int32_t DragManager::StartDrag(const DragData &dragData, int32_t pid) // 写完， 问亚军，委托相关
{
    CALL_DEBUG_ENTER;
    TestStartDrag(dragData, pid);
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    if (dragState_ == DragState::DRAGGING) {
        FI_HILOGE("Drag instance is running, can not start drag again");
        return RET_ERR;
    }
    dragOutPid_ = pid;
    if (monitorId_ < 0) {
        monitorId_ = inputMgr->AddMonitor(monitorConsumer_);
        if (monitorId_ < 0) {
            FI_HILOGE("AddMonitor failed, Error code:%{public}d", monitorId_);
            return RET_ERR;
        }
    } else {
        FI_HILOGE("Can not add monitor again");
        return RET_ERR;
    }
    inputMgr->AppendExtraData(ConstructExtraData(dragData, true));
    // MMI::PointerStyle pointerStyle;
    // int32_t ret = inputMgr->GetPointerStyle(0, pointerStyle); // 获取全局PointerStyle的接口尚未完成，不需要windowId, 或传0
    // if (ret != RET_OK) {
    //     FI_HILOGE("GetPointerStyle failed");
    //     return ret;
    // }
    // DataAdapter.Init(dragData, pointerStyle);
    // if (inputMgr->SetPointerVisible(false) != RET_OK) {
    //     FI_HILOGE("SetPointerVisible failed");
    //     return RET_ERR;
    // }
    dragDrawing_.Draw(0, 0); // Draw 的参数 x y 还不能获取
    dragState_ = DragState::DRAGGING;
    // if (NotifyMonitor(DragState::DRAGGING) != RET_OK) {
    //     FI_HILOGE("Failed to notify monitor");
    //     return RET_ERR;
    // }
    return RET_OK;
}

int32_t DragManager::StopDrag(int32_t &dragResult)
{
    CALL_DEBUG_ENTER;
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    if (dragState_ == DragState::FREE) {
        FI_HILOGE("No drag instance is running, can not stop drag");
        return RET_ERR;
    }
    (void) inputMgr;
    dragTargetPid_ = GetDragTargetPid(); // 鑫海接口
    // // TODO 结束阴影托管
    if (monitorId_ == -1) {
        FI_HILOGE("Invalid monitorId_ to remove");
        return RET_ERR;
    }
    // inputMgr->RemoveMonitor(monitorId_);
    // inputMgr->AppendExtraData(ConstructExtraData(DataAdapter.GetDragData(), false));
    // if (inputMgr->SetPointerVisible(true) != RET_OK) {
    //     FI_HILOGE("SetPointerVisible failed");
    //     return RET_ERR;
    // }
    // TODO 删除绘制
    /**
     * TODO 通知客户端执行callback
     * 修改dragResult
     * DRAG_SUCCESS = 0,
     * DRAG_FAIL = 1,
     * DRAG_CANCEL = 2,
    */
    dragState_ = DragState::FREE;
    // if (NotifyMonitor(DragState::FREE) != RET_OK) {
    //     FI_HILOGE("Failed to notify monitor");
    //     return RET_ERR;
    // }
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

void DragManager::TestStartDrag(const DragData &dragData, int32_t pid)
{
    int32_t pixelBytes = dragData.pixelMap->GetPixelBytes(); 
    FI_HILOGD("pixelBytes: %{public}d", pixelBytes);
    FI_HILOGD("x: %{public}d, y: %{public}d", dragData.x, dragData.y);
    std::ostringstream ostream;
    for (const auto& elem : dragData.buffer) {
        ostream << elem << ", ";
    }
    FI_HILOGD("buffer: %{public}s", ostream.str().c_str());
    FI_HILOGD("sourceType: %{public}d", dragData.sourceType);
    FI_HILOGD("clientPid: %{public}d", pid);
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