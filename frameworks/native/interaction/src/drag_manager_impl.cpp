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

#include "drag_manager_impl.h"

#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragManagerImpl" };
} // namespace

int32_t DragManagerImpl::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    if ((style < DragCursorStyle::DEFAULT) || (style > DragCursorStyle::MOVE)) {
        FI_HILOGE("Invalid style:%{public}d", static_cast<int32_t>(style));
        return RET_ERR;
    }
    FI_HILOGD("Ready to modify the style(%{public}d)", static_cast<int32_t>(style));
    return DeviceStatusClient::GetInstance().UpdateDragStyle(style);
}

int32_t DragManagerImpl::StartDrag(const DragData &dragData, std::function<void(const DragNotifyMsg&)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPR(callback, RET_ERR);
    CHKPR(dragData.shadowInfo.pixelMap, RET_ERR);
    if ((dragData.shadowInfo.x > 0) || (dragData.shadowInfo.y > 0) ||
        (dragData.shadowInfo.x < -dragData.shadowInfo.pixelMap->GetWidth()) ||
        (dragData.shadowInfo.y < -dragData.shadowInfo.pixelMap->GetHeight())) {
        FI_HILOGE("Start drag, invalid parameter, shadowInfox:%{public}d, shadowInfoy:%{public}d",
            dragData.shadowInfo.x, dragData.shadowInfo.y);
        return RET_ERR;
    }
    if ((dragData.dragNum <= 0) || (dragData.buffer.size() > MAX_BUFFER_SIZE) ||
        (dragData.displayX < 0) || (dragData.displayY < 0)) {
        FI_HILOGE("Start drag, invalid argument, dragNum:%{public}d, bufferSize:%{public}zu, "
            "displayX:%{public}d, displayY:%{public}d",
            dragData.dragNum, dragData.buffer.size(), dragData.displayX, dragData.displayY);
        return RET_ERR;
    }
    {
        std::lock_guard<std::mutex> guard(mtx_);
        stopCallback_ = callback;
    }
    return DeviceStatusClient::GetInstance().StartDrag(dragData);
}

int32_t DragManagerImpl::StopDrag(const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().StopDrag(dropResult);
}

int32_t DragManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragTargetPid();
}

int32_t DragManagerImpl::GetUdKey(std::string &udKey)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetUdKey(udKey);
}

int32_t DragManagerImpl::OnNotifyResult(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    DragNotifyMsg notifyMsg;
    int32_t result = 0;
    pkt >> notifyMsg.displayX >> notifyMsg.displayY >> result >> notifyMsg.targetPid;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read drag msg failed");
        return RET_ERR;
    }
    if ((result < static_cast<int32_t>(DragResult::DRAG_SUCCESS)) ||
        (result > static_cast<int32_t>(DragResult::DRAG_EXCEPTION))) {
        FI_HILOGE("Invalid result:%{public}d", result);
        return RET_ERR;
    }
    notifyMsg.result = static_cast<DragResult>(result);
    std::lock_guard<std::mutex> guard(mtx_);
    CHKPR(stopCallback_, RET_ERR);
    stopCallback_(notifyMsg);
    return RET_OK;
}

int32_t DragManagerImpl::OnStateChangedMessage(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t state = 0;
    pkt >> state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read drag msg failed");
        return RET_ERR;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &listener : dragListener_) {
        listener->OnDragMessage(static_cast<DragState>(state));
    }
    return RET_OK;
}

int32_t DragManagerImpl::OnDragStyleChangedMessage(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t style = 0;
    pkt >> style;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read drag msg failed");
        return RET_ERR;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &listener : subscriptListener_) {
        listener->OnMessage(static_cast<DragCursorStyle>(style));
    }
    return RET_OK;
}

int32_t DragManagerImpl::AddDraglistener(DragListenerPtr listener)
{
    CALL_INFO_TRACE;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (!hasRegistered_) {
        FI_HILOGI("Start monitoring");
        int32_t ret = DeviceStatusClient::GetInstance().AddDraglistener();
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register draglistener");
            return ret;
        }
        hasRegistered_ = true;
    }
    if (std::all_of(dragListener_.cbegin(), dragListener_.cend(),
                    [listener](DragListenerPtr tListener) {
                        return (tListener != listener);
                    })) {
        dragListener_.push_back(listener);
    } else {
        FI_HILOGW("The draglistener already exists");
    }
    return RET_OK;
}

