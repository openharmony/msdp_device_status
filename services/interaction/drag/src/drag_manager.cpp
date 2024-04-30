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

#include <atomic>

#include "display_manager.h"
#include "extra_data.h"
#include "hitrace_meter.h"
#include "pixel_map.h"
#include "udmf_client.h"
#include "unified_types.h"
#include "window_manager.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "drag_data_manager.h"
#include "drag_hisysevent.h"
#include "fi_log.h"
#include "proto.h"

#undef LOG_TAG
#define LOG_TAG "DragManager"
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t TIMEOUT_MS { 3000 };
constexpr int32_t INTERVAL_MS { 500 };
constexpr uint64_t FOLD_SCREEN_ID { 5 };
std::atomic<int64_t> g_startFilterTime { -1 };
const std::string DRAG_STYLE_DEFAULT {"DEFAULT"};
const std::string DRAG_STYLE_FORBIDDEN {"FORBIDDEN"};
const std::string DRAG_STYLE_COPY {"COPY"};
const std::string DRAG_STYLE_MOVE {"MOVE"};
const std::string DRAG_STYLE_UNKNOW {"UNKNOW"};
#ifdef OHOS_DRAG_ENABLE_INTERCEPTOR
constexpr int32_t DRAG_PRIORITY { 500 };
#endif // OHOS_DRAG_ENABLE_INTERCEPTOR
} // namespace

DragManager::~DragManager()
{
    EventHub::UnRegisterEvent(eventHub_);
}

