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

#include "display_manager.h"
#include "extra_data.h"
#include "hitrace_meter.h"
#include "input_manager.h"
#include "pixel_map.h"
#include "pointer_style.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "drag_data_manager.h"
#include "fi_log.h"
#include "proto.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "udmf_client.h"
#include "unified_types.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
constexpr int32_t TIMEOUT_MS { 2000 };
constexpr int32_t DRAG_PRIORITY { 500 };
constexpr double PERCENT_CONST { 100.0 };
} // namespace

int32_t DragManager::Init(IContext* context)
{
    CALL_INFO_TRACE;
    CHKPR(context, RET_ERR);
    context_ = context;
    return RET_OK;
}

void DragManager::OnSessionLost(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    if (RemoveListener(session) != RET_OK) {
        FI_HILOGE("Failed to clear client listener");
    }
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
    FI_HILOGD("PixelFormat:%{public}d, PixelAlphaType:%{public}d, PixelAllocatorType:%{public}d, PixelWidth:%{public}d,"
        "PixelHeight:%{public}d, sourceType:%{public}d, pointerId:%{public}d, shadowX:%{public}d,"
        "shadowY:%{public}d, displayId:%{public}d, dragNum:%{public}d, hasCanceledAnimation:%{public}d",
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetPixelFormat()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAlphaType()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAllocatorType()),
        dragData.shadowInfo.pixelMap->GetWidth(), dragData.shadowInfo.pixelMap->GetHeight(), dragData.sourceType,
        dragData.pointerId, dragData.shadowInfo.x, dragData.shadowInfo.y,
        dragData.displayId, dragData.dragNum, dragData.hasCanceledAnimation);
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
    if (result != DragResult::DRAG_EXCEPTION && context_ != nullptr && timerId_ >= 0) {
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
        ret = RET_ERR;
    }
    DRAG_DATA_MGR.ResetDragData();
    dragResult_ = static_cast<DragResult>(result);
    StateChangedNotify(DragState::STOP);
    return ret;
}

int32_t DragManager::GetDragTargetPid() const
{
    return DRAG_DATA_MGR.GetTargetPid();
}

int32_t DragManager::GetUdKey(std::string &udKey) const
{
    CALL_DEBUG_ENTER;
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.udKey.empty()) {
        FI_HILOGE("Target udKey is empty");
        return RET_ERR;
    }
    udKey = dragData.udKey;
    return RET_OK;
}

int32_t DragManager::UpdateDragStyle(DragCursorStyle style, int32_t targetPid, int32_t targetTid)
{
    CALL_DEBUG_ENTER;
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running, can not update drag style");
        return RET_ERR;
    }
    if (style < DragCursorStyle::DEFAULT || style > DragCursorStyle::MOVE) {
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    DRAG_DATA_MGR.SetDragStyle(style);
    DRAG_DATA_MGR.SetTargetPid(targetPid);
    DRAG_DATA_MGR.SetTargetTid(targetTid);
    dragDrawing_.UpdateDragStyle(style);
    return RET_OK;
}

int32_t DragManager::NotifyDragResult(DragResult result)
{
    CALL_DEBUG_ENTER;
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    int32_t targetPid = GetDragTargetPid();
    NetPacket pkt(MessageId::DRAG_NOTIFY_RESULT);
    if (result < DragResult::DRAG_SUCCESS || result > DragResult::DRAG_EXCEPTION) {
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

void DragManager::DragCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    int32_t pointerAction = pointerEvent->GetPointerAction();
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
        OnDragMove(pointerEvent);
    } else if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        OnDragUp(pointerEvent);
    } else {
        FI_HILOGD("Unknow action, sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
            pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerAction);
    }
}

