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

#include "extra_data.h"
#include "hitrace_meter.h"
#include "input_manager.h"
#include "pixel_map.h"
#include "pointer_style.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "drag_data_adapter.h"
#include "fi_log.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
} // namespace

void DragManager::OnSessionLost(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    CHKPV(session);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    stateNotify_.RemoveNotifyMsg(info);
}

int32_t DragManager::AddListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    info->msgId = MessageId::DRAG_STATE_LISTENER;
    stateNotify_.AddNotifyMsg(info);
    return RET_OK;
}

int32_t DragManager::RemoveListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    stateNotify_.RemoveNotifyMsg(info);
    return RET_OK;
}

int32_t DragManager::StartDrag(const DragData &dragData, SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    if (dragState_ == DragState::DRAGGING) {
        FI_HILOGE("Drag instance is running, can not start drag again");
        return RET_ERR;
    }
    CHKPR(sess, RET_ERR);
    dragOutSession_ = sess;
    auto callback = std::bind(&DragManager::DragCallback, this, std::placeholders::_1);
    monitorConsumer_ = std::make_shared<MonitorConsumer>(MonitorConsumer(callback));
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    MMI::PointerStyle pointerStyle;
    if (inputMgr->GetPointerStyle(-1, pointerStyle) != RET_OK) {
        FI_HILOGE("GetPointerStyle failed");
        return RET_ERR;
    }
    DataAdapter.Init(dragData, pointerStyle);
    auto extraData = GetExtraData(true);
    inputMgr->AppendExtraData(extraData);
    monitorId_ = inputMgr->AddMonitor(monitorConsumer_);
    if (monitorId_ < 0) {
        FI_HILOGE("AddMonitor failed, Error code:%{public}d", monitorId_);
        return RET_ERR;
    }
    inputMgr->SetPointerVisible(false);
    dragState_ = DragState::DRAGGING;
    stateNotify_.StateChangedNotify(DragMessage::MSG_DRAG_STATE_START);
    return RET_OK;
}

int32_t DragManager::StopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    if (dragState_ == DragState::FREE) {
        FI_HILOGE("No drag instance is running, can not start drag again");
        return RET_ERR;
    }
    dragState_ = DragState::FREE;
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    inputMgr->SetPointerVisible(true);
    stateNotify_.StateChangedNotify(DragMessage::MSG_DRAG_STATE_STOP);
    if (monitorId_ < 0) {
        FI_HILOGE("Invalid monitor to be removed :%{public}d", monitorId_);
        return RET_ERR;
    }
    inputMgr->RemoveMonitor(monitorId_);
    NotifyDragResult(result);
    dragOutSession_ = nullptr;
    return RET_OK;
}

int32_t DragManager::GetDragTargetPid() const
{
    return dragTargetPid_;
}

int32_t DragManager::NotifyDragResult(int32_t result)
{
    CALL_DEBUG_ENTER;
    NetPacket pkt(MessageId::DRAG_NOTIFY_RESULT);
    pkt << result;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    if (!dragOutSession_->SendMsg(pkt)) {
        FI_HILOGE("Send message failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}

void DragManager::DragCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    auto pointerAction = pointerEvent->GetPointerAction();
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
        OnDragMove(pointerEvent);
    } else if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        OnDragUp(pointerEvent);
    } else {
        FI_HILOGW("Unsupported pointerAction:%{public}d", pointerEvent->GetPointerAction());
    }
}

void DragManager::OnDragMove(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
}

void DragManager::OnDragUp(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    auto inputMgr =  OHOS::MMI::InputManager::GetInstance();
    dragTargetPid_ = inputMgr->GetWindowPid(pointerItem.GetTargetWindowId());
    auto extraData = GetExtraData(false);
    inputMgr->AppendExtraData(extraData);
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const {}
void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const {}
void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    CHKPV(callback_);
    callback_(pointerEvent);
}

int32_t DragManager::OnRegisterThumbnailDraw(SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    return RET_OK;
}

int32_t DragManager::OnUnregisterThumbnailDraw(SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    return RET_OK;
}

OHOS::MMI::ExtraData DragManager::GetExtraData(bool appended) const
{
    auto dragData = DataAdapter.GetDragData();
    OHOS::MMI::ExtraData extraData;
    extraData.buffer = dragData.buffer;
    extraData.sourceType = dragData.sourceType;
    extraData.appended = appended;
    return extraData;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