int32_t DragManager::Init(IContext* context)
{
    FI_HILOGI("enter");
    CHKPR(context, RET_ERR);
    context_ = context;
    int32_t repeatCount = 1;
    context_->GetTimerManager().AddTimer(INTERVAL_MS, repeatCount, [this]() {
        if (eventHub_ == nullptr) {
            eventHub_ = EventHub::GetEventHub(context_);
        }
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            FI_HILOGE("samgrProxy is nullptr");
            return;
        }
        statusListener_ = new (std::nothrow) DragAbilityStatusChange(eventHub_);
        if (statusListener_ == nullptr) {
            FI_HILOGE("statusListener_ is nullptr");
            return;
        }
        int32_t ret = samgrProxy->SubscribeSystemAbility(COMMON_EVENT_SERVICE_ID, statusListener_);
        FI_HILOGI("SubscribeSystemAbility COMMON_EVENT_SERVICE_ID result:%{public}d", ret);
        displayAbilityStatusChange_ = new (std::nothrow) DisplayAbilityStatusChange(context_);
        if (displayAbilityStatusChange_ == nullptr) {
            FI_HILOGE("displayAbilityStatusChange is nullptr");
            return;
        }
        ret = samgrProxy->SubscribeSystemAbility(DISPLAY_MANAGER_SERVICE_SA_ID, displayAbilityStatusChange_);
        FI_HILOGI("SubscribeSystemAbility DISPLAY_MANAGER_SERVICE_SA_ID result:%{public}d", ret);
    });
    FI_HILOGI("leave");
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
void DragManager::OnSessionLost(SocketSessionPtr session)
{
    CHKPV(session);
    RemoveListener(session->GetPid());
}
#else
void DragManager::OnSessionLost(SessionPtr session)
{
    FI_HILOGI("enter");
    if (RemoveListener(session) != RET_OK) {
        FI_HILOGE("Failed to clear client listener");
    }
    FI_HILOGI("leave");
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DragManager::AddListener(int32_t pid)
{
    FI_HILOGI("enter");
    CHKPR(context_, RET_ERR);
    auto session = context_->GetSocketSessionManager().FindSessionByPid(pid);
#else
int32_t DragManager::AddListener(SessionPtr session)
{
    FI_HILOGI("enter");
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    info->msgId = MessageId::DRAG_STATE_LISTENER;
    info->msgType = MessageType::NOTIFY_STATE;
    stateNotify_.AddNotifyMsg(info);
    FI_HILOGI("leave");
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DragManager::RemoveListener(int32_t pid)
{
    FI_HILOGI("enter");
    CHKPR(context_, RET_ERR);
    auto session = context_->GetSocketSessionManager().FindSessionByPid(pid);
#else
int32_t DragManager::RemoveListener(SessionPtr session)
{
    FI_HILOGI("enter");
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    info->msgType = MessageType::NOTIFY_STATE;
    stateNotify_.RemoveNotifyMsg(info);
    FI_HILOGI("leave");
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DragManager::AddSubscriptListener(int32_t pid)
{
    FI_HILOGI("enter");
    CHKPR(context_, RET_ERR);
    auto session = context_->GetSocketSessionManager().FindSessionByPid(pid);
#else
int32_t DragManager::AddSubscriptListener(SessionPtr session)
{
    FI_HILOGI("enter");
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->session = session;
    info->msgId = MessageId::DRAG_STYLE_LISTENER;
    info->msgType = MessageType::NOTIFY_STYLE;
    stateNotify_.AddNotifyMsg(info);
    FI_HILOGI("leave");
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DragManager::RemoveSubscriptListener(int32_t pid)
{
    FI_HILOGI("enter");
    CHKPR(context_, RET_ERR);
    auto session = context_->GetSocketSessionManager().FindSessionByPid(pid);
#else
int32_t DragManager::RemoveSubscriptListener(SessionPtr session)
{
    FI_HILOGI("enter");
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    CHKPR(session, RET_ERR);
    auto info = std::make_shared<StateChangeNotify::MessageInfo>();
    info->msgType = MessageType::NOTIFY_STYLE;
    info->session = session;
    stateNotify_.RemoveNotifyMsg(info);
    FI_HILOGI("leave");
    return RET_OK;
}

void DragManager::PrintDragData(const DragData &dragData, const std::string &packageName)
{
    FI_HILOGI("enter");
    for (const auto& shadowInfo : dragData.shadowInfos) {
        CHKPV(shadowInfo.pixelMap);
        FI_HILOGI("PixelFormat:%{public}d, PixelAlphaType:%{public}d, PixelAllocatorType:%{public}d,"
            " PixelWidth:%{public}d, PixelHeight:%{public}d, shadowX:%{public}d, shadowY:%{public}d",
            static_cast<int32_t>(shadowInfo.pixelMap->GetPixelFormat()),
            static_cast<int32_t>(shadowInfo.pixelMap->GetAlphaType()),
            static_cast<int32_t>(shadowInfo.pixelMap->GetAllocatorType()),
            shadowInfo.pixelMap->GetWidth(), shadowInfo.pixelMap->GetHeight(), shadowInfo.x, shadowInfo.y);
    }
    std::string summarys;
    for (const auto &[udKey, recordSize] : dragData.summarys) {
        std::string str = udKey + "-" + std::to_string(recordSize) + ";";
        summarys += str;
    }
    FI_HILOGI("SourceType:%{public}d, pointerId:%{public}d, displayId:%{public}d,"
        " displayX:%{public}d, displayY:%{public}d, dragNum:%{public}d,"
        " hasCanceledAnimation:%{public}d, udKey:%{public}s, hasCoordinateCorrected:%{public}d, summarys:%{public}s,"
        " packageName:%{public}s", dragData.sourceType, dragData.pointerId, dragData.displayId, dragData.displayX,
        dragData.displayY, dragData.dragNum, dragData.hasCanceledAnimation, GetAnonyString(dragData.udKey).c_str(),
        dragData.hasCoordinateCorrected, summarys.c_str(), packageName.c_str());
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DragManager::StartDrag(const DragData &dragData, int32_t pid)
#else
int32_t DragManager::StartDrag(const DragData &dragData, SessionPtr sess)
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
{
    FI_HILOGI("enter");
    if (dragState_ == DragState::START) {
        FI_HILOGE("Drag instance already exists, no need to start drag again");
        return RET_ERR;
    }
    std::string packageName = std::string();
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    CHKPR(context_, RET_ERR);
    dragOutSession_ = context_->GetSocketSessionManager().FindSessionByPid(pid);
    if (dragOutSession_ != nullptr) {
        context_->GetSocketSessionManager().AddSessionDeletedCallback(pid,
            std::bind(&DragManager::OnSessionLost, this, std::placeholders::_1));
    }
    packageName = (pid == -1) ? "Cross-device drag" : dragOutSession_->GetProgramName();
#else
    dragOutSession_ = sess;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    PrintDragData(dragData, packageName);
    if (InitDataManager(dragData) != RET_OK) {
        FI_HILOGE("Failed to init data manager");
        return RET_ERR;
    }
    if (OnStartDrag() != RET_OK) {
        DragDFX::WriteStartDrag(dragState_, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        FI_HILOGE("Failed to execute OnStartDrag");
        return RET_ERR;
    }
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
    if (notifyPUllUpCallback_ != nullptr) {
        notifyPUllUpCallback_(false);
    }
#endif
    SetDragState(DragState::START);
    stateNotify_.StateChangedNotify(DragState::START);
    StateChangedNotify(DragState::START);
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::StopDrag(const DragDropResult &dropResult, const std::string &packageName)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    std::string dragOutPkgName =
        (dragOutSession_ == nullptr) ? "Cross-device drag" : dragOutSession_->GetProgramName();
    FI_HILOGI("mainWindow:%{public}d, dragResult:%{public}d, drop packageName:%{public}s,"
        "drag Out packageName:%{public}s", dropResult.mainWindow, dropResult.result, packageName.c_str(),
        dragOutPkgName.c_str());
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    if (dragState_ == DragState::STOP) {
        FI_HILOGE("No drag instance running, can not stop drag");
        return RET_ERR;
    }
    if ((dropResult.result != DragResult::DRAG_EXCEPTION) && (context_ != nullptr) && (timerId_ >= 0)) {
        context_->GetTimerManager().RemoveTimer(timerId_);
        timerId_ = -1;
    }
    int32_t ret = RET_OK;
    if (OnStopDrag(dropResult.result, dropResult.hasCustomAnimation) != RET_OK) {
        DragDFX::WriteStopDrag(dragState_, dropResult, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        FI_HILOGE("On stop drag failed");
        ret = RET_ERR;
    }
    if (dropResult.result == DragResult::DRAG_SUCCESS && dropResult.mainWindow > 0) {
        Rosen::WMError result = Rosen::WindowManager::GetInstance().RaiseWindowToTop(dropResult.mainWindow);
        if (result != Rosen::WMError::WM_OK) {
            FI_HILOGE("Raise window to top failed, mainWindow:%{public}d", dropResult.mainWindow);
        }
    }
    stateNotify_.StateChangedNotify(DragState::STOP);
    DragBehavior dragBehavior = dropResult.dragBehavior;
    GetDragBehavior(dropResult, dragBehavior);
    if (NotifyDragResult(dropResult.result, dragBehavior) != RET_OK) {
        FI_HILOGE("Notify drag result failed");
    }
    DRAG_DATA_MGR.ResetDragData();
    dragResult_ = static_cast<DragResult>(dropResult.result);
    StateChangedNotify(DragState::STOP);
    SetDragState(DragState::STOP);
    if (isControlMultiScreenVisible_) {
        isControlMultiScreenVisible_ = false;
    }
    FI_HILOGI("leave");
    return ret;
}

int32_t DragManager::GetDragTargetPid() const
{
    FI_HILOGI("enter");
    return DRAG_DATA_MGR.GetTargetPid();
}

int32_t DragManager::GetUdKey(std::string &udKey) const
{
    FI_HILOGI("enter");
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.udKey.empty()) {
        FI_HILOGE("Target udKey is empty");
        return RET_ERR;
    }
    udKey = dragData.udKey;
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::UpdateDragStyle(DragCursorStyle style, int32_t targetPid, int32_t targetTid)
{
    FI_HILOGD("DragStyle from ark is dragStyle%{public}s", GetDragStyleName(style).c_str());
    auto lastTargetPid = DRAG_DATA_MGR.GetTargetPid();
    DRAG_DATA_MGR.SetTargetPid(targetPid);
    DRAG_DATA_MGR.SetTargetTid(targetTid);
    if (style == DRAG_DATA_MGR.GetDragStyle()) {
        FI_HILOGD("Not need update drag style");
        if (targetPid != lastTargetPid) {
            stateNotify_.StyleChangedNotify(GetRealDragStyle(style));
        }
        return RET_OK;
    }
    DRAG_DATA_MGR.SetDragStyle(style);
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running, can not update drag style");
        return RET_ERR;
    }
    if ((style < DragCursorStyle::DEFAULT) || (style > DragCursorStyle::MOVE)) {
        DragDFX::WriteUpdateDragStyle(style, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if (OnUpdateDragStyle(style) != RET_OK) {
        FI_HILOGE("OnUpdateDragStyle dragStyle%{public}s failed", GetDragStyleName(style).c_str());
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragManager::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    FI_HILOGI("enter");
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running, can not update shadow picture");
        return RET_ERR;
    }
    DRAG_DATA_MGR.SetShadowInfos({ shadowInfo });
    FI_HILOGI("leave");
    return dragDrawing_.UpdateShadowPic(shadowInfo);
}

int32_t DragManager::GetDragData(DragData &dragData)
{
    FI_HILOGI("enter");
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running, can not get dragData");
        return RET_ERR;
    }
    dragData = DRAG_DATA_MGR.GetDragData();
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::GetDragState(DragState &dragState)
{
    FI_HILOGD("enter");
    dragState = GetDragState();
    if (dragState == DragState::ERROR) {
        FI_HILOGE("dragState_ is error");
        return RET_ERR;
    }
    FI_HILOGD("leave");
    return RET_OK;
}

int32_t DragManager::NotifyDragResult(DragResult result, DragBehavior dragBehavior)
{
    FI_HILOGI("enter");
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    int32_t targetPid = GetDragTargetPid();
    NetPacket pkt(MessageId::DRAG_NOTIFY_RESULT);
    if ((result < DragResult::DRAG_SUCCESS) || (result > DragResult::DRAG_EXCEPTION)) {
        DragDFX::WriteNotifyDragResult(result, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        FI_HILOGE("The invalid result:%{public}d", static_cast<int32_t>(result));
        return RET_ERR;
    }
    pkt << dragData.displayX << dragData.displayY << static_cast<int32_t>(result) << targetPid <<
        static_cast<int32_t>(dragBehavior);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Failed to packet write data");
        return RET_ERR;
    }
    CHKPR(dragOutSession_, RET_ERR);
    if (!dragOutSession_->SendMsg(pkt)) {
        FI_HILOGE("Failed to send message");
        return MSG_SEND_FAIL;
    }
    DragDFX::WriteNotifyDragResult(result, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR);
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::NotifyHideIcon()
{
    FI_HILOGD("enter");
    NetPacket pkt(MessageId::DRAG_NOTIFY_HIDE_ICON);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    CHKPR(dragOutSession_, RET_ERR);
    if (!dragOutSession_->SendMsg(pkt)) {
        FI_HILOGE("Send message failed");
        return MSG_SEND_FAIL;
    }
    FI_HILOGD("leave");
    return RET_OK;
}

void DragManager::DragCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CHKPV(pointerEvent);
    int32_t pointerAction = pointerEvent->GetPointerAction();
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
        OnDragMove(pointerEvent);
        return;
    }
    FI_HILOGI("DragCallback, pointerAction:%{public}d", pointerAction);
    if (pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        CHKPV(context_);
#ifdef OHOS_DRAG_ENABLE_ANIMATION
        dragDrawing_.NotifyDragInfo(DragEvent::DRAG_UP, pointerEvent->GetPointerId());
#endif // OHOS_DRAG_ENABLE_ANIMATION
        int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
            std::bind(&DragManager::OnDragUp, this, pointerEvent));
        if (ret != RET_OK) {
            FI_HILOGE("Post async task failed");
        }

        return;
    }
    FI_HILOGD("Unknown action, sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerAction);
}

void DragManager::OnDragMove(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CHKPV(pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    int32_t pointerId = pointerEvent->GetPointerId();
    int32_t displayX = pointerItem.GetDisplayX();
    int32_t displayY = pointerItem.GetDisplayY();
    FI_HILOGD("SourceType:%{public}d, pointerId:%{public}d, displayX:%{public}d, displayY:%{public}d",
        pointerEvent->GetSourceType(), pointerId, displayX, displayY);
#ifdef OHOS_DRAG_ENABLE_ANIMATION
    dragDrawing_.NotifyDragInfo(DragEvent::DRAG_MOVE, pointerId, displayX, displayY);
#endif // OHOS_DRAG_ENABLE_ANIMATION
    dragDrawing_.Draw(pointerEvent->GetTargetDisplayId(), displayX, displayY);
}

void DragManager::SendDragData(int32_t targetTid, const std::string &udKey)
{
    FI_HILOGI("enter");
    UDMF::QueryOption option;
    option.key = udKey;
    UDMF::Privilege privilege;
    privilege.tokenId = static_cast<uint32_t>(targetTid);
    int32_t ret = UDMF::UdmfClient::GetInstance().AddPrivilege(option, privilege);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send pid to Udmf client");
    }
    FI_HILOGI("leave");
}

int32_t DragManager::OnDragUp(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    FI_HILOGI("enter");
    CHKPR(pointerEvent, RET_ERR);
    CHKPR(notifyPUllUpCallback_, RET_ERR);
    notifyPUllUpCallback_(true);
    if (dragState_ != DragState::START) {
        FI_HILOGW("No drag instance running");
        return RET_ERR;
    }
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        dragDrawing_.EraseMouseIcon();
        FI_HILOGI("Set the pointer cursor visible");
        MMI::InputManager::GetInstance()->SetPointerVisible(true);
    }
    CHKPR(context_, RET_ERR);
    int32_t repeatCount = 1;
    timerId_ = context_->GetTimerManager().AddTimer(TIMEOUT_MS, repeatCount, [this]() {
        DragDropResult dropResult { DragResult::DRAG_EXCEPTION, false, -1 };
        FI_HILOGW("Timeout, automatically stop dragging");
        this->StopDrag(dropResult);
    });
    FI_HILOGI("leave");
    return RET_OK;
}

#ifdef OHOS_DRAG_ENABLE_INTERCEPTOR
void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
}

void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    if (g_startFilterTime > 0) {
        auto actionTime = pointerEvent->GetActionTime();
        if (g_startFilterTime >= actionTime
            && pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE) {
            FI_HILOGW("Invalid event");
            return;
        }
        g_startFilterTime = -1;
    }
    CHKPV(pointerEventCallback_);
    pointerEventCallback_(pointerEvent);
    pointerEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_NO_INTERCEPT);
    MMI::InputManager::GetInstance()->SimulateInputEvent(pointerEvent);
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        FI_HILOGI("Pointer button is released, appened extra data");
        MMI::InputManager::GetInstance()->AppendExtraData(DragManager::CreateExtraData(false));
    }
}

void DragManager::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}
#endif // OHOS_DRAG_ENABLE_INTERCEPTOR

#ifdef OHOS_DRAG_ENABLE_MONITOR
void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    FI_HILOGD("enter");
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    FI_HILOGD("enter");
    CHKPV(pointerEvent);
    CHKPV(pointerEventCallback_);
    pointerEventCallback_(pointerEvent);
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_PULL_UP) {
        FI_HILOGI("Pointer button is released, appened extra data");
        MMI::InputManager::GetInstance()->AppendExtraData(DragManager::CreateExtraData(false));
    }
    FI_HILOGD("leave");
}

void DragManager::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
    FI_HILOGD("enter");
}
#endif // OHOS_DRAG_ENABLE_MONITOR

void DragManager::Dump(int32_t fd) const
{
    DragCursorStyle style = DRAG_DATA_MGR.GetDragStyle();
    int32_t targetTid = DRAG_DATA_MGR.GetTargetTid();
    dprintf(fd, "Drag information:\n");
#ifdef OHOS_DRAG_ENABLE_INTERCEPTOR
    dprintf(fd,
            "dragState:%s | dragResult:%s | interceptorId:%d | dragTargetPid:%d | dragtargetTid:%d | "
            "cursorStyle:%s | isWindowVisble:%s\n", GetDragState(dragState_).c_str(),
            GetDragResult(dragResult_).c_str(), pointerEventInterceptorId_, GetDragTargetPid(), targetTid,
            GetDragCursorStyle(style).c_str(), DRAG_DATA_MGR.GetDragWindowVisible() ? "true" : "false");
#endif // OHOS_DRAG_ENABLE_INTERCEPTOR
#ifdef OHOS_DRAG_ENABLE_MONITOR
    dprintf(fd,
            "dragState:%s | dragResult:%s | monitorId:%d | dragTargetPid:%d | dragtargetTid:%d | "
            "cursorStyle:%s | isWindowVisble:%s\n", GetDragState(dragState_).c_str(),
            GetDragResult(dragResult_).c_str(), pointerEventMonitorId_, GetDragTargetPid(), targetTid,
            GetDragCursorStyle(style).c_str(), DRAG_DATA_MGR.GetDragWindowVisible() ? "true" : "false");
#endif // OHOS_DRAG_ENABLE_MONITOR
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    std::string udKey;
    if (RET_ERR == GetUdKey(udKey)) {
        FI_HILOGE("Target udKey is empty");
        udKey = "";
    }
    for (const auto& shadowInfo : dragData.shadowInfos) {
        dprintf(fd, "dragData = {\n""\tshadowInfoX:%d\n\tshadowInfoY\n", shadowInfo.x, shadowInfo.y);
    }
    dprintf(fd, "dragData = {\n"
            "\tudKey:%s\n\tfilterInfo:%s\n\textraInfo:%s\n\tsourceType:%d"
            "\tdragNum:%d\n\tpointerId:%d\n\tdisplayX:%d\n\tdisplayY:%d\n""\tdisplayId:%d\n\thasCanceledAnimation:%s\n",
            GetAnonyString(dragData.udKey).c_str(), dragData.filterInfo.c_str(), dragData.extraInfo.c_str(),
            dragData.sourceType, dragData.dragNum, dragData.pointerId, dragData.displayX, dragData.displayY,
            dragData.displayId, dragData.hasCanceledAnimation ? "true" : "false");
    if (dragState_ != DragState::STOP) {
        for (const auto& shadowInfo : dragData.shadowInfos) {
            CHKPV(shadowInfo.pixelMap);
            dprintf(fd, "\tpixelMapWidth:%d\n\tpixelMapHeight:%d\n", shadowInfo.pixelMap->GetWidth(),
                shadowInfo.pixelMap->GetHeight());
        }
    }
    dprintf(fd, "}\n");
}

std::string DragManager::GetDragState(DragState value) const
{
    std::string state = "unknown";
    const std::map<DragState, std::string> dragStates = {
        { DragState::START, "start" },
        { DragState::STOP, "stop" },
        { DragState::CANCEL, "cancel" },
        { DragState::ERROR, "error" }
    };
    auto iter = dragStates.find(value);
    if (iter != dragStates.end()) {
        state = iter->second;
    }
    return state;
}

std::string DragManager::GetDragResult(DragResult value) const
{
    std::string result = "unknown";
    const std::map<DragResult, std::string> dragResults = {
        { DragResult::DRAG_SUCCESS, "success" },
        { DragResult::DRAG_FAIL, "fail" },
        { DragResult::DRAG_CANCEL, "cancel" },
        { DragResult::DRAG_EXCEPTION, "abnormal" }
    };
    auto iter = dragResults.find(value);
    if (iter != dragResults.end()) {
        result = iter->second;
    }
    return result;
}

std::string DragManager::GetDragCursorStyle(DragCursorStyle value) const
{
    std::string style = "unknown";
    const std::map<DragCursorStyle, std::string> cursorStyles = {
        { DragCursorStyle::COPY, "copy" },
        { DragCursorStyle::DEFAULT, "default" },
        { DragCursorStyle::FORBIDDEN, "forbidden" },
        { DragCursorStyle::MOVE, "move" }
    };
    auto iter = cursorStyles.find(value);
    if (iter != cursorStyles.end()) {
        style = iter->second;
    }
    return style;
}

MMI::ExtraData DragManager::CreateExtraData(bool appended)
{
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    MMI::ExtraData extraData;
    extraData.buffer = dragData.buffer;
    extraData.sourceType = dragData.sourceType;
    extraData.pointerId = dragData.pointerId;
    extraData.appended = appended;
    FI_HILOGD("sourceType:%{public}d, pointerId:%{public}d", extraData.sourceType, extraData.pointerId);
    return extraData;
}

int32_t DragManager::InitDataManager(const DragData &dragData) const
{
    FI_HILOGI("enter");
    DRAG_DATA_MGR.Init(dragData);
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::AddDragEventHandler(int32_t sourceType)
{
    FI_HILOGI("enter");
    uint32_t deviceTags = 0;
#ifdef OHOS_DRAG_ENABLE_INTERCEPTOR
    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        deviceTags = MMI::CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER);
    } else if (sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        deviceTags = MMI::CapabilityToTags(MMI::INPUT_DEV_CAP_TOUCH) |
            MMI::CapabilityToTags(MMI::INPUT_DEV_CAP_TABLET_TOOL);
    } else {
        FI_HILOGW("Drag is not supported for this device type:%{public}d", sourceType);
        return RET_ERR;
    }
#endif // OHOS_DRAG_ENABLE_INTERCEPTOR
    if (AddPointerEventHandler(deviceTags) != RET_OK) {
        FI_HILOGE("Failed to add pointer event handler");
        return RET_ERR;
    }
    if (AddKeyEventMonitor() != RET_OK) {
        FI_HILOGE("Failed to add key event handler");
        return RET_ERR;
    }
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::AddPointerEventHandler(uint32_t deviceTags)
{
    FI_HILOGI("enter");
#ifdef OHOS_DRAG_ENABLE_MONITOR
    auto monitor = std::make_shared<MonitorConsumer>(std::bind(&DragManager::DragCallback, this,
        std::placeholders::_1));
    pointerEventMonitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor);
    if (pointerEventMonitorId_ <= 0) {
        FI_HILOGE("Failed to add pointer event monitor");
        return RET_ERR;
    }
#else
    auto callback = std::bind(&DragManager::DragCallback, this, std::placeholders::_1);
    auto interceptor = std::make_shared<InterceptorConsumer>(callback);
    pointerEventInterceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(
        interceptor, DRAG_PRIORITY, deviceTags);
    if (pointerEventInterceptorId_ <= 0) {
        FI_HILOGE("Failed to add pointer event interceptor");
        return RET_ERR;
    }
#endif // OHOS_DRAG_ENABLE_MONITOR
    FI_HILOGI("Add drag poniter event handle successfully");
    return RET_OK;
}

int32_t DragManager::AddKeyEventMonitor()
{
    FI_HILOGI("enter");
    keyEventMonitorId_ = MMI::InputManager::GetInstance()->AddMonitor(
        std::bind(&DragManager::DragKeyEventCallback, this, std::placeholders::_1));
    if (keyEventMonitorId_ <= 0) {
        FI_HILOGE("Failed to add key event monitor");
        return RET_ERR;
    }
    FI_HILOGI("Add drag key event monitor successfully");
    return RET_OK;
}

int32_t DragManager::RemovePointerEventHandler()
{
    FI_HILOGI("enter");
#ifdef OHOS_DRAG_ENABLE_MONITOR
    if (pointerEventMonitorId_ <= 0) {
        FI_HILOGE("Invalid pointer event monitor id:%{public}d", pointerEventMonitorId_);
        return RET_ERR;
    }
    MMI::InputManager::GetInstance()->RemoveMonitor(pointerEventMonitorId_);
    pointerEventMonitorId_ = -1;
#else
    if (pointerEventInterceptorId_ <= 0) {
        FI_HILOGE("Invalid pointer event interceptor id:%{public}d", pointerEventInterceptorId_);
    }
    MMI::InputManager::GetInstance()->RemoveInterceptor(pointerEventInterceptorId_);
    pointerEventInterceptorId_ = -1;
#endif // OHOS_DRAG_ENABLE_MONITOR
    FI_HILOGI("Remove drag pointer event handler successfully");
    return RET_OK;
}

int32_t DragManager::RemoveKeyEventMonitor()
{
    FI_HILOGI("enter");
    if (keyEventMonitorId_ <= 0) {
        FI_HILOGE("Invalid key event monitor id:%{public}d", keyEventMonitorId_);
        return RET_ERR;
    }
    MMI::InputManager::GetInstance()->RemoveMonitor(keyEventMonitorId_);
    keyEventMonitorId_ = -1;
    FI_HILOGI("Remove drag key event handle successfully");
    return RET_OK;
}

int32_t DragManager::OnStartDrag()
{
    FI_HILOGI("enter");
    if (isControlMultiScreenVisible_) {
        isControlMultiScreenVisible_ = false;
    }
    auto extraData = CreateExtraData(true);
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    dragDrawing_.SetScreenId(dragData.displayId);
    if (Rosen::DisplayManager::GetInstance().IsFoldable()) {
        Rosen::FoldDisplayMode foldMode = Rosen::DisplayManager::GetInstance().GetFoldDisplayMode();
        if (foldMode == Rosen::FoldDisplayMode::MAIN) {
            dragDrawing_.SetScreenId(FOLD_SCREEN_ID);
        }
    }
    int32_t ret = dragDrawing_.Init(dragData);
    if (ret == INIT_FAIL) {
        FI_HILOGE("Init drag drawing failed");
        dragDrawing_.DestroyDragWindow();
        dragDrawing_.UpdateDrawingState();
        return RET_ERR;
    }
    if (ret == INIT_CANCEL) {
        FI_HILOGE("Init drag drawing cancel, drag animation is running");
        return RET_ERR;
    }
    dragDrawing_.Draw(dragData.displayId, dragData.displayX, dragData.displayY);
    FI_HILOGI("Start drag, appened extra data");
    MMI::InputManager::GetInstance()->AppendExtraData(extraData);
    ret = AddDragEventHandler(dragData.sourceType);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to add drag event handler");
        dragDrawing_.DestroyDragWindow();
        dragDrawing_.UpdateDrawingState();
        return RET_ERR;
    }
    dragAction_.store(DragAction::MOVE);
    FI_HILOGI("leave");
    return RET_OK;
}

