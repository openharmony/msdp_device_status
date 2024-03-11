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

#include "input_event_transmission/input_event_builder.h"

#include "display_info.h"

#include "cooperate_context.h"
#include "devicestatus_define.h"
#include "input_event_transmission/input_event_serialization.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "InputEventBuilder"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

InputEventBuilder::InputEventBuilder(IContext *env)
    : env_(env)
{
    observer_ = std::make_shared<DSoftbusObserver>(*this);
    pointerEvent_ = MMI::PointerEvent::Create();
    keyEvent_ = MMI::KeyEvent::Create();
}

InputEventBuilder::~InputEventBuilder()
{
    Disable();
}

void InputEventBuilder::Enable(Context &context)
{
    CALL_INFO_TRACE;
    if (enable_) {
        return;
    }
    enable_ = true;
    cursorPos_.pos = context.CursorPosition();
    remoteNetworkId_ = context.Peer();
    env_->GetDSoftbus().AddObserver(observer_);
    FI_HILOGI("Cursor transite in (%{public}d, %{public}d)", cursorPos_.pos.x, cursorPos_.pos.y);
}

void InputEventBuilder::Disable()
{
    CALL_INFO_TRACE;
    if (enable_) {
        enable_ = false;
        env_->GetDSoftbus().RemoveObserver(observer_);
    }
}

void InputEventBuilder::Update(Context &context)
{
    CALL_INFO_TRACE;
    remoteNetworkId_ = context.Peer();
}

bool InputEventBuilder::OnPacket(const std::string &networkId, Msdp::NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    if (networkId != remoteNetworkId_) {
        return false;
    }
    switch (packet.GetMsgId()) {
        case MessageId::DSOFTBUS_INPUT_POINTER_EVENT: {
            OnPointerEvent(packet);
            break;
        }
        case MessageId::DSOFTBUS_INPUT_KEY_EVENT: {
            OnKeyEvent(packet);
            break;
        }
        default: {
            return false;
        }
    }
    return true;
}

void InputEventBuilder::OnPointerEvent(Msdp::NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent_);
    int32_t ret = InputEventSerialization::Unmarshalling(packet, pointerEvent_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to deserialize pointer event");
        return;
    }
    if (!AccumulateMotion(pointerEvent_) ||
        !UpdatePointerEvent(pointerEvent_)) {
        return;
    }
    env_->GetInput().SimulateInputEvent(pointerEvent_);
    pointerEvent_->Reset();
}

void InputEventBuilder::OnKeyEvent(Msdp::NetPacket &packet)
{
    CALL_DEBUG_ENTER;
    CHKPV(keyEvent_);
    int32_t ret = InputEventSerialization::NetPacketToKeyEvent(packet, keyEvent_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to deserialize key event");
        return;
    }
    env_->GetInput().SimulateInputEvent(keyEvent_);
    ResetKeyEvent(keyEvent_);
}

bool InputEventBuilder::AccumulateMotion(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    if ((pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) ||
        (pointerEvent->GetPointerAction() != MMI::PointerEvent::POINTER_ACTION_MOVE)) {
        return true;
    }
    MMI::PointerEvent::PointerItem item;

    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), item)) {
        FI_HILOGE("Corrupted pointer event");
        return false;
    }
    cursorPos_.pos.x += item.GetDisplayX();
    cursorPos_.pos.y += item.GetDisplayY();

    if (cursorPos_.displayId < 0) {
        auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        CHKPF(display);
        cursorPos_.displayId = display->GetId();
    }
    auto targetDisplay = Rosen::DisplayManager::GetInstance().GetDisplayById(cursorPos_.displayId);
    if (targetDisplay == nullptr) {
        targetDisplay = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
        CHKPF(targetDisplay);
    }
    if (IsInDisplay(targetDisplay)) {
        return true;
    }
    std::vector<sptr<Rosen::Display>> displays = Rosen::DisplayManager::GetInstance().GetAllDisplays();

    for (const auto &display : displays) {
        if ((display != nullptr) && IsInDisplay(display)) {
            targetDisplay = display;
            break;
        }
    }
    sptr<Rosen::DisplayInfo> targetDisplayInfo = targetDisplay->GetDisplayInfo();
    CHKPF(targetDisplayInfo);
    cursorPos_.displayId = targetDisplay->GetId();
    cursorPos_.pos.x = std::clamp<int32_t>(cursorPos_.pos.x, targetDisplayInfo->GetOffsetX(),
        targetDisplayInfo->GetOffsetX() + targetDisplayInfo->GetWidth() - 1);
    cursorPos_.pos.y = std::clamp<int32_t>(cursorPos_.pos.y, targetDisplayInfo->GetOffsetY(),
        targetDisplayInfo->GetOffsetY() + targetDisplayInfo->GetHeight() - 1);
    FI_HILOGD("Cursor move to [%{public}d](%{public}d, %{public}d)",
        pointerEvent->GetId(), cursorPos_.pos.x, cursorPos_.pos.y);
    return true;
}

bool InputEventBuilder::IsInDisplay(sptr<Rosen::Display> display) const
{
    sptr<Rosen::DisplayInfo> displayInfo = display->GetDisplayInfo();
    CHKPF(displayInfo);
    return ((cursorPos_.pos.x >= displayInfo->GetOffsetX()) &&
            (cursorPos_.pos.x < displayInfo->GetOffsetX() + displayInfo->GetWidth()) &&
            (cursorPos_.pos.y >= displayInfo->GetOffsetY()) &&
            (cursorPos_.pos.y < displayInfo->GetOffsetY() + displayInfo->GetHeight()));
}

bool InputEventBuilder::UpdatePointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    if (pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        return true;
    }
    MMI::PointerEvent::PointerItem item;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), item)) {
        FI_HILOGE("Corrupted pointer event");
        return false;
    }
    item.SetDisplayX(cursorPos_.pos.x);
    item.SetDisplayY(cursorPos_.pos.y);
    pointerEvent->UpdatePointerItem(pointerEvent->GetPointerId(), item);

    int64_t time = Utility::GetSysClockTime();
    pointerEvent->SetActionTime(time);
    pointerEvent->SetActionStartTime(time);
    pointerEvent->SetTargetDisplayId(cursorPos_.displayId);
    pointerEvent->SetTargetWindowId(-1);
    pointerEvent->SetAgentWindowId(-1);
    // Tag remote event.
    pointerEvent->SetDeviceId(
        (pointerEvent->GetDeviceId() >= 0) ?
        -(pointerEvent->GetDeviceId() + 1) :
        pointerEvent->GetDeviceId());
    return true;
}

void InputEventBuilder::ResetKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    std::vector<MMI::KeyEvent::KeyItem> keys = keyEvent->GetKeyItems();

    for (MMI::KeyEvent::KeyItem &item : keys) {
        item.SetPressed(false);
        keyEvent->RemoveReleasedKeyItems(item);
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
