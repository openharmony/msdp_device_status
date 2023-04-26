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

#include "across_device_drag.h"

#include "pointer_event.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_softbus_adapter.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "devicestatus_define.h"
#include "drag_data_adapter.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "AcrossDeviceDrag" };
} // namespace

AcrossDeviceDrag::AcrossDeviceDrag()
{
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CooSoftbusAdapter->RegisterRecvFunc(CoordinationSoftbusAdapter::DRAGING_DATA,
        std::bind(&AcrossDeviceDrag::RecvDragingData, this, std::placeholders::_1, std::placeholders::_2));
    CooSoftbusAdapter->RegisterRecvFunc(CoordinationSoftbusAdapter::STOPDRAG_DATA,
        std::bind(&AcrossDeviceDrag::RecvStopDragData, this, std::placeholders::_1, std::placeholders::_2));
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t AcrossDeviceDrag::Init(IContext *context)
{
    CALL_DEBUG_ENTER;
    CHKPR(context, RET_ERR);
    context_ = context;
    context_->GetDragManager().RegisterStateChange(std::bind(&AcrossDeviceDrag::DragStateChanged,
        this, std::placeholders::_1));
    return RET_OK;
}

void AcrossDeviceDrag::RecvDragingData(const void* data, uint32_t dataLen)
{
    CALL_DEBUG_ENTER;
    CHKPV(data);
    CHKPV(context_);
    if (dataLen == 0) {
        FI_HILOGE("Recv data len is 0");
        return;
    }
    const DragInfo* dragInfo = static_cast<const DragInfo*>(data);
    if (dragInfo->dragState != static_cast<int32_t>(DragState::START)) {
        FI_HILOGE("Drag state is not draging");
        return;
    }
    std::optional<DragData> dragData = ConvertDragingData(dragInfo);
    if (!dragData) {
        FI_HILOGI("ConvertDragingData failed");
        return;
    }
    if (context_->GetDragManager().StartDrag(dragData.value(), nullptr) != RET_OK) {
        FI_HILOGI("StartDrag failed");
    }
}
std::optional<DragData> AcrossDeviceDrag::ConvertDragingData(const DragInfo* dragInfo)
{
    CALL_DEBUG_ENTER;
    if (dragInfo == nullptr) {
        return std::nullopt;
    }
    DragData dragData;
    std::vector<uint8_t> buffer(dragInfo->data, dragInfo->data + dragInfo->dataLen);
    Media::PixelMap *pixelMap = Media::PixelMap::DecodeTlv(buffer);
    if (pixelMap == nullptr) {
        return std::nullopt;
    }
    dragData.shadowInfo.pixelMap = std::shared_ptr<Media::PixelMap>(pixelMap);
    dragData.shadowInfo.x = dragInfo->offsetX;
    dragData.shadowInfo.y = dragInfo->offsetY;
    dragData.buffer.insert(dragData.buffer.begin(), dragInfo->buffer, dragInfo->buffer + MAX_BUFFER_SIZE);
    dragData.sourceType = dragInfo->sourceType;
    dragData.pointerId = dragInfo->pointerId;
    dragData.displayId = dragInfo->displayId;
    dragData.dragNum = dragInfo->dragNum;
    dragData.udKey = dragInfo->udKey;
    dragData.hasCanceledAnimation = dragInfo->hasCanceledAnimation;
    PrintDragingData(dragData);
    return dragData;
}

void AcrossDeviceDrag::RecvStopDragData(const void* data, uint32_t dataLen)
{
    CALL_DEBUG_ENTER;
    CHKPV(data);
    CHKPV(context_);
    if (dataLen == 0) {
        FI_HILOGE("Recv data len is 0");
        return;
    }
    auto result = static_cast<const DragResult*>(data);
    context_->GetDragManager().StopDrag(*result, false);
}

void AcrossDeviceDrag::SendDragingData()
{
    CALL_DEBUG_ENTER;
    CHKPV(context_);
    if (context_->GetDragManager().GetDragState() != DragState::START) {
        FI_HILOGE("Drag state is not draging");
        return;
    }
    auto dragData = DataAdapter.GetDragData();
    if (dragData.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGE("Source type is not mouse");
        return;
    }
	
    PrintDragingData(dragData);
    auto pixelMap = dragData.shadowInfo.pixelMap;
    CHKPV(pixelMap);
    std::vector<uint8_t> pixelBuffer;
    if (!pixelMap->EncodeTlv(pixelBuffer)) {
        FI_HILOGE("Pixel map tlv encode fail");
        return;
    }
    auto size = sizeof(DragInfo) + pixelBuffer.size();
    FI_HILOGI("Malloc size:%{public}zu, drag info size:%{public}zu, pixel buffer size:%{public}zu",
        size, sizeof(DragInfo), pixelBuffer.size());
    DragInfo* dragInfo = (DragInfo*)malloc(size);
    CHKPV(dragInfo);
    if (ConvertDragingInfo(dragData, pixelBuffer, dragInfo) != RET_OK) {
        FI_HILOGE("Encode drag info failed");
        free(dragInfo);
        return;
    }
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::string remotedeviceId = CooSM->GetRemoteId();
    if (remotedeviceId.empty()) {
        free(dragInfo);
        FI_HILOGE("Remote device id is empty");
        return;
    }
    if (CooSoftbusAdapter->OpenInputSoftbus(remotedeviceId) != RET_OK) {
        FI_HILOGE("OpenInputSoftbus failed");
        free(dragInfo);
        return;
    }
    if (CooSoftbusAdapter->SendData(remotedeviceId, CoordinationSoftbusAdapter::DRAGING_DATA,
        dragInfo, size) != RET_OK) {
        FI_HILOGE("Send draging data failed");
        free(dragInfo);
        return;
    }
#else
    FI_HILOGW("Coordination does not support");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    free(dragInfo);
}

int32_t AcrossDeviceDrag::ConvertDragingInfo(const DragData& dragData,
    const std::vector<uint8_t>& pixelBuffer, DragInfo* dragInfo)
{
    CALL_DEBUG_ENTER;
    CHKPR(dragInfo, RET_ERR);
    CHKPR(context_, RET_ERR);
    dragInfo->dragState = static_cast<int32_t>(context_->GetDragManager().GetDragState());
    dragInfo->sourceType = static_cast<int32_t>(dragData.sourceType);
    dragInfo->pointerId = dragData.pointerId;
    dragInfo->displayId = dragData.displayId;
    dragInfo->dragNum = dragData.dragNum;
    dragInfo->dragStyle = static_cast<int32_t>(DataAdapter.GetDragStyle());
    dragInfo->offsetX = dragData.shadowInfo.x;
    dragInfo->offsetY = dragData.shadowInfo.y;
    dragInfo->hasCanceledAnimation = dragData.hasCanceledAnimation;
    dragInfo->udKey = dragData.udKey;
    dragInfo->isExisted = true;
    if (dragData.buffer.empty()) {
        FI_HILOGE("Drag data buffer is empty");
        return RET_ERR;
    }
    errno_t ret = memcpy_s(dragInfo->buffer, MAX_BUFFER_SIZE, &dragData.buffer[0], dragData.buffer.size());
    if (ret != EOK) {
        FI_HILOGE("Memcpy buffer failed");
        return RET_ERR;
    }
    if (pixelBuffer.empty()) {
        FI_HILOGE("Pix buffer is empty");
        return RET_ERR;
    }
    dragInfo->dataLen = pixelBuffer.size();
    ret = memcpy_s(dragInfo->data, dragInfo->dataLen, &pixelBuffer[0], pixelBuffer.size());
    if (ret != EOK) {
        FI_HILOGE("Memcpy pix buffer failed");
        return RET_ERR;
    }
    return RET_OK;
}

void AcrossDeviceDrag::SendStopDragData()
{
    CALL_DEBUG_ENTER;
}

void AcrossDeviceDrag::PrintDragingData(const DragData &dragData) const
{
    CHKPV(dragData.shadowInfo.pixelMap);
    FI_HILOGD("PixelFormat:%{public}d, PixelAlphaType:%{public}d, PixelAllocatorType:%{public}d, PixelWidth:%{public}d,"
        "PixelHeight:%{public}d, udKey:%{public}s, sourceType:%{public}d, pointerId:%{public}d, shadowX:%{public}d,"
        "shadowY:%{public}d, displayId:%{public}d, dragNum:%{public}d, hasCanceledAnimation:%{public}d",
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetPixelFormat()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAlphaType()),
        static_cast<int32_t>(dragData.shadowInfo.pixelMap->GetAllocatorType()),
        dragData.shadowInfo.pixelMap->GetWidth(), dragData.shadowInfo.pixelMap->GetHeight(),
        dragData.udKey.c_str(), dragData.sourceType, dragData.pointerId, dragData.shadowInfo.x, dragData.shadowInfo.y,
        dragData.displayId, dragData.dragNum, dragData.hasCanceledAnimation);
}

