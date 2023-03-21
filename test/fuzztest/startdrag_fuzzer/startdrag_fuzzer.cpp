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
    int32_t width = 0;
    int32_t height = 0;
    startPos += GetObject<int32_t>(data + startPos, size - startPos, width);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, height);
    shadowInfo.pixelMap = CreatePixelMap(width, height);
    
    DragData dragData;
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.sourceType);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.dragNum);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.pointerId);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayX);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayY);
    startPos += GetObject<int32_t>(data + startPos, size - startPos, dragData.displayId);
    dragData.hasCanceledAnimation = true;
    dragData.shadowInfo = shadowInfo;
    return dragData;
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

