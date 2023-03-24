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

#include "startdrag_fuzzer.h"

#include <memory>
#include <utility>

#include "input_manager.h"
#include "pointer_event.h"
#include "securec.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "StartDragFuzzTest" };
constexpr int32_t DRAG_UP_X { 100 };
constexpr int32_t DRAG_UP_Y { 100 };
constexpr int32_t POINTER_ID { 0 };
constexpr int32_t DEFAULT_DEVICE_ID { 0 };
#define INPUT_MANAGER  MMI::InputManager::GetInstance()
} // namespace
template<class T>
size_t GetObject(const uint8_t *data, size_t size, T &object)
{
    size_t objectSize = sizeof(object);
    if (objectSize > size) {
        return 0;
    }
    errno_t ret = memcpy_s(&object, objectSize, data, objectSize);
    if (ret != RET_OK) {
        return 0;
    }
    return objectSize;
}

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

DragData CreateDragData(const uint8_t* data, size_t size)
{
    size_t startPos = 0;
    ShadowInfo shadowInfo;
    startPos += GetObject<int32_t>(data + startPos, size - startPos, shadowInfo.x);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, shadowInfo.y);
    shadowInfo.pixelMap = CreatePixelMap(MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT);
    
    DragData dragData;
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.dragNum);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayX);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayY);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayId);
    dragData.hasCanceledAnimation = true;
    dragData.shadowInfo = shadowInfo;
    dragData.pointerId = POINTER_ID;
    dragData.sourceType = OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE;
    return dragData;
}

OHOS::MMI::PointerEvent::PointerItem CreatePointerItem(int32_t pointerId,
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

void StartDragFuzzTest(const uint8_t* data, size_t  size)
{
    if (data == nullptr) {
        return;
    }
    auto func = [](const DragNotifyMsg& notifyMessage) {
        FI_HILOGD("StartDragFuzzTest:displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result, notifyMessage.targetPid);
    };
    DragData dragData = CreateDragData(data, size);
    InteractionManager::GetInstance()->StartDrag(dragData, func);
    SimulateUpEvent({DRAG_UP_X, DRAG_UP_Y}, OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID );
    InteractionManager::GetInstance()->StopDrag(DragResult::DRAG_SUCCESS, true);
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
    OHOS::Msdp::DeviceStatus::StartDragFuzzTest(data, size);
    return 0;
}