int32_t DragManager::OnStopDrag(DragResult result, bool hasCustomAnimation)
{
    FI_HILOGI("Add custom animation:%{public}s", hasCustomAnimation ? "true" : "false");
    if (RemovePointerEventHandler() != RET_OK) {
        FI_HILOGE("Failed to remove pointer event handler");
        return RET_ERR;
    }
    if (RemoveKeyEventMonitor() != RET_OK) {
        FI_HILOGE("Failed to remove key event handler");
        return RET_ERR;
    }
    dragAction_.store(DragAction::MOVE);
    FI_HILOGI("Stop drag, appened extra data");
    MMI::InputManager::GetInstance()->AppendExtraData(DragManager::CreateExtraData(false));
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        dragDrawing_.EraseMouseIcon();
        if (dragState_ != DragState::MOTION_DRAGGING) {
            FI_HILOGI("Set the pointer cursor visible");
            MMI::InputManager::GetInstance()->SetPointerVisible(true);
        }
    }
    return HandleDragResult(result, hasCustomAnimation);
}

int32_t DragManager::OnSetDragWindowVisible(bool visible, bool isForce)
{
    FI_HILOGI("Set drag window visibleion:%{public}s", visible ? "true" : "false");
    if (dragState_ == DragState::MOTION_DRAGGING) {
        FI_HILOGW("Currently in motion dragging");
        return RET_OK;
    }
    if (dragState_ == DragState::STOP) {
        FI_HILOGW("No drag instance running, can not set drag window visible");
        return RET_ERR;
    }
    if (!isForce && isControlMultiScreenVisible_) {
        FI_HILOGW("The drag-and-drop window is controlled by multi-screen coordination,"
            "can not set drag window visible:%{public}d", visible);
        return RET_OK;
    }
    DragDFX::WriteDragWindowVisible(dragState_, visible, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR);
    DRAG_DATA_MGR.SetDragWindowVisible(visible);
    dragDrawing_.UpdateDragWindowState(visible);
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE && visible) {
        FI_HILOGI("Set the pointer cursor invisible");
        MMI::InputManager::GetInstance()->SetPointerVisible(false);
    }
    if (isForce) {
        isControlMultiScreenVisible_ = isForce;
        FI_HILOGW("The drag-and-drop window is controlled by multi-screen coordination");
    }
    return RET_OK;
}

