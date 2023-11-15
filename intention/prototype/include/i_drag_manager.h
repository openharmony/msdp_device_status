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

#ifndef I_DRAG_MANAGER_H
#define I_DRAG_MANAGER_H

#include <cstdint>
#include <functional>

#include <input_manager.h>

#include "drag_data.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDragManager {
public:
    IDragManager() = default;
    virtual ~IDragManager() = default;

    virtual void Dump(int32_t fd) const = 0;
    virtual void RegisterStateChange(std::function<void(DragState)> callback) = 0;
    virtual int32_t StartDrag(const DragData &dragData, SessionPtr sess) = 0;
    virtual int32_t StopDrag(const DragDropResult &dropResult) = 0;
    virtual DragState GetDragState() const = 0;
    virtual void SetDragState(DragState state) = 0;
    virtual DragResult GetDragResult() const = 0;
    virtual int32_t OnSetDragWindowVisible(bool visible) = 0;
    virtual OHOS::MMI::ExtraData GetExtraData(bool appended) const = 0;
    virtual void RegisterNotifyPullUp(std::function<void(void)> callback) = 0;
    virtual void SetPointerEventFilterTime(int64_t filterTime) = 0;
    virtual void MoveTo(int32_t x, int32_t y) = 0;
    virtual void GetAllowDragState(bool &isAllowDrag) = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_DRAG_MANAGER_H