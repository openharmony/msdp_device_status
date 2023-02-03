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

#ifndef DRAG_DRAWING_H
#define DRAG_DRAWING_H

#include "pixel_map.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragDrawing {
public:
    void InitPicture(const OHOS::Media::PixelMap &pixelMap, int32_t x, int32_t y);
    void Draw(int32_t x, int32_t y);
private:
    void DrawMessage() {}
    void DrawStyle() {}
    void DrawPicture() {}
};
} // namespace DragDrawing
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DRAWING_H