int32_t DragManager::OnGetShadowOffset(ShadowOffset &shadowOffset)
{
    return DRAG_DATA_MGR.GetShadowOffset(shadowOffset);
}

void DragManager::RegisterStateChange(std::function<void(DragState)> callback)
{
    FI_HILOGI("enter");
    CHKPV(callback);
    stateChangedCallback_ = callback;
    FI_HILOGI("leave");
}

void DragManager::RegisterNotifyPullUp(std::function<void(bool)> callback)
{
    FI_HILOGI("enter");
    CHKPV(callback);
    notifyPUllUpCallback_ = callback;
    FI_HILOGI("leave");
}

void DragManager::StateChangedNotify(DragState state)
{
    FI_HILOGD("enter");
    CHKPV(stateChangedCallback_);
    if (state == DragState::STOP) {
        stateChangedCallback_(state);
    } else if (dragState_ != DragState::MOTION_DRAGGING) {
        stateChangedCallback_(state);
    }
    FI_HILOGD("leave");
}

MMI::ExtraData DragManager::GetExtraData(bool appended) const
{
    return CreateExtraData(appended);
}

DragState DragManager::GetDragState() const
{
    return dragState_;
}

void DragManager::GetAllowDragState(bool &isAllowDrag)
{
    FI_HILOGD("enter");
    if (dragState_ != DragState::START) {
        FI_HILOGW("Currently state is \'%{public}d\' not in allowed dragState", static_cast<int32_t>(dragState_));
        return;
    }
    isAllowDrag = dragDrawing_.GetAllowDragState();
    FI_HILOGD("leave");
}

