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
    remoteNetworkId_ = context.Peer();
    env_->GetDSoftbus().AddObserver(observer_);
    Coordinate cursorPos = context.CursorPosition();
    FI_HILOGI("Cursor transite in (%{public}d, %{public}d)", cursorPos.x, cursorPos.y);
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
    remoteNetworkId_ = context.Peer();
    FI_HILOGI("Update peer to \'%{public}s\'", Utility::Anonymize(remoteNetworkId_).c_str());
}

bool InputEventBuilder::OnPacket(const std::string &networkId, Msdp::NetPacket &packet)
{
    if (networkId != remoteNetworkId_) {
        FI_HILOGW("Unexpected packet from \'%{public}s\'", Utility::Anonymize(networkId).c_str());
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
            FI_HILOGW("Unexpected message(%{public}d) from \'%{public}s\'",
                static_cast<int32_t>(packet.GetMsgId()), Utility::Anonymize(networkId).c_str());
            return false;
        }
    }
    return true;
}

void InputEventBuilder::OnPointerEvent(Msdp::NetPacket &packet)
{
    CHKPV(pointerEvent_);
    int32_t ret = InputEventSerialization::Unmarshalling(packet, pointerEvent_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to deserialize pointer event");
        return;
    }
    if (!UpdatePointerEvent(pointerEvent_)) {
        return;
    }
    TagRemoteEvent(pointerEvent_);
    FI_HILOGI("PointerEvent(No:%{public}d,Source:%{public}s,Action:%{public}s)",
        pointerEvent_->GetId(), pointerEvent_->DumpSourceType(), pointerEvent_->DumpPointerAction());
    env_->GetInput().SimulateInputEvent(pointerEvent_);
    pointerEvent_->Reset();
}

void InputEventBuilder::OnKeyEvent(Msdp::NetPacket &packet)
{
    CHKPV(keyEvent_);
    int32_t ret = InputEventSerialization::NetPacketToKeyEvent(packet, keyEvent_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to deserialize key event");
        return;
    }
    FI_HILOGD("KeyEvent(No:%{public}d,Key:%{public}d,Action:%{public}d)",
        keyEvent_->GetId(), keyEvent_->GetKeyCode(), keyEvent_->GetKeyAction());
    env_->GetInput().SimulateInputEvent(keyEvent_);
    keyEvent_->Reset();
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
    pointerEvent->AddFlag(MMI::InputEvent::EVENT_FLAG_RAW_POINTER_MOVEMENT);
    int64_t time = Utility::GetSysClockTime();
    pointerEvent->SetActionTime(time);
    pointerEvent->SetActionStartTime(time);
    pointerEvent->SetTargetDisplayId(-1);
    pointerEvent->SetTargetWindowId(-1);
    pointerEvent->SetAgentWindowId(-1);
    return true;
}
void InputEventBuilder::TagRemoteEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    pointerEvent->SetDeviceId(
        (pointerEvent->GetDeviceId() >= 0) ?
        -(pointerEvent->GetDeviceId() + 1) :
        pointerEvent->GetDeviceId());
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