int32_t DragManagerImpl::RemoveDraglistener(DragListenerPtr listener)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        dragListener_.clear();
    } else {
        dragListener_.erase(std::remove_if(dragListener_.begin(), dragListener_.end(),
            [listener] (auto lIter) {
                return lIter == listener;
            })
        );
    }

    if (hasRegistered_ && dragListener_.empty()) {
        hasRegistered_ = false;
        return DeviceStatusClient::GetInstance().RemoveDraglistener();
    }
    return RET_OK;
}

int32_t DragManagerImpl::AddSubscriptListener(SubscriptListenerPtr listener)
{
    CALL_INFO_TRACE;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (!hasSubscriptRegistered_) {
        FI_HILOGI("Start monitoring");
        int32_t ret = DeviceStatusClient::GetInstance().AddSubscriptListener();
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register");
            return ret;
        }
        hasSubscriptRegistered_ = true;
    }
    if (std::all_of(subscriptListener_.cbegin(), subscriptListener_.cend(),
                    [listener](SubscriptListenerPtr tListener) {
                        return (tListener != listener);
                    })) {
        subscriptListener_.push_back(listener);
    } else {
        FI_HILOGW("The listener already exists");
    }
    return RET_OK;
}

int32_t DragManagerImpl::RemoveSubscriptListener(SubscriptListenerPtr listener)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        subscriptListener_.clear();
    } else {
        subscriptListener_.erase(std::remove_if(subscriptListener_.begin(), subscriptListener_.end(),
            [listener] (auto iter) {
                return iter == listener;
            })
        );
    }

    if (hasSubscriptRegistered_ && subscriptListener_.empty()) {
        hasSubscriptRegistered_ = false;
        return DeviceStatusClient::GetInstance().RemoveSubscriptListener();
    }
    return RET_OK;
}

int32_t DragManagerImpl::SetDragWindowVisible(bool visible)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().SetDragWindowVisible(visible);
}

int32_t DragManagerImpl::GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetShadowOffset(offsetX, offsetY, width, height);
}

int32_t DragManagerImpl::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    CHKPR(shadowInfo.pixelMap, RET_ERR);
    if ((shadowInfo.x > 0) || (shadowInfo.y > 0) ||
        (shadowInfo.x < -shadowInfo.pixelMap->GetWidth()) ||
        (shadowInfo.y < -shadowInfo.pixelMap->GetHeight())) {
        FI_HILOGE("Invalid parameter, shadowInfox:%{public}d, shadowInfoy:%{public}d",
            shadowInfo.x, shadowInfo.y);
        return RET_ERR;
    }
    return DeviceStatusClient::GetInstance().UpdateShadowPic(shadowInfo);
}

int32_t DragManagerImpl::GetDragData(DragData &dragData)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragData(dragData);
}

int32_t DragManagerImpl::GetDragState(DragState &dragState)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragState(dragState);
}

int32_t DragManagerImpl::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    return DeviceStatusClient::GetInstance().UpdatePreviewStyle(previewStyle);
}

int32_t DragManagerImpl::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle, const PreviewAnimation &animation)
{
    return DeviceStatusClient::GetInstance().UpdatePreviewStyleWithAnimation(previewStyle, animation);
}

int32_t DragManagerImpl::GetDragSummary(std::map<std::string, int64_t> &summarys)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragSummary(summarys);
}

int32_t DragManagerImpl::GetDragAction(DragAction& dragAction)
{
    return DeviceStatusClient::GetInstance().GetDragAction(dragAction);
}

int32_t DragManagerImpl::EnterTextEditorArea(bool enable)
{
    return DeviceStatusClient::GetInstance().EnterTextEditorArea(enable);
}

int32_t DragManagerImpl::GetExtraInfo(std::string &extraInfo)
{
    return DeviceStatusClient::GetInstance().GetExtraInfo(extraInfo);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