void DragManager::SetDragState(DragState state)
{
    FI_HILOGI("SetDragState:%{public}d to %{public}d", static_cast<int32_t>(dragState_), static_cast<int32_t>(state));
    dragState_ = state;
    if (state == DragState::START) {
        UpdateDragStyleCross();
    }
}

DragResult DragManager::GetDragResult() const
{
    return dragResult_;
}

int32_t DragManager::GetDragSummary(std::map<std::string, int64_t> &summarys)
{
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    summarys = dragData.summarys;
    if (summarys.empty()) {
        FI_HILOGD("Summarys is empty");
    }
    return RET_OK;
}

int32_t DragManager::HandleDragResult(DragResult result, bool hasCustomAnimation)
{
    FI_HILOGI("enter");
    switch (result) {
        case DragResult::DRAG_SUCCESS: {
            if (!hasCustomAnimation) {
                dragDrawing_.OnDragSuccess(context_);
            } else {
                dragDrawing_.DestroyDragWindow();
                dragDrawing_.UpdateDrawingState();
            }
            break;
        }
        case DragResult::DRAG_FAIL:
        case DragResult::DRAG_CANCEL: {
            if (!hasCustomAnimation) {
                dragDrawing_.OnDragFail(context_);
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
    FI_HILOGI("leave");
    return RET_OK;
}

void DragManager::SetPointerEventFilterTime(int64_t filterTime)
{
    FI_HILOGD("enter");
    g_startFilterTime = filterTime;
    FI_HILOGD("leave");
}

void DragManager::MoveTo(int32_t x, int32_t y)
{
    if (dragState_ != DragState::START && dragState_ != DragState::MOTION_DRAGGING) {
        FI_HILOGE("Drag instance not running");
        return;
    }
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    FI_HILOGI("displayId:%{public}d, x:%{public}d, y:%{public}d", dragData.displayId, x, y);
    dragDrawing_.Draw(dragData.displayId, x, y);
}

int32_t DragManager::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    if (dragState_ != DragState::START && dragState_ != DragState::MOTION_DRAGGING) {
        FI_HILOGE("Drag instance not running");
        return RET_ERR;
    }
    if (previewStyle == DRAG_DATA_MGR.GetPreviewStyle()) {
        FI_HILOGD("Not need to update previewStyle");
        return RET_OK;
    }
    DRAG_DATA_MGR.SetPreviewStyle(previewStyle);
    FI_HILOGI("Update previewStyle successfully");
    return dragDrawing_.UpdatePreviewStyle(previewStyle);
}

int32_t DragManager::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle,
    const PreviewAnimation &animation)
{
    if (dragState_ != DragState::START && dragState_ != DragState::MOTION_DRAGGING) {
        FI_HILOGE("Drag instance not running");
        return RET_ERR;
    }
    if (previewStyle == DRAG_DATA_MGR.GetPreviewStyle()) {
        FI_HILOGD("Not need to update previewStyle");
        return RET_OK;
    }
    DRAG_DATA_MGR.SetPreviewStyle(previewStyle);
    FI_HILOGI("Update previewStyle successfully");
    return dragDrawing_.UpdatePreviewStyleWithAnimation(previewStyle, animation);
}

void DragManager::DragKeyEventCallback(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CHKPV(keyEvent);
    auto keyItems = keyEvent->GetKeyItems();
    auto iter = std::find_if(keyItems.begin(), keyItems.end(),
        [] (std::optional<MMI::KeyEvent::KeyItem> keyItem) {
            return ((keyItem->GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_LEFT) ||
                    (keyItem->GetKeyCode() == MMI::KeyEvent::KEYCODE_CTRL_RIGHT));
        });
    if (iter == keyItems.end()) {
        dragAction_.store(DragAction::MOVE);
        return;
    }
    if ((DRAG_DATA_MGR.GetDragStyle() == DragCursorStyle::DEFAULT) ||
        (DRAG_DATA_MGR.GetDragStyle() == DragCursorStyle::FORBIDDEN)) {
        dragAction_.store(DragAction::MOVE);
        return;
    }
    if (!iter->IsPressed()) {
        CtrlKeyStyleChangedNotify(DRAG_DATA_MGR.GetDragStyle(), DragAction::MOVE);
        HandleCtrlKeyEvent(DRAG_DATA_MGR.GetDragStyle(), DragAction::MOVE);
        dragAction_.store(DragAction::MOVE);
        return;
    }
    if (DRAG_DATA_MGR.GetDragStyle() == DragCursorStyle::COPY) {
        FI_HILOGD("Not need update drag style");
        return;
    }
    CtrlKeyStyleChangedNotify(DragCursorStyle::COPY, DragAction::COPY);
    HandleCtrlKeyEvent(DragCursorStyle::COPY, DragAction::COPY);
    dragAction_.store(DragAction::COPY);
}

void DragManager::HandleCtrlKeyEvent(DragCursorStyle style, DragAction action)
{
    FI_HILOGD("enter");
    if (action == dragAction_.load()) {
        FI_HILOGD("Not need update drag style");
        return;
    }
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&DragDrawing::UpdateDragStyle, &dragDrawing_, style));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
    FI_HILOGD("leave");
}

