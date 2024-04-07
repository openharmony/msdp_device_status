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

#include "input_event_transmission/input_event_interceptor.h"

#include "cooperate_context.h"
#include "devicestatus_define.h"
#include "display_manager.h"
#include "input_event_transmission/input_event_serialization.h"

#undef LOG_TAG
#define LOG_TAG "InputEventInterceptor"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr float HALF_RATIO { 0.5 };
} // namespace

std::set<int32_t> InputEventInterceptor::filterKeys_ {
    MMI::KeyEvent::KEYCODE_BACK,
    MMI::KeyEvent::KEYCODE_VOLUME_UP,
    MMI::KeyEvent::KEYCODE_VOLUME_DOWN,
    MMI::KeyEvent::KEYCODE_POWER,
};

InputEventInterceptor::~InputEventInterceptor()
{
    Disable();
}

void InputEventInterceptor::Enable(Context &context)
{
    CALL_INFO_TRACE;
    if (interceptorId_ > 0) {
        return;
    }
    cursorPos_ = context.CursorPosition();
    remoteNetworkId_ = context.Peer();
    startDeviceId_ = context.StartDeviceId();
    sender_ = context.Sender();
    FI_HILOGI("Cursor transite out at (%{public}d, %{public}d)", cursorPos_.x, cursorPos_.y);
    SetupBound();
    CheckBound();

    interceptorId_ = env_->GetInput().AddInterceptor(
        std::bind(&InputEventInterceptor::OnPointerEvent, this, std::placeholders::_1),
        std::bind(&InputEventInterceptor::OnKeyEvent, this, std::placeholders::_1));
    if (interceptorId_ < 0) {
        FI_HILOGE("Input::AddInterceptor fail");
    }
}

void InputEventInterceptor::Disable()
{
    CALL_INFO_TRACE;
    if (interceptorId_ > 0) {
        env_->GetInput().RemoveInterceptor(interceptorId_);
        interceptorId_ = -1;
    }
}

void InputEventInterceptor::Update(Context &context)
{
    CALL_INFO_TRACE;
    remoteNetworkId_ = context.Peer();
}

void InputEventInterceptor::SetupBound()
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        FI_HILOGE("No default display");
        displayBound_ = {};
        cursorBound_ = {};
        return;
    }
    displayBound_ = Rectangle {
        .width = display->GetWidth(),
        .height = display->GetHeight(),
    };
    if ((displayBound_.width <= 0) || (displayBound_.height <= 0)) {
        FI_HILOGE("Invalid display information");
        return;
    }
    displayBound_.x = static_cast<int32_t>(displayBound_.width * HALF_RATIO);
    displayBound_.y = static_cast<int32_t>(displayBound_.height * HALF_RATIO);

    constexpr double boundRatio { 0.8 };
    cursorBound_.width = static_cast<int32_t>(displayBound_.width * boundRatio);
    cursorBound_.height = static_cast<int32_t>(displayBound_.height * boundRatio);
    cursorBound_.x = (displayBound_.x - static_cast<int32_t>(cursorBound_.width * HALF_RATIO));
    cursorBound_.y = (displayBound_.y - static_cast<int32_t>(cursorBound_.height * HALF_RATIO));
    FI_HILOGI("Display bound (%{public}d, %{public}d)[%{public}d, %{public}d]",
        displayBound_.x, displayBound_.y, displayBound_.width, displayBound_.height);
    FI_HILOGI("Cursor bound (%{public}d, %{public}d)[%{public}d, %{public}d]",
        cursorBound_.x, cursorBound_.y, cursorBound_.width, cursorBound_.height);
}

