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
#ifndef DRAG_MANAGER_IMPL_H
#define DRAG_MANAGER_IMPL_H

#include <functional>
#include <mutex>
#include <string>

#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
struct ThumbnailDrawCallback {
    std::function<void(int32_t, int32_t)> startDrag;
    std::function<void(int32_t)> notice;
    std::function<void(void)> endDrag;
};

class DragManagerImpl  {
public:
    DragManagerImpl() = default;
    ~DragManagerImpl() = default;
    int32_t UpdateDragStyle(int32_t style);
    int32_t UpdateDragMessage(const std::u16string &message);
    int32_t GetDragTargetPid();
    int32_t StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback);
    int32_t StopDrag(int32_t result);
    int32_t RegisterThumbnailDraw(std::function<void(int32_t, int32_t)> startDrag,
        std::function<void(int32_t)> notice, std::function<void(void)> endDrag);
    int32_t UnregisterThumbnailDraw();
private:
    std::mutex mtx_;
    std::function<void(int32_t&)> stopCallback_;
    bool hasRegisterThumbnailDraw_ { false };
    ThumbnailDrawCallback thumbnailDrawCallback_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_IMPL_H
