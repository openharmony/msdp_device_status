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

#include "drag_data.h"
// #include "intention_client.h"
// #include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragManagerImpl" };
} // namespace

int32_t DragManagerImpl::UpdateDragStyle(DragCursorStyle style)
{
    if ((style < DragCursorStyle::DEFAULT) || (style > DragCursorStyle::MOVE)) {
        return RET_ERR;
    }

    DragStyleParam param { static_cast<int32_t>(style) };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().SetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::DRAG_STYLE, param, reply);
}

int32_t DragManagerImpl::StartDrag(const DragData &dragData, std::function<void(const DragNotifyMsg&)> callback)
{
    CHKPR(callback, RET_ERR);
    CHKPR(dragData.shadowInfo.pixelMap, RET_ERR);
    if ((dragData.shadowInfo.x > 0) || (dragData.shadowInfo.y > 0) ||
        (dragData.shadowInfo.x < -dragData.shadowInfo.pixelMap->GetWidth()) ||
        (dragData.shadowInfo.y < -dragData.shadowInfo.pixelMap->GetHeight())) {
        FI_HILOGE("Invalid parameter, shadowInfox:%{public}d, shadowInfoy:%{public}d",
            dragData.shadowInfo.x, dragData.shadowInfo.y);
        return RET_ERR;
    }
    if ((dragData.dragNum <= 0) || (dragData.buffer.size() > MAX_BUFFER_SIZE) ||
        (dragData.displayX < 0) || (dragData.displayY < 0)) {
        FI_HILOGE("Invalid parameter, dragNum:%{public}d, bufferSize:%{public}zu, "
            "displayX:%{public}d, displayY:%{public}d",
            dragData.dragNum, dragData.buffer.size(), dragData.displayX, dragData.displayY);
        return RET_ERR;
    }
    {
        std::lock_guard<std::mutex> guard(mtx_);
        stopCallback_ = callback;
    }

    StartDragParam param { dragData };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().Start(static_cast<uint32_t>(Intention::DRAG), param, reply);
}

int32_t DragManagerImpl::StopDrag(DragResult result, bool hasCustomAnimation)
{
    StopDragParam param { static_cast<int32_t>(result), hasCustomAnimation };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().Stop(static_cast<uint32_t>(Intention::DRAG), param, reply);
}

int32_t DragManagerImpl::GetDragTargetPid()
{
    DragTargetPidParam param;
    DefaultDragReply reply;
    return IntentionClient::GetInstance().GetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::PID, param, reply);
}

int32_t DragManagerImpl::GetUdKey(std::string &udKey)
{
    GetUdKeyParam param { udKey };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().GetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::UDKEY, param, reply);
}

int32_t DragManagerImpl::AddDraglistener(DragListenerPtr listener)
{
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (!hasRegistered_) {
        AddDragListenerParam param;
        DefaultDragReply reply;
        int32_t ret = IntentionClient::GetInstance().AddWatch(static_cast<uint32_t>(Intention::DRAG), DragParam::LISTENER, param, reply);
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register");
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
        FI_HILOGW("The listener already exists");
    }
    return RET_OK;
}

int32_t DragManagerImpl::RemoveDraglistener(DragListenerPtr listener)
{
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        dragListener_.clear();
    } else {
        dragListener_.erase(std::remove_if(dragListener_.begin(), dragListener_.end(),
            [listener] (auto iter) {
                return iter == listener;
            })
        );
    }

    if (hasRegistered_ && dragListener_.empty()) {
        hasRegistered_ = false;
        RemoveDragListenerParam param;
        DefaultDragReply reply;
        return IntentionClient::GetInstance().RemoveWatch(static_cast<uint32_t>(Intention::DRAG), DragParam::LISTENER, param, reply);
    }
    return RET_OK;
}

int32_t DragManagerImpl::SetDragWindowVisible(bool visible)
{
    SetDragWindowVisibleParam param { visible };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().SetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::WINDOW_VISIBLE, param, reply);
}

int32_t DragManagerImpl::GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height)
{
    GetShadowOffsetParam param { offsetX, offsetY, width, height };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().GetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::SHADOW_OFFSET, param, reply);
}

int32_t DragManagerImpl::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CHKPR(shadowInfo.pixelMap, RET_ERR);
    if ((shadowInfo.x > 0) || (shadowInfo.y > 0) ||
        (shadowInfo.x < -shadowInfo.pixelMap->GetWidth()) ||
        (shadowInfo.y < -shadowInfo.pixelMap->GetHeight())) {
        FI_HILOGE("Invalid parameter, shadowInfox:%{public}d, shadowInfoy:%{public}d",
            shadowInfo.x, shadowInfo.y);
        return RET_ERR;
    }

    UpdateShadowPicParam param { shadowInfo };
    DefaultDragReply reply;
    return IntentionClient::GetInstance().SetParam(static_cast<uint32_t>(Intention::DRAG), DragParam::SHADOW_PIC, param, reply);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