void AcrossDeviceDrag::DragStateChanged(DragState state)
{
    CALL_DEBUG_ENTER;
    dragState_ = state;
    switch (state) {
        case DragState::START: {
            ProcessDragingState();
            break;
        }
        case DragState::STOP: {
            ProcessStopDragState();
            break;
        }
        default: {
            FI_HILOGW("Unknown state:%{public}d", state);
            break;
        }
    }
}

void AcrossDeviceDrag::ProcessDragingState()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CooSM->RegisterStateChange(CooStateChangeType::STATE_FREE_TO_IN,
        std::bind(&AcrossDeviceDrag::ProcessFreeToIn, this, std::placeholders::_1, std::placeholders::_2));
    CooSM->RegisterStateChange(CooStateChangeType::STATE_FREE_TO_OUT,
        std::bind(&AcrossDeviceDrag::ProcessFreeToOut, this, std::placeholders::_1, std::placeholders::_2));
    CooSM->RegisterStateChange(CooStateChangeType::STATE_IN_TO_FREE,
        std::bind(&AcrossDeviceDrag::ProcessInToFree, this, std::placeholders::_1, std::placeholders::_2));
    CooSM->RegisterStateChange(CooStateChangeType::STATE_OUT_TO_FREE,
        std::bind(&AcrossDeviceDrag::ProcessOutToFree, this, std::placeholders::_1, std::placeholders::_2));
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION
void AcrossDeviceDrag::ProcessFreeToIn(CoordinationState oldState, CoordinationState newState)
{
    CALL_INFO_TRACE;
    cooStateChangeType_ = CooStateChangeType::STATE_FREE_TO_IN;
}

void AcrossDeviceDrag::ProcessFreeToOut(CoordinationState oldState, CoordinationState newState)
{
    CALL_INFO_TRACE;
    cooStateChangeType_ = CooStateChangeType::STATE_FREE_TO_OUT;
}

void AcrossDeviceDrag::ProcessInToFree(CoordinationState oldState, CoordinationState newState)
{
    CALL_INFO_TRACE;
    cooStateChangeType_ = CooStateChangeType::STATE_IN_TO_FREE;
}

void AcrossDeviceDrag::ProcessOutToFree(CoordinationState oldState, CoordinationState newState)
{
    CALL_INFO_TRACE;
    cooStateChangeType_ = CooStateChangeType::STATE_OUT_TO_FREE;
}
#endif // OHOS_BUILD_ENABLE_COORDINATION

void AcrossDeviceDrag::ProcessStopDragState()
{
    CALL_INFO_TRACE;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS