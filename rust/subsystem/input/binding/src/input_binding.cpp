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

#include "input_binding.h"

#include <vector>

#include "input_manager.h"

#include "devicestatus_define.h"
#include "input_binding_internal.h"

#undef LOG_TAG
#define LOG_TAG "InputBinding"

namespace {
#define INPUT_MANAGER OHOS::MMI::InputManager::GetInstance()
} // namespace

DragMonitorConsumer::DragMonitorConsumer(void (*cb)(CPointerEvent *)) : callback_(cb)
{
}

void DragMonitorConsumer::OnInputEvent(std::shared_ptr<OHOS::MMI::AxisEvent> axisEvent) const
{
}

void DragMonitorConsumer::OnInputEvent(std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) const
{
}

void DragMonitorConsumer::OnInputEvent(std::shared_ptr<OHOS::MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    auto cPointerEvent = new (std::nothrow) CPointerEvent(pointerEvent);
    CHKPV(cPointerEvent);
    CHKPV(callback_);
    callback_(cPointerEvent);
}

int32_t CGetPointerId(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return cPointerEvent->event->GetPointerId();
}

int32_t CGetPointerAction(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    auto ret = cPointerEvent->event->GetPointerAction();
    FI_HILOGD("action:%{public}d", ret);
    return ret;
}

int32_t CGetTargetWindowId(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return cPointerEvent->event->GetTargetWindowId();
}

int32_t CGetSourceType(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return cPointerEvent->event->GetSourceType();
}

int32_t CGetTargetDisplayId(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return cPointerEvent->event->GetTargetDisplayId();
}

int32_t CGetDisplayX(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    OHOS::MMI::PointerEvent::PointerItem pointerItem;
    cPointerEvent->event->GetPointerItem(CGetPointerId(cPointerEvent), pointerItem);
    return pointerItem.GetDisplayX();
}

int32_t CGetDisplayY(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    OHOS::MMI::PointerEvent::PointerItem pointerItem;
    cPointerEvent->event->GetPointerItem(CGetPointerId(cPointerEvent), pointerItem);
    return pointerItem.GetDisplayY();
}

void CPointerEventAddFlag(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(cPointerEvent);
    CHKPV(cPointerEvent->event);
    cPointerEvent->event->AddFlag(OHOS::MMI::InputEvent::EVENT_FLAG_NO_INTERCEPT);
}

void CKeyEventAddFlag(const CKeyEvent* cKeyEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(cKeyEvent);
    CHKPV(cKeyEvent->event);
    cKeyEvent->event->AddFlag(OHOS::MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
}

int32_t CGetDeviceId(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return cPointerEvent->event->GetDeviceId();
}

int32_t CGetKeyCode(const CKeyEvent* cKeyEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cKeyEvent, RET_ERR);
    CHKPR(cKeyEvent->event, RET_ERR);
    return cKeyEvent->event->GetKeyCode();
}

int32_t CAddMonitor(void (*callback)(CPointerEvent *))
{
    CALL_DEBUG_ENTER;
    CHKPR(callback, RET_ERR);
    auto consumer = std::make_shared<DragMonitorConsumer>(callback);
    auto ret = INPUT_MANAGER->AddMonitor(consumer);
    if (ret < 0) {
        FI_HILOGE("Failed to add monitor");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CGetWindowPid(const CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerEvent, RET_ERR);
    CHKPR(cPointerEvent->event, RET_ERR);
    return INPUT_MANAGER->GetWindowPid(CGetTargetWindowId(cPointerEvent));
}

int32_t CGetPointerStyle(CPointerStyle* cPointerStyle)
{
    CALL_DEBUG_ENTER;
    CHKPR(cPointerStyle, RET_ERR);
    OHOS::MMI::PointerStyle pointerStyle;
    if (INPUT_MANAGER->GetPointerStyle(OHOS::MMI::GLOBAL_WINDOW_ID, pointerStyle) != RET_OK) {
        FI_HILOGE("Failed to get pointer style");
        return RET_ERR;
    }

    cPointerStyle->size = pointerStyle.size;
    cPointerStyle->color = pointerStyle.color;
    cPointerStyle->id = pointerStyle.id;
    return RET_OK;
}

void CAppendExtraData(CExtraData cExtraData)
{
    CALL_DEBUG_ENTER;
    CHKPV(cExtraData.buffer);
    uint8_t* uData = cExtraData.buffer;
    CHKPV(uData);
    OHOS::MMI::ExtraData extraData;
    extraData.appended = cExtraData.appended;
    extraData.pointerId = cExtraData.pointerId;
    extraData.sourceType = cExtraData.sourceType;
    for (size_t i = 0; i < cExtraData.bufferSize; ++i) {
        extraData.buffer.emplace_back(*uData);
        ++uData;
    }
    INPUT_MANAGER->AppendExtraData(extraData);
}

int32_t CSetPointerVisible(bool visible)
{
    CALL_DEBUG_ENTER;
    if (INPUT_MANAGER->SetPointerVisible(visible) != RET_OK) {
        FI_HILOGE("Failed to set pointer visible");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CEnableInputDevice(bool enable)
{
    CALL_DEBUG_ENTER;
    if (INPUT_MANAGER->EnableInputDevice(enable) != RET_OK) {
        FI_HILOGE("Failed to enable input device");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CRemoveInputEventFilter(int32_t filterId)
{
    CALL_DEBUG_ENTER;
    if (INPUT_MANAGER->RemoveInputEventFilter(filterId) != RET_OK) {
        FI_HILOGE("Failed to remove input event filter");
        return RET_ERR;
    }
    return RET_OK;
}

void CRemoveMonitor(int32_t monitorId)
{
    CALL_DEBUG_ENTER;
    INPUT_MANAGER->RemoveMonitor(monitorId);
}

void CRemoveInterceptor(int32_t interceptorId)
{
    CALL_DEBUG_ENTER;
    INPUT_MANAGER->RemoveInterceptor(interceptorId);
}

void CSetPointerLocation(int32_t physicalX, int32_t physicalY)
{
    CALL_DEBUG_ENTER;
    INPUT_MANAGER->SetPointerLocation(physicalX, physicalY);
}

void CDestroyPointerEvent(CPointerEvent* cPointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(cPointerEvent);
    CHKPV(cPointerEvent->event);
    cPointerEvent->event = nullptr;
    delete cPointerEvent;
}