/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef DRAG_H
#define DRAG_H
#include <functional>
#include <memory>
#include "drag_data.h"
#include "pixel_map.h"
#include "input_manager.h"


namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Drag {
public:
    enum class DragState{
        NORMAL = -1,
        START_DRAG = 0,
        DRAGGING = 1,
        STOP_DRAG = 2,
    };

    enum class DragResult {
        DRAG_SUCCESS = 0,
        DRAG_FAIL = 1,
        DRAG_CANCEL = 2,
    };

    using FuncDragCallback = std::function<void(int32_t)>;

    class MonitorConsumer : public MMI::IInputEventConsumer {
    public:
        explicit MonitorConsumer(std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : callback_(cb) {}
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_;
    };

public:
    Drag();
    virtual ~Drag();

    void Init();

    int32_t StartDrag(const DragData &dragData);

    int32_t StopDrag(int32_t &dragResult);

    void OnStop();

private:
    bool IsStarting() const;

    bool IsStopping() const;

    bool IsDragging() const;

    bool IsNormal() const;

    void SetExtraData(ExtraData &extraData);


private:
    DragState dragState_ { DragState::NORMAL };
    bool isStarting_ { false };
    bool isSTopping_ { false };
    int32_t monitorId_ { -1 };
    int32_t callingPid_ { -1 };
    DragData dragData_;
};
} // namespace Drag
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_H