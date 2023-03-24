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

#include <memory>
#include <optional>
#include <utility>

#include "stopdrag_fuzzer.h"

#include "input_manager.h"
#include "pointer_event.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "StopDragFuzzTest" };
constexpr int32_t DRAG_NUM { 1 };
constexpr bool HAS_CANCELED_ANIMATION { false };
constexpr bool HAS_CUSTOM_ANIMATION { false };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DRAG_SRC_X { 0 };
constexpr int32_t DRAG_SRC_Y { 0 };
constexpr int32_t DRAG_UP_X { 100 };
constexpr int32_t DRAG_UP_Y { 100 };
constexpr int32_t POINTER_ID { 0 };
constexpr int32_t DEFAULT_DEVICE_ID { 0 };
#define INPUT_MANAGER  MMI::InputManager::GetInstance()
} // namespace

std::shared_ptr<Media::PixelMap> CreatePixelMap(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (width <= 0 || width > MAX_PIXEL_MAP_WIDTH ||
       height <= 0 || height > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size,width:%{public}d,height:%{public}d", width, height);
        return nullptr;
    }
    OHOS::Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(opts);
    return pixelMap;
}

std::optional<DragData> CreateDragData(std::pair<int32_t, int32_t> pixelMapSize,
    int32_t sourceType, int32_t pointerId, int32_t displayId, std::pair<int32_t, int32_t> location)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(pixelMapSize.first, pixelMapSize.second);
    if (pixelMap == nullptr) {
        FI_HILOGE("CreatePixelMap failed");
        return std::nullopt;
    }
    DragData dragData;
    dragData.shadowInfo.pixelMap = pixelMap;
    dragData.shadowInfo.x = 0;
    dragData.shadowInfo.y = 0;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = sourceType;
    dragData.pointerId = pointerId;
    dragData.dragNum = DRAG_NUM;
    dragData.displayX = location.first;
    dragData.displayY = location.second;
    dragData.displayId = displayId;
    dragData.hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    return dragData;
}

MMI::PointerEvent::PointerItem CreatePointerItem(int32_t pointerId,
    int32_t deviceId, std::pair<int, int> displayLocation, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    item.SetDisplayX(displayLocation.first);
    item.SetDisplayY(displayLocation.second);
    item.SetPressed(isPressed);
    return item;
}

std::shared_ptr<OHOS::MMI::PointerEvent> SetupPointerEvent(std::pair<int, int> displayLocation,
    int32_t action, int32_t sourceType, int32_t pointerId, bool isPressed)
{
    CALL_DEBUG_ENTER;
    auto pointerEvent = OHOS::MMI::PointerEvent::Create();
    CHKPP(pointerEvent);
    pointerEvent->SetPointerAction(action);
    pointerEvent->SetSourceType(sourceType);
    pointerEvent->SetPointerId(pointerId);
    auto curPointerItem = CreatePointerItem(pointerId, DEFAULT_DEVICE_ID, displayLocation, isPressed);
    pointerEvent->AddPointerItem(curPointerItem);
    return pointerEvent;
}

void SimulateUpEvent(std::pair<int, int> location, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<OHOS::MMI::PointerEvent> pointerEvent =
        SetupPointerEvent(location, OHOS::MMI::PointerEvent::POINTER_ACTION_UP, sourceType, pointerId, false);
    FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
    INPUT_MANAGER->SimulateInputEvent(pointerEvent);
}

void StopDragFuzzTest(const uint8_t* data, size_t  size)
{
    if (data == nullptr) {
        return;
    }
    auto func = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("StartDragFuzzTest:displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    dragData->hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    InteractionManager::GetInstance()->StartDrag(dragData.value(), func);
    int32_t result = *(reinterpret_cast<const int32_t*>(data));
    SimulateUpEvent({DRAG_UP_X, DRAG_UP_Y}, OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID );
    InteractionManager::GetInstance()->StopDrag(static_cast<DragResult>(result), HAS_CUSTOM_ANIMATION);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }
    if (size < sizeof(int32_t)) {
        return 0;
    }
    OHOS::Msdp::DeviceStatus::StopDragFuzzTest(data, size);
    return 0;
}

