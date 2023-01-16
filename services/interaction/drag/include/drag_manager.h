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

#include <functional>
#include <memory>
#include <vector>

#include "drag_data.h"
#include "drag_drawing.h"
#include "devicestatus_define.h"
#include "input_manager.h"
#include "i_input_event_consumer.h"
#include "pixel_map.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManager {
public:
    class MonitorConsumer : public MMI::IInputEventConsumer {
    public:
        explicit MonitorConsumer(std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : callback_(cb) {}
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_;
    };

public:
    DragManager() {
        monitorConsumer_ = std::make_shared<MonitorConsumer>(nullptr);
    }
    ~DragManager() = default;

    int32_t StartDrag(const DragData &dragData, int32_t pid);
    int32_t StopDrag(int32_t &dragResult);
    int32_t GetDragTargetPid();
    int32_t AddMonitor(int32_t pid);
    int32_t RemoveMonitor(int32_t monitorId);

private:
    int32_t NotifyMonitor(DragState dragState);
    MMI::ExtraData ConstructExtraData(const DragData &dragData, bool appended);
    void TestStartDrag(const DragData &dragData, int32_t pid);

private:
    DragState dragState_ { DragState::FREE };
    int32_t monitorId_ { -1 };
    int32_t dragOutPid_ { -1 };
    int32_t dragTargetPid_ { -1 };
    std::vector<int32_t> monitors_;
    std::shared_ptr<MonitorConsumer> monitorConsumer_;
    DragDrawing dragDrawing_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_H