void DragManager::OnDragMove(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    FI_HILOGD("SourceType:%{public}d, pointerId:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId());
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    dragDrawing_.Draw(pointerEvent->GetTargetDisplayId(), pointerItem.GetDisplayX(), pointerItem.GetDisplayY());
}

void DragManager::SendDragData(int32_t targetTid, const std::string &udKey)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    UDMF::QueryOption option;
    option.key = udKey;
    UDMF::Privilege privilege;
    privilege.tokenId = static_cast<uint32_t>(targetTid);
    FI_HILOGD("AddPrivilege enter");
    int32_t ret = UDMF::UdmfClient::GetInstance().AddPrivilege(option, privilege);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send pid to Udmf client");
    }
#else
    (void)(targetTid);
    (void)(udKey);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

void DragManager::OnDragUp(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    FI_HILOGD("SourceType:%{public}d, pointerId:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId());
    int32_t pid = MMI::InputManager::GetInstance()->GetWindowPid(pointerEvent->GetTargetWindowId());
    FI_HILOGD("Target window drag pid:%{public}d", pid);

    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        dragDrawing_.EraseMouseIcon();
        MMI::InputManager::GetInstance()->SetPointerVisible(true);
    }
    int32_t targetTid = DRAG_DATA_MGR.GetTargetTid();
    FI_HILOGD("Target window drag tid: %{public}d", targetTid);
    SendDragData(targetTid, dragData.udKey);
    CHKPV(context_);
    timerId_ = context_->GetTimerManager().AddTimer(TIMEOUT_MS, 1, [this]() {
        this->StopDrag(DragResult::DRAG_EXCEPTION, false);
    });
}

void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    CALL_DEBUG_ENTER;
}

void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    CHKPV(callback_);
    CHKPV(context_);
    callback_(pointerEvent);
    pointerEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_NO_INTERCEPT);
    auto fun = [] (std::shared_ptr<MMI::PointerEvent> pointerEvent) -> int32_t {
        MMI::InputManager::GetInstance()->SimulateInputEvent(pointerEvent);
        if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
            MMI::InputManager::GetInstance()->AppendExtraData(DragManager::CreateExtraData(false));
        }
        return RET_OK;
    };
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(std::bind(fun, pointerEvent));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
}

void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
    CALL_DEBUG_ENTER;
}

void DragManager::Dump(int32_t fd) const
{
    CALL_DEBUG_ENTER;
    DragCursorStyle style = DRAG_DATA_MGR.GetDragStyle();
    int32_t targetTid = DRAG_DATA_MGR.GetTargetTid();
    dprintf(fd, "Drag information:\n");
    dprintf(fd,
            "dragState:%s | dragResult:%s | interceptorId:%d | dragTargetPid:%d | dragTargetTid:%d | "
            "cursorStyle:%s | isWindowVisble:%s\n", GetDragState(dragState_).c_str(),
            GetDragResult(dragResult_).c_str(), interceptorId_, GetDragTargetPid(), targetTid,
            GetDragCursorStyle(style).c_str(), DRAG_DATA_MGR.GetDragWindowVisible() ? "true" : "false");
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    std::string udKey;
    if (RET_ERR == GetUdKey(udKey)) {
        FI_HILOGE("Target udKey is empty");
        udKey = "";
    }
    dprintf(fd, "dragData = {\n"
            "\tshadowInfoX:%d\n\tshadowInfoY:%d\n\tudKey:%s\n\tsourceType:%d\n\tdragNum:%d\n\tpointerId:%d\n"
            "\tdisplayX:%d\n\tdisplayY:%d\n""\tdisplayId:%d\n\thasCanceledAnimation:%s\n",
            dragData.shadowInfo.x, dragData.shadowInfo.y, udKey.c_str(), dragData.sourceType, dragData.dragNum,
            dragData.pointerId, dragData.displayX, dragData.displayY, dragData.displayId,
            dragData.hasCanceledAnimation ? "true" : "false");
    if (dragState_ != DragState::STOP) {
        std::shared_ptr<OHOS::Media::PixelMap> pixelMap = dragData.shadowInfo.pixelMap;
        CHKPV(pixelMap);
        dprintf(fd, "\tpixelMapWidth:%d\n\tpixelMapHeight:%d\n", pixelMap->GetWidth(), pixelMap->GetHeight());
    }
    dprintf(fd, "}\n");
}

