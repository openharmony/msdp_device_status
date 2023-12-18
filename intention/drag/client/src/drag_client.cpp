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

#include "drag_client.h"

#include "default_params.h"
#include "drag_params.h"
#include "devicestatus_define.h"
#include "devicestatus_func_callback.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragClient" };
} // namespace

int32_t DragClient::StartDrag(ITunnelClient &tunnel,
    const DragData &dragData, std::shared_ptr<IStartDragListener> listener)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    for (const auto& shadowInfo : dragData.shadowInfos) {
        CHKPR(shadowInfo.pixelMap, RET_ERR);
        if ((shadowInfo.x > 0) || (shadowInfo.y > 0) ||
            (shadowInfo.x < -shadowInfo.pixelMap->GetWidth()) ||
            (shadowInfo.y < -shadowInfo.pixelMap->GetHeight())) {
            FI_HILOGE("Invalid parameter, shadowInfox:%{public}d, shadowInfoy:%{public}d",
                shadowInfo.x, shadowInfo.y);
            return RET_ERR;
        }
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
        startDragListener_ = listener;
    }
    StartDragParam param { dragData };
    DefaultReply reply {};

    int32_t ret = tunnel.Start(Intention::DRAG, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("ITunnelClient::Start fail");
    }
    return ret;
}

int32_t DragClient::StopDrag(ITunnelClient &tunnel, const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    StopDragParam param(dropResult);
    DefaultReply reply;

    int32_t ret = tunnel.Stop(Intention::DRAG, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("ITunnelClient::Start fail");
    }
    return ret;
}

int32_t DragClient::AddDraglistener(ITunnelClient &tunnel, DragListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (std::any_of(dragListener_.cbegin(), dragListener_.cend(),
                    [listener](DragListenerPtr tListener) {
                        return (tListener == listener);
                    })) {
        return RET_OK;
    }
    if (!hasRegistered_) {
        DefaultParam param {};
        DefaultReply reply {};

        int32_t ret = tunnel.AddWatch(Intention::DRAG, DragRequestID::ADD_DRAG_LISTENER, param, reply);
        if (ret != RET_OK) {
            FI_HILOGE("ITunnelClient::AddWatch fail");
            return ret;
        }
        hasRegistered_ = true;
    }
    dragListener_.push_back(listener);
    return RET_OK;
}

int32_t DragClient::RemoveDraglistener(ITunnelClient &tunnel, DragListenerPtr listener)
{
    CALL_DEBUG_ENTER;
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
        DefaultParam param {};
        DefaultReply reply {};

        int32_t ret = tunnel.RemoveWatch(Intention::DRAG, DragRequestID::REMOVE_DRAG_LISTENER, param, reply);
        if (ret != RET_OK) {
            FI_HILOGE("ITunnelClient::RemoveWatch fail");
            return ret;
        }
    }
    return RET_OK;
}

int32_t DragClient::AddSubscriptListener(ITunnelClient &tunnel, SubscriptListenerPtr listener)
{
    return RET_ERR;
}

int32_t DragClient::RemoveSubscriptListener(ITunnelClient &tunnel, SubscriptListenerPtr listener)
{
    return RET_ERR;
}

int32_t DragClient::SetDragWindowVisible(ITunnelClient &tunnel, bool visible)
{
    return RET_ERR;
}

int32_t DragClient::UpdateDragStyle(ITunnelClient &tunnel, DragCursorStyle style)
{
    return RET_ERR;
}

int32_t DragClient::UpdateShadowPic(ITunnelClient &tunnel, const ShadowInfo &shadowInfo)
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
    UpdateShadowPicParam param { shadowInfo };
    DefaultReply reply {};

    int32_t ret = tunnel.SetParam(Intention::DRAG, DragRequestID::UPDATE_SHADOW_PIC, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("ITunnelClient::SetParam fail");
    }
    return ret;
}

int32_t DragClient::GetDragTargetPid(ITunnelClient &tunnel)
{
    return RET_ERR;
}

int32_t DragClient::GetUdKey(ITunnelClient &tunnel, std::string &udKey)
{
    return RET_ERR;
}

int32_t DragClient::GetShadowOffset(ITunnelClient &tunnel,
    int32_t &offsetX, int32_t &offsetY, int32_t &width, int32_t &height)
{
    return RET_ERR;
}

int32_t DragClient::GetDragData(ITunnelClient &tunnel, DragData &dragData)
{
    CALL_DEBUG_ENTER;
    DefaultParam param {};
    GetDragDataReply reply { dragData };

    int32_t ret = tunnel.GetParam(Intention::DRAG, DragRequestID::GET_DRAG_DATA, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("ITunnelClient::GetParam fail");
    }
    return ret;
}

int32_t DragClient::UpdatePreviewStyle(ITunnelClient &tunnel, const PreviewStyle &previewStyle)
{
    return RET_ERR;
}

int32_t DragClient::UpdatePreviewStyleWithAnimation(ITunnelClient &tunnel,
    const PreviewStyle &previewStyle, const PreviewAnimation &animation)
{
    return RET_ERR;
}

int32_t DragClient::GetDragSummary(ITunnelClient &tunnel, std::map<std::string, int64_t> &summarys)
{
    return RET_ERR;
}

int32_t DragClient::GetDragState(ITunnelClient &tunnel, DragState &dragState)
{
    return RET_ERR;
}

int32_t DragClient::EnterTextEditorArea(ITunnelClient &tunnel, bool enable)
{
    return RET_ERR;
}

int32_t DragClient::GetDragAction(ITunnelClient &tunnel, DragAction &dragAction)
{
    return RET_ERR;
}

int32_t DragClient::GetExtraInfo(ITunnelClient &tunnel, std::string &extraInfo)
{
    return RET_ERR;
}

int32_t DragClient::AddPrivilege(ITunnelClient &tunnel)
{
    return RET_ERR;
}

int32_t DragClient::OnNotifyResult(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}

int32_t DragClient::OnNotifyHideIcon(const StreamClient& client, NetPacket& pkt)
{
    return RET_ERR;
}

int32_t DragClient::OnStateChangedMessage(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}

int32_t DragClient::OnDragStyleChangedMessage(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
