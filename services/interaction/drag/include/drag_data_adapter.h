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

#ifndef DRAG_DATA_ADAPTER_H
#define DRAG_DATA_ADAPTER_H
#include <string>

#include "hitrace_meter.h"
#include "pixel_map.h"
#include "pointer_style.h"
#include "singleton.h"

#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragDataAdapter final {
    DECLARE_SINGLETON(DragDataAdapter);

public:
    DISALLOW_MOVE(DragDataAdapter);

    void Init(const DragData &dragData, const MMI::PointerStyle &pointerStyle);
    DragData GetDragData() const;
    int32_t GetDragStyle() const;
    std::u16string GetDragMessage() const;
    void SetDragWindowVisible(bool visible);
    bool GetDragWindowVisible() const;
    int32_t GetShadowOffset(int32_t& offsetX, int32_t& offsetY) const;
    
private:
    DragData dragData_;
    OHOS::MMI::PointerStyle pointerStyle_;
    int32_t dragStyle_ { -1 };
    std::u16string dragMessage_;
    bool visible_ { false };
};

#define DataAdapter ::OHOS::Singleton<DragDataAdapter>::GetInstance()

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DATA_ADAPTER_H