std::string DragManager::GetDragState(DragState value) const
{
    std::string state;
    switch (value) {
        case DragState::START: {
            state = "start";
            break;
        }
        case DragState::STOP: {
            state = "stop";
            break;
        }
        case DragState::CANCEL: {
            state = "cancel";
            break;
        }
        case DragState::ERROR: {
            state = "error";
            break;
        }
        default: {
            state = "unknown";
            FI_HILOGW("Drag status unknown");
            break;
        }
    }
    return state;
}

std::string DragManager::GetDragResult(DragResult value) const
{
    std::string result;
    switch (value) {
        case DragResult::DRAG_SUCCESS: {
            result = "success";
            break;
        }
        case DragResult::DRAG_FAIL: {
            result = "fail";
            break;
        }
        case DragResult::DRAG_CANCEL: {
            result = "cancel";
            break;
        }
        case DragResult::DRAG_EXCEPTION: {
            result = "abnormal";
            break;
        }
        default: {
            result = "unknown";
            FI_HILOGW("Drag result unknown");
            break;
        }
    }
    return result;
}

std::string DragManager::GetDragCursorStyle(DragCursorStyle value) const
{
    std::string style;
    switch (value) {
        case DragCursorStyle::COPY: {
            style = "copy";
            break;
        }
        case DragCursorStyle::DEFAULT: {
            style = "default";
            break;
        }
        case DragCursorStyle::FORBIDDEN: {
            style = "forbidden";
            break;
        }
        case DragCursorStyle::MOVE: {
            style = "move";
            break;
        }
        default: {
            style = "unknown";
            FI_HILOGW("Drag cursor style unknown");
            break;
        }
    }
    return style;
}

OHOS::MMI::ExtraData DragManager::CreateExtraData(bool appended)
{
    CALL_DEBUG_ENTER;
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    OHOS::MMI::ExtraData extraData;
    extraData.buffer = dragData.buffer;
    extraData.sourceType = dragData.sourceType;
    extraData.pointerId = dragData.pointerId;
    extraData.appended = appended;
    FI_HILOGD("sourceType:%{public}d, pointerId:%{public}d", extraData.sourceType, extraData.pointerId);
    return extraData;
}

int32_t DragManager::InitDataManager(const DragData &dragData) const
{
    CALL_DEBUG_ENTER;
    MMI::PointerStyle pointerStyle;
    if (MMI::InputManager::GetInstance()->GetPointerStyle(MMI::GLOBAL_WINDOW_ID, pointerStyle) != RET_OK) {
        FI_HILOGE("Get pointer style failed");
        return RET_ERR;
    }
    DRAG_DATA_MGR.Init(dragData, pointerStyle);
    return RET_OK;
}

int32_t DragManager::AddDragEventInterceptor(int32_t sourceType)
{
    CALL_DEBUG_ENTER;
    uint32_t deviceTags = 0;
    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        deviceTags = MMI::CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER);
    } else if (sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        deviceTags = MMI::CapabilityToTags(MMI::INPUT_DEV_CAP_TOUCH);
    } else {
        FI_HILOGW("Drag is not supported for this device type:%{public}d", sourceType);
        return RET_ERR;
    }
    auto callback = std::bind(&DragManager::DragCallback, this, std::placeholders::_1);
    auto interceptor = std::make_shared<InterceptorConsumer>(context_, callback);
    interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, DRAG_PRIORITY, deviceTags);
    if (interceptorId_ <= 0) {
        FI_HILOGE("Failed to add interceptor, Error code:%{public}d", interceptorId_);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragManager::OnStartDrag()
{
    auto extraData = CreateExtraData(true);
    MMI::InputManager::GetInstance()->AppendExtraData(extraData);
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    int32_t ret = dragDrawing_.Init(dragData);
    if (ret == INIT_FAIL) {
        FI_HILOGE("Init drag drawing failed");
        dragDrawing_.DestroyDragWindow();
        return RET_ERR;
    }
    if (ret == INIT_CANCEL) {
        FI_HILOGE("Init drag drawing cancel, drag animation is running");
        return RET_ERR;
    }
    dragDrawing_.Draw(dragData.displayId, dragData.displayX, dragData.displayY);
    ret = AddDragEventInterceptor(dragData.sourceType);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to add drag event interceptor");
        dragDrawing_.DestroyDragWindow();
        return RET_ERR;
    }
    if (dragData.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        MMI::InputManager::GetInstance()->SetPointerVisible(false);
    }
    return RET_OK;
}

