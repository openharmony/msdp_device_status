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
    return RET_OK;
    stateNotify_.AddNotifyMsg(info);
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

void DragManager::MarshallPixelmap(const DragData &dragData, NetPacket& pkt)
{
    pkt << dragData.pixelMap->GetPixelFormat() << dragData.pixelMap->GetAlphaType() << dragData.pixelMap->GetWidth()
        << dragData.pixelMap->GetHeight() << dragData.pixelMap->GetAllocatorType();
    auto size = dragData.pixelMap->GetByteCount();
    pkt << size;
    pkt.Write(reinterpret_cast<const char *>(dragData.pixelMap->GetPixels()), static_cast<size_t>(size));
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
    dragState_ = DragState::DRAGGING;
    CHKPR(thumbnailDrawSession_, RET_ERR);
    NetPacket startPkt(MessageId::START_THUMBNAIL_DRAW);
    MarshallPixelmap(dragData, startPkt);
    if (!thumbnailDrawSession_->SendMsg(startPkt)) {
        FI_HILOGE("Sending failed");
        return RET_ERR;
    }

    NetPacket noticePkt(MessageId::NOTICE_THUMBNAIL_DRAW);
    noticePkt << ThumbnailDrawState::START;
    if (!thumbnailDrawSession_->SendMsg(noticePkt)) {
        FI_HILOGE("Sending failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragManager::StopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    if (dragState_ == DragState::FREE) {
        FI_HILOGE("No drag instance running, can not stop drag");
        return RET_ERR;
    }
    dragState_ = DragState::FREE;

    CHKPR(thumbnailDrawSession_, RET_ERR);
    NetPacket pkt(MessageId::STOP_THUMBNAIL_DRAW);;
    if (!thumbnailDrawSession_->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DragManager::GetDragTargetPid() const
{
    return dragTargetPid_;
}

int32_t DragManager::OnRegisterThumbnailDraw(SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    CHKPR(sess, RET_ERR);
    thumbnailDrawSession_ = sess;
    // FI_HILOGE(" 11111111111 %{public}p", thumbnailDrawSession_);
    return RET_OK;
}

int32_t DragManager::OnUnregisterThumbnailDraw(SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    CHKPR(sess, RET_ERR);
    if (thumbnailDrawSession_ == sess) {
        thumbnailDrawSession_ = nullptr;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