int32_t DragManager::OnUpdateDragStyle(DragCursorStyle style)
{
    FI_HILOGD("enter");
    DragCursorStyle updateStyle = GetRealDragStyle(style);
    stateNotify_.StyleChangedNotify(updateStyle);
    if (dragDrawing_.UpdateDragStyle(updateStyle) != RET_OK) {
        DragDFX::WriteUpdateDragStyle(updateStyle, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        return RET_ERR;
    }
    FI_HILOGD("Update dragStyle:%{public}s successfully", GetDragStyleName(updateStyle).c_str());
    return RET_OK;
}

void DragManager::UpdateDragStyleCross()
{
    FI_HILOGD("enter");
    auto dragStyle = DRAG_DATA_MGR.GetDragStyle();
    FI_HILOGI("OnUpdateDragStyle dragStyle:%{public}s", GetDragStyleName(dragStyle).c_str());
    if (OnUpdateDragStyle(DRAG_DATA_MGR.GetDragStyle()) != RET_OK) {
        FI_HILOGE("OnUpdateDragStyle failed");
    }
    FI_HILOGD("leave");
}

std::string DragManager::GetDragStyleName(DragCursorStyle style)
{
    switch (style) {
        case DragCursorStyle::DEFAULT : {
            return DRAG_STYLE_DEFAULT;
        }
        case DragCursorStyle::FORBIDDEN : {
            return DRAG_STYLE_FORBIDDEN;
        }
        case DragCursorStyle::COPY : {
            return DRAG_STYLE_COPY;
        }
        case DragCursorStyle::MOVE : {
            return DRAG_STYLE_MOVE;
        }
        default:
            break;
    }
    return DRAG_STYLE_UNKNOW;
}

DragCursorStyle DragManager::GetRealDragStyle(DragCursorStyle style)
{
    if ((dragAction_ == DragAction::COPY) && (style == DragCursorStyle::MOVE)) {
        return DragCursorStyle::COPY;
    }
    return style;
}

void DragManager::GetDragBehavior(const DragDropResult &dropResult, DragBehavior &dragBehavior)
{
    FI_HILOGD("enter");
    if (dropResult.result != DragResult::DRAG_SUCCESS) {
        dragBehavior = DragBehavior::UNKNOWN;
        return;
    }
    if (dragBehavior == DragBehavior::UNKNOWN) {
        if (DRAG_DATA_MGR.GetDragStyle() == DragCursorStyle::COPY) {
            dragBehavior = DragBehavior::COPY;
            return;
        }
        if (dragAction_.load()== DragAction::COPY) {
            dragBehavior = DragBehavior::COPY;
            return;
        }
        DragData dragData = DRAG_DATA_MGR.GetDragData();
        if (dropResult.mainWindow == dragData.mainWindow) {
            dragBehavior = DragBehavior::MOVE;
        } else {
            dragBehavior = DragBehavior::COPY;
        }
    }
    FI_HILOGD("leave");
}

void DragManager::CtrlKeyStyleChangedNotify(DragCursorStyle style, DragAction action)
{
    FI_HILOGD("enter");
    if (action == dragAction_.load()) {
        FI_HILOGD("Has notified");
        return;
    }
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&StateChangeNotify::StyleChangedNotify, &stateNotify_, style));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
    FI_HILOGD("leave");
}

