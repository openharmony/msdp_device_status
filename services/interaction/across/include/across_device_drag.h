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

#ifndef ACROSS_DEVICE_DRAG
#define ACROSS_DEVICE_DRAG

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_sm.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "drag_data.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class AcrossDeviceDrag final {
public:
    AcrossDeviceDrag();
    ~AcrossDeviceDrag() = default;

    int32_t Init(IContext *context);

public:
    struct DragInfo {
        int32_t dragState { 0 };
        int32_t sourceType { 0 };
        int32_t pointerId { 0 };
        int32_t displayId { 0 };
        int32_t offsetX { 0 };
        int32_t offsetY { 0 };
        int32_t dragNum { 0 };
        int32_t dragStyle { 0 };
        bool hasCanceledAnimation { false };
        bool isExisted { false };
        std::string udKey;
        uint8_t buffer[MAX_BUFFER_SIZE] { 0 };
        uint32_t dataLen { 0 };
        uint8_t data[0];
    };

private:
    void RecvDragingData(const void* data, uint32_t dataLen);
    void RecvStopDragData(const void* data, uint32_t dataLen);
    void DragStateChanged(DragState state);
    void SendDragingData();
    void SendStopDragData();
    void ProcessDragingState();
    void ProcessStopDragState();
    std::optional<DragData> ConvertDragingData(const DragInfo* dragInfo);
    void PrintDragingData(const DragData &dragData) const;
    int32_t ConvertDragingInfo(const DragData& dragData, const std::vector<uint8_t>& pixBuffer,
        DragInfo* dragInfo);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    void ProcessFreeToIn(CoordinationState oldState, CoordinationState newState);
    void ProcessFreeToOut(CoordinationState oldState, CoordinationState newState);
    void ProcessInToFree(CoordinationState oldState, CoordinationState newState);
    void ProcessOutToFree(CoordinationState oldState, CoordinationState newState);
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    IContext *context_ { nullptr };
    DragState dragState_ { DragState::ERROR };
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CooStateChangeType cooStateChangeType_ { CooStateChangeType::STATE_NONE };
#endif // OHOS_BUILD_ENABLE_COORDINATION
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // ACROSS_DEVICE_DRAG