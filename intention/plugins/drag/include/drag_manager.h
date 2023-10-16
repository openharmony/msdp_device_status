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

#ifndef DRAG_MANAGER_H
#define DRAG_MANAGER_H

#include "pixel_map.h"

#include "drag_data.h"
#include "drag_drawing.h"
#include "intention_define.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManager {
public:
    DragManager() {}
    DISALLOW_COPY_AND_MOVE(DragManager);
    ~DragManager() = default;

public:
    int32_t AddListener(SessionPtr session);
    int32_t RemoveListener(SessionPtr session);
    int32_t StartDrag(const DragData &dragData, SessionPtr sess);
    int32_t StopDrag(DragResult result, bool hasCustomAnimation);
    int32_t GetDragTargetPid() const;
    int32_t GetUdKey(std::string &udKey) const;
    int32_t UpdateDragStyle(DragCursorStyle style, int32_t targetPid, int32_t targetTid);
    int32_t UpdateShadowPic(const ShadowInfo &shadowInfo);
    int32_t SetDragWindowVisible(bool visible);

private:
    int32_t InitDataManager(const DragData &dragData) const;
    int32_t OnStartDrag();
    int32_t OnStopDrag(DragResult result, bool hasCustomAnimation);
    void StateChangedNotify(DragState state);

private:
    StateChangeNotify stateNotify_;
    DragState dragState_ { DragState::STOP };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_H
