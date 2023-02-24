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
#include <memory>
#include <vector>

#include "pixel_map.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
static const int32_t MAX_PIXEL_MAP_WIDTH = 200;
static const int32_t MAX_PIXEL_MAP_HEIGHT = 200;
static const int32_t MAX_BUFFER_SIZE = 512;
struct PictureResourse {
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap { nullptr };
    int32_t x { -1 };
    int32_t y { -1 };
};

struct DragData {
    PictureResourse pictureResourse;
    std::vector<uint8_t> buffer;
    int32_t sourceType { -1 };
    int32_t dragNum { -1 };
    int32_t pointerId { -1 };
};

enum class DragState {
    FREE = 0,
    DRAGGING = 1
};

enum class DragCursorStyle {
    FORBIDDEN,
    ALLOWABLE
};

enum class DragResult {
    DRAG_SUCCESS = 0,
    DRAG_FAIL = 1,
    DRAG_CANCEL = 2
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DATA_H