void InputEventInterceptor::CheckBound()
{
    if ((cursorBound_.width <= 0) || (cursorBound_.height <= 0)) {
        return;
    }
    if ((cursorPos_.x < cursorBound_.x) ||
        (cursorPos_.x >= (cursorBound_.x + cursorBound_.width)) ||
        (cursorPos_.y < cursorBound_.y) ||
        (cursorPos_.y >= (cursorBound_.y + cursorBound_.height))) {
        inTrans_ = true;
        transCursorPos_ = cursorPos_;
        env_->GetInput().SetPointerVisibility(false);
        env_->GetInput().SetPointerLocation(displayBound_.x, displayBound_.y);
        FI_HILOGD("Cursor is out of bound, reset to (%{public}d, %{public}d)", displayBound_.x, displayBound_.y);
    }
}

void InputEventInterceptor::CheckTrans(const Coordinate &cursorPos)
{
    if (!inTrans_) {
        return;
    }
    double d1 = ::hypot(cursorPos.x - transCursorPos_.x, cursorPos.y - transCursorPos_.y);
    double d2 = ::hypot(cursorPos.x - displayBound_.x, cursorPos.y - displayBound_.y);
    if (d2 < d1) {
        inTrans_ = false;
        cursorPos_.x = displayBound_.x;
        cursorPos_.y = displayBound_.y;
    }
}

void InputEventInterceptor::OnPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CHKPV(pointerEvent);
    if (pointerEvent->GetDeviceId() != startDeviceId_) {
        ReportPointerEvent(pointerEvent);
        return;
    }
    if (!DifferentiateMotion(pointerEvent)) {
        return;
    }
    NetPacket packet(MessageId::DSOFTBUS_INPUT_POINTER_EVENT);

    int32_t ret = InputEventSerialization::Marshalling(pointerEvent, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to serialize pointer event");
        return;
    }
    env_->GetDSoftbus().SendPacket(remoteNetworkId_, packet);
}

void InputEventInterceptor::OnKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CHKPV(keyEvent);
    if (filterKeys_.find(keyEvent->GetKeyCode()) != filterKeys_.end()) {
        keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
        env_->GetInput().SimulateInputEvent(keyEvent);
        return;
    }
    NetPacket packet(MessageId::DSOFTBUS_INPUT_KEY_EVENT);

    int32_t ret = InputEventSerialization::KeyEventToNetPacket(keyEvent, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to serialize key event");
        return;
    }
    env_->GetDSoftbus().SendPacket(remoteNetworkId_, packet);
}

void InputEventInterceptor::ReportPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    MMI::PointerEvent::PointerItem pointerItem;

    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        FI_HILOGE("Corrupted pointer event");
        return;
    }
    sender_.Send(CooperateEvent(
        CooperateEventType::INPUT_POINTER_EVENT,
        InputPointerEvent {
            .deviceId = pointerEvent->GetDeviceId(),
            .pointerAction = pointerEvent->GetPointerAction(),
            .sourceType = pointerEvent->GetSourceType(),
            .position = Coordinate {
                .x = pointerItem.GetDisplayX(),
                .y = pointerItem.GetDisplayY(),
            }
        }));
}

bool InputEventInterceptor::DifferentiateMotion(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    if ((pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        ((pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE) &&
         (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_PULL_MOVE))) {
        return true;
    }
    MMI::PointerEvent::PointerItem item;

    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), item)) {
        FI_HILOGE("Corrupted pointer event");
        return false;
    }
    CheckTrans(Coordinate {
        .x = item.GetDisplayX(),
        .y = item.GetDisplayY(),
    });
    Coordinate delta {
        .x = (item.GetDisplayX() - cursorPos_.x),
        .y = (item.GetDisplayY() - cursorPos_.y),
    };
    FI_HILOGD("Cursor moved [%{public}d](%{public}d, %{public}d)", pointerEvent->GetId(), delta.x, delta.y);
    cursorPos_.x = item.GetDisplayX();
    cursorPos_.y = item.GetDisplayY();
    item.SetDisplayX(delta.x);
    item.SetDisplayY(delta.y);
    pointerEvent->UpdatePointerItem(pointerEvent->GetPointerId(), item);
    CheckBound();
    return true;
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
