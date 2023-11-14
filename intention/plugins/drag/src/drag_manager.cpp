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

#include "drag_params.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
constexpr int32_t SUBSTR_UDKEY_LEN { 6 };
} // namespace

int32_t DragManager::AddListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::RemoveListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::StartDrag(const DragData &dragData, SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("PixelFormat:%{public}d, PixelAlphaType:%{public}d, PixelAllocatorType:%{public}d,"
        " PixelWidth:%{public}d, PixelHeight:%{public}d, shadowX:%{public}d, shadowY:%{public}d,"
        " sourceType:%{public}d, pointerId:%{public}d, displayId:%{public}d, displayX:%{public}d,"
        " displayY:%{public}d, dragNum:%{public}d, hasCanceledAnimation:%{public}d, udKey:%{public}s",
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetPixelFormat()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAlphaType()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAllocatorType()),
        dragData.shadowInfo.pixelMap->GetWidth(), dragData.shadowInfo.pixelMap->GetHeight(),
        dragData.shadowInfo.x, dragData.shadowInfo.y, dragData.sourceType, dragData.pointerId,
        dragData.displayId, dragData.displayX, dragData.displayY, dragData.dragNum, dragData.hasCanceledAnimation,
        dragData.udKey.substr(0, SUBSTR_UDKEY_LEN).c_str());
    if (dragState_ == DragState::START) {
        FI_HILOGE("Drag instance is running, can not start drag again");
        return RET_ERR;
    }
    dragOutSession_ = sess;
    if (InitDataManager(dragData) != RET_OK) {
        FI_HILOGE("Init data manager failed");
        return RET_ERR;
    }
    if (OnStartDrag() != RET_OK) {
        FI_HILOGE("OnStartDrag failed");
        return RET_ERR;
    }
    dragState_ = DragState::START;
    stateNotify_.StateChangedNotify(DragState::START);
    StateChangedNotify(DragState::START);
    return RET_OK;
}

int32_t DragManager::StopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    if (dragState_ == DragState::STOP) {
        FI_HILOGE("No drag instance running, can not stop drag");
        return RET_ERR;
    }
    if ((result != DragResult::DRAG_EXCEPTION) && (context_ != nullptr) && (timerId_ >= 0)) {
        context_->GetTimerManager().RemoveTimer(timerId_);
        timerId_ = -1;
    }
    int32_t ret = RET_OK;
    if (OnStopDrag(result, hasCustomAnimation) != RET_OK) {
        FI_HILOGE("On stop drag failed");
        ret = RET_ERR;
    }
    dragState_ = DragState::STOP;
    stateNotify_.StateChangedNotify(DragState::STOP);
    if (NotifyDragResult(result) != RET_OK) {
        FI_HILOGE("Notify drag result failed");
    }
    DRAG_DATA_MGR.ResetDragData();
    dragResult_ = static_cast<DragResult>(result);
    StateChangedNotify(DragState::STOP);
    return RET_OK;
}

int32_t DragManager::GetDragTargetPid() const
{
    return RET_ERR;
}

int32_t DragManager::GetUdKey(std::string &udKey) const
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::UpdateDragStyle(DragCursorStyle style, int32_t targetPid, int32_t targetTid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::SetDragWindowVisible(bool visible)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::InitDataManager(const DragData &dragData) const
{
    CALL_DEBUG_ENTER;
    DRAG_DATA_MGR.Init(dragData);
    return RET_OK;
}

void DragManager::StateChangedNotify(DragState state)
{
    CALL_DEBUG_ENTER;
    if ((stateChangedCallback_ != nullptr) && (!DRAG_DATA_MGR.IsMotionDrag())) {
        stateChangedCallback_(state);
    }
}

int32_t DragManager::OnStartDrag()
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::OnStopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::NotifyDragResult(DragResult result)
{
    CALL_DEBUG_ENTER;
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    int32_t targetPid = GetDragTargetPid();
    NetPacket pkt(MessageId::DRAG_NOTIFY_RESULT);
    if ((result < DragResult::DRAG_SUCCESS) || (result > DragResult::DRAG_EXCEPTION)) {
        FI_HILOGE("Invalid result:%{public}d", static_cast<int32_t>(result));
        return RET_ERR;
    }
    pkt << dragData.displayX << dragData.displayY << static_cast<int32_t>(result) << targetPid;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    CHKPR(dragOutSession_, RET_ERR);
    if (!dragOutSession_->SendMsg(pkt)) {
        FI_HILOGE("Send message failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
