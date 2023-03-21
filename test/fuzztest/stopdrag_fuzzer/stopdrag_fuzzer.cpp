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

#include "stopdrag_fuzzer.h"

#include "fi_log.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

constexpr int32_t DRAG_NUM { 1 };
constexpr bool HAS_CANCELED_ANIMATION { false };
constexpr bool HAS_CUSTOM_ANIMATION { false };
constexpr int32_t TOUCH_POINTER_ID { 1 };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DRAG_SRC_X { 0 };
constexpr int32_t DRAG_SRC_Y { 0 };

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


std::optional<DragData> InteractionManagerTest::CreateDragData(std::pair<int32_t, int32_t> pixelMapSize,
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

void StopDragFuzzTest(const uint8_t* data, size_t  size)
{
    if (data == nullptr) {
        return;
    }
    std::optional<DragData> dragData = CreateDragData({ MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, DISPLAY_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    dragData->hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    int32_t result = *(reinterpret_cast<const int32_t*>(data));
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