int32_t DragManager::OnStopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    if (interceptorId_ <= 0) {
        FI_HILOGE("Invalid interceptor to be removed, interceptorId_:%{public}d", interceptorId_);
        return RET_ERR;
    }
    MMI::InputManager::GetInstance()->RemoveInterceptor(interceptorId_);
    interceptorId_ = -1;
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE && !DRAG_DATA_MGR.IsMotionDrag()) {
        dragDrawing_.EraseMouseIcon();
        MMI::InputManager::GetInstance()->SetPointerVisible(true);
    }
    switch (result) {
        case DragResult::DRAG_SUCCESS: {
            if (!hasCustomAnimation) {
                dragDrawing_.OnDragSuccess();
            } else {
                dragDrawing_.DestroyDragWindow();
                dragDrawing_.UpdateDrawingState();
            }
            break;
        }
        case DragResult::DRAG_FAIL:
        case DragResult::DRAG_CANCEL: {
            if (!hasCustomAnimation) {
                dragDrawing_.OnDragFail();
            } else {
                dragDrawing_.DestroyDragWindow();
                dragDrawing_.UpdateDrawingState();
            }
            break;
        }
        case DragResult::DRAG_EXCEPTION: {
            dragDrawing_.DestroyDragWindow();
            dragDrawing_.UpdateDrawingState();
            break;
        }
        default: {
            FI_HILOGW("Unsupported DragResult type, DragResult:%{public}d", result);
            break;
        }
    }
    return RET_OK;
}

int32_t DragManager::OnSetDragWindowVisible(bool visible)
{
    DRAG_DATA_MGR.SetDragWindowVisible(visible);
    dragDrawing_.UpdateDragWindowState(visible);
    return RET_OK;
}

int32_t DragManager::OnGetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height)
{
    return DRAG_DATA_MGR.GetShadowOffset(offsetX, offsetY, width, height);
}

void DragManager::RegisterStateChange(std::function<void(DragState)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    stateChangedCallback_ = callback;
}

void DragManager::StateChangedNotify(DragState state)
{
    CALL_DEBUG_ENTER;
    if ((stateChangedCallback_ != nullptr) && (!DRAG_DATA_MGR.IsMotionDrag())) {
        stateChangedCallback_(state);
    }
}

void DragManager::MoveTo(int32_t xPercent, int32_t yPercent)
{
    CALL_DEBUG_ENTER;
    if (xPercent < 0 || xPercent >= 100 || yPercent < 0 || yPercent >= 100) {
        FI_HILOGE("Param error");
        return;
    }
    auto display = OHOS::Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPV(display);
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    int32_t x = static_cast<int32_t>(width * (xPercent / PERCENT_CONST));
    int32_t y = static_cast<int32_t>(height * (yPercent / PERCENT_CONST));
    dragDrawing_.MoveTo(x, y);
}

OHOS::MMI::ExtraData DragManager::GetExtraData(bool appended) const
{
    return CreateExtraData(appended);
}

DragState DragManager::GetDragState() const
{
    CALL_DEBUG_ENTER;
    return dragState_;
}

DragResult DragManager::GetDragResult() const
{
    CALL_DEBUG_ENTER;
    return dragResult_;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
