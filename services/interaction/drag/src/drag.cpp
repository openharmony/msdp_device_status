/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "drag.h"
#include "proto.h"
#include "input_manager.h"
#include "devicestatus_define.h"
#include "pixel_map.h"
#include "extra_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "Drag" };
} // namespace

Drag::Drag()
{
}

Drag::~Drag()
{
}

int32_t Drag::StartDrag(const DragData &dragData)
{
    /**
     * TODO AddMonitor, 向多模添加监听
     * TODO AppendExtraData，更改多模事件上报模式为Drag
     * TODO 存buffer，将buffer暂存
     * TODO 按照设计wiki时序图开发
     * x y 是干啥的， PixelMap是干啥的， 没有用到，
    */
    if (!IsNormal()) {
        FI_HILOGE("Drag instance is running");
        return RET_ERR;
    }
    if (monitorId_ <= 0) {
        auto monitor = std::make_shared<MonitorConsumer>(
            std::bind(&CoordinationSM::UpdateLastPointerEventCallback, this, std::placeholders::_1));
        monitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor);
        if (monitorId_ <= 0) {
            FI_HILOGE("Failed to add monitor, Error code:%{public}d", monitorId_);
            monitorId_ = -1;
            return RET_ERR;
        }
    }
    MMI::ExtraData extraData;
    SetExtraData(extraData);
    MMI::InputManager::GetInstance()->AppendExtraData(extraData);
    return RET_OK;
}

int32_t Drag::StopDrag(int32_t &dragResult)
{
    /**
     * TODO AppendExtraData，恢复多模事件上报模式
     * TODO RemoveMonitor，去除监听
     * TODO callback(dragResult)，执行结束回调
     * TODO 按照设计wiki时序图开发
    */
   if (IsNormal()) {
        FI_HILOGE("StopDrag failed, no drag instance is running");
        return RET_ERR;
    }

}

bool Drag::IsStarting() const
{
    dragState_
    return dragState_ == DragState::START_DRAG;

}

bool Drag::IsStopping() const
{
    return dragState_ == DragState::STOP_DRAG;
}

bool Drag::IsDragging() const
{
    return dragState_ == DragState::DRAGGING;
}

bool Drag::IsNormal() const
{
    return dragState_ == DragState::NORMAL;
}

void Drag::SetExtraData(ExtraData &extraData)
{
    extraData.appended = true;
    extraData.buffer = dragData_.buffer;
    extraData.sourceType = dragData_.sourceType;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS