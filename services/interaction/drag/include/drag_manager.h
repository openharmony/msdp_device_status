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

#include "extra_data.h"
#include "i_context.h"
#include "i_input_event_consumer.h"
#include "input_manager.h"
#include "pixel_map.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "drag_drawing.h"
#include "state_change_notify.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManager {
public:
    DragManager()
    {}
    ~DragManager() = default;
    DISALLOW_COPY_AND_MOVE(DragManager);

    int32_t Init(IContext* context);
    void OnSessionLost(SessionPtr session);
    int32_t AddListener(SessionPtr session);
    int32_t RemoveListener(SessionPtr session);
    int32_t StartDrag(const DragData &dragData, SessionPtr sess);
    int32_t StopDrag(DragResult result, bool hasCustomAnimation);
    int32_t GetDragTargetPid() const;
    int32_t GetUdKey(std::string &udKey);
    void SetDragTargetPid(int32_t dragTargetPid);
    void SendDragData(int32_t targetPid, const std::string &udKey);
    int32_t UpdateDragStyle(DragCursorStyle style);
    void DragCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnDragUp(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void OnDragMove(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    int32_t OnSetDragWindowVisible(bool visible);
    int32_t OnGetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height);
    class InterceptorConsumer final : public MMI::IInputEventConsumer {
    public:
        InterceptorConsumer(IContext *context,
            std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : context_(context), callback_(cb)
        {}
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    private:
        IContext* context_ { nullptr };
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_;
    };
private:
    int32_t AddDragEventInterceptor(int32_t sourceType);
    int32_t NotifyDragResult(DragResult result);
    OHOS::MMI::ExtraData CreateExtraData(bool appended) const;
    int32_t InitDataAdapter(const DragData &dragData) const;
    int32_t OnStartDrag();
    int32_t OnStopDrag(DragResult result, bool hasCustomAnimation);
private:
    int32_t timerId_ { 0 };
    StateChangeNotify stateNotify_;
    DragMessage dragState_ { DragMessage::MSG_DRAG_STATE_STOP };
    int32_t interceptorId_ { -1 };
    int32_t dragTargetPid_ { -1 };
    SessionPtr dragOutSession_ { nullptr };
    DragDrawing dragDrawing_;
    IContext* context_ { nullptr };
};
#define INPUT_MANAGER  OHOS::MMI::InputManager::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_H
