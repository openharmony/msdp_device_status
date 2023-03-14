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
#include "drag_message.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManagerImpl" };
} // namespace

int32_t DragManagerImpl::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragStyle(style);
}

int32_t DragManagerImpl::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragMessage(message);
}

int32_t DragManagerImpl::StartDrag(const DragData &dragData, std::function<void(const DragNotifyMsg&)> callback,
    std::function<void()> disconnectCallback)
{
    CALL_DEBUG_ENTER;
    CHKPR(callback, RET_ERR);
    CHKPR(dragData.shadowRes.pixelMap, RET_ERR);
    if (dragData.shadowRes.pixelMap->GetWidth() > MAX_PIXEL_MAP_WIDTH ||
        dragData.shadowRes.pixelMap->GetHeight() > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Too big pixelMap, width:%{public}d, height:%{public}d",
            dragData.shadowRes.pixelMap->GetWidth(), dragData.shadowRes.pixelMap->GetHeight());
        return RET_ERR;
    }
    if (dragData.dragNum <= 0 || dragData.buffer.size() > MAX_BUFFER_SIZE ||
        dragData.displayX < 0 || dragData.displayY < 0 || dragData.displayId < 0) {
        FI_HILOGE("Invalid parameter, dragNum:%{public}d, bufferSize:%{public}zu, "
                  "displayX:%{public}d, displayY:%{public}d, displayId:%{public}d",
            dragData.dragNum, dragData.buffer.size(), dragData.displayX, dragData.displayY, dragData.displayId);
        return RET_ERR;
    }
    {
        std::lock_guard<std::mutex> guard(mtx_);
        stopCallback_ = callback;
        disconnectCallback_ = disconnectCallback;
    }
    return DeviceStatusClient::GetInstance().StartDrag(dragData);
}

int32_t DragManagerImpl::StopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().StopDrag(result, hasCustomAnimation);
}

int32_t DragManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().GetDragTargetPid();
}

int32_t DragManagerImpl::OnNotifyResult(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    DragNotifyMsg notifyMsg;
    int32_t result = 0;
    pkt >> notifyMsg.displayX >> notifyMsg.displayY >> result >> notifyMsg.targetPid;
    notifyMsg.result = static_cast<DragResult>(result);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read drag msg failed");
        return RET_ERR;
    }
    std::lock_guard<std::mutex> guard(mtx_);
    CHKPR(stopCallback_, RET_ERR);
    stopCallback_(notifyMsg);
    StreamClient &streamClient = const_cast<StreamClient &>(client);
    streamClient.Stop();
    CHKPR(disconnectCallback_, RET_ERR);
    disconnectCallback_();
    return RET_OK;
}

int32_t DragManagerImpl::AddDraglistener(DragListenerPtr listener)
{
    CALL_INFO_TRACE;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (!hasRegistered_) {
        FI_HILOGI("Start monitoring");
        hasRegistered_ = true;
        int32_t ret = DeviceStatusClient::GetInstance().AddDraglistener();
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register");
            return ret;
        }
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
    CALL_INFO_TRACE;
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
        return DeviceStatusClient::GetInstance().RemoveDraglistener();
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
