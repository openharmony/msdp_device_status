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

#ifndef DRAG_DATA_MANAGER_H
#define DRAG_DATA_MANAGER_H

#include <string>

#include "pixel_map.h"
#include "pointer_style.h"
#include "singleton.h"

#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragDataManager final {
    DECLARE_SINGLETON(DragDataManager);

public:
    DISALLOW_MOVE(DragDataManager);

    void Init(const DragData &dragData);
    DragData GetDragData() const;
    void SetDragStyle(DragCursorStyle style);
    void SetShadowInfo(const ShadowInfo &shadowInfo);
    DragCursorStyle GetDragStyle() const;
    void SetDragWindowVisible(bool visible);
    bool GetDragWindowVisible() const;
    int32_t GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height) const;
    void ResetDragData();
    void SetTargetTid(int32_t tokenId);
    int32_t GetTargetTid() const;
    void SetTargetPid(int32_t pid);
    int32_t GetTargetPid() const;
    void SetMotionDrag(bool isMotionDrag);
    bool IsMotionDrag() const;
    void SetDragItemStyle(const DragItemStyle &dragItemStyle);
    DragItemStyle GetDragItemStyle();
private:
    DragData dragData_;
    DragCursorStyle dragStyle_ { DragCursorStyle::DEFAULT };
    std::u16string dragMessage_;
    bool visible_ { false };
    int32_t targetTid_ { -1 };
    int32_t targetPid_ { -1 };
    bool isMotionDrag_ { false };
    DragItemStyle dragItemStyle_;
};

#define DRAG_DATA_MGR OHOS::Singleton<DragDataManager>::GetInstance()

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DATA_MANAGER_H