int32_t DragManager::GetDragAction(DragAction &dragAction) const
{
    FI_HILOGD("enter");
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running, can not get drag action");
        return RET_ERR;
    }
    dragAction = dragAction_.load();
    FI_HILOGD("leave");
    return RET_OK;
}

int32_t DragManager::EnterTextEditorArea(bool enable)
{
    FI_HILOGD("enter");
    if (dragState_ != DragState::START) {
        FI_HILOGE("No drag instance running");
        return RET_ERR;
    }
    if (DRAG_DATA_MGR.GetTextEditorAreaFlag() == enable) {
        FI_HILOGE("Set textEditorArea:%{public}s already", (enable ? "true" : "false"));
        return RET_ERR;
    }
    if (DRAG_DATA_MGR.GetCoordinateCorrected()) {
        FI_HILOGE("GetCoordinateCorrected failed");
        return RET_ERR;
    }
    FI_HILOGD("leave");
    return dragDrawing_.EnterTextEditorArea(enable);
}

int32_t DragManager::GetExtraInfo(std::string &extraInfo) const
{
    FI_HILOGD("enter");
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    if (dragData.extraInfo.empty()) {
        FI_HILOGE("The extraInfo is empty");
        return RET_ERR;
    }
    extraInfo = dragData.extraInfo;
    FI_HILOGD("leave");
    return RET_OK;
}

int32_t DragManager::AddPrivilege(int32_t tokenId)
{
    FI_HILOGD("enter");
    if (dragState_ != DragState::START && dragState_ != DragState::MOTION_DRAGGING) {
        FI_HILOGE("Drag instance not running");
        return RET_ERR;
    }
    DragData dragData = DRAG_DATA_MGR.GetDragData();
    FI_HILOGD("Target window drag tid:%{public}d", tokenId);
    SendDragData(tokenId, dragData.udKey);
    FI_HILOGD("leave");
    return RET_OK;
}

int32_t DragManager::RotateDragWindow(Rosen::Rotation rotation)
{
    FI_HILOGD("enter");
    dragDrawing_.SetRotation(rotation);
    if (dragState_ != DragState::START && dragState_ != DragState::MOTION_DRAGGING) {
        FI_HILOGD("Drag instance not running");
        return RET_OK;
    }
    FI_HILOGD("leave");
    return dragDrawing_.RotateDragWindow(rotation);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
