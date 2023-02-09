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

#include <string>

#include "i_input_event_consumer.h"
#include "input_manager.h"
#include "pixel_map.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "state_change_notify.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManager {
public:
    DragManager() = default;
    ~DragManager() = default;

    void OnSessionLost(SessionPtr session);
    int32_t AddListener(SessionPtr session);
    int32_t RemoveListener(SessionPtr session);
    int32_t StartDrag(const DragData &dragData, SessionPtr sess);
    int32_t StopDrag(int32_t result);
    int32_t GetDragTargetPid() const;
    int32_t OnRegisterThumbnailDraw(SessionPtr sess);
    int32_t OnUnregisterThumbnailDraw(SessionPtr sess);
private:
    StateChangeNotify stateNotify_;
    DragState dragState_ { DragState::FREE };
    SessionPtr dragOutSession_ { nullptr };
    int32_t dragTargetPid_ { -1 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_H
