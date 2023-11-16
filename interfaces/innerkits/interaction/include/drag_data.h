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

#ifndef DRAG_DATA_H
#define DRAG_DATA_H

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "pixel_map.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
constexpr size_t MAX_BUFFER_SIZE { 512 };
constexpr size_t MAX_UDKEY_SIZE { 100 };
constexpr size_t MAX_SUMMARY_SIZE { 200 };
struct ShadowInfo {
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap { nullptr };
    int32_t x { -1 };
    int32_t y { -1 };
};

struct DragData {
    ShadowInfo shadowInfo;
    std::vector<uint8_t> buffer;
    std::string udKey;
    std::string extraInfo;
    std::string filterInfo;
    int32_t sourceType { -1 };
    int32_t dragNum { -1 };
    int32_t pointerId { -1 };
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t displayId { -1 };
    bool hasCanceledAnimation { false };
    std::map<std::string, int64_t> summarys;
};

enum class DragState {
    ERROR = 0,
    START = 1,
    STOP = 2,
    CANCEL = 3,
    MOTION_DRAGGING = 4
};

enum class DragResult {
    DRAG_SUCCESS = 0,
    DRAG_FAIL = 1,
    DRAG_CANCEL = 2,
    DRAG_EXCEPTION = 3
};

struct DragDropResult {
    DragResult result { DragResult::DRAG_FAIL };
    bool hasCustomAnimation { false };
    int32_t windowId { -1 };
};

struct DragNotifyMsg {
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t targetPid { -1 };
    DragResult result { DragResult::DRAG_FAIL };
};

struct DragAnimationData {
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t offsetX { -1 };
    int32_t offsetY { -1 };
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap { nullptr };
};

struct DragItemStyle {
    uint32_t foregroundColor { 0 };
    int32_t radius { 0 };
    uint32_t alpha { 0 };

    bool operator == (const DragItemStyle &style) const
    {
        return foregroundColor == style.foregroundColor &&
               radius == style.radius && alpha == style.alpha;
    }

    bool operator!=(const DragItemStyle &style) const
    {
        return !(*this == style);
    }
};

enum class DragCursorStyle {
    DEFAULT = 0,
    FORBIDDEN,
    COPY,
    MOVE
};

enum class DragAction {
    INVALID = -1,
    MOVE = 0,
    COPY = 1
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DATA_H