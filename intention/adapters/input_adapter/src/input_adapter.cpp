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

#include "input_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

bool InputAdapter::PointerFilter::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    pointerCallback_(pointerEvent);
}

void InputAdapter::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    CHKPV(keyEvent);
    keyCallback_(keyEvent);
}

void InputAdapter::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    pointerCallback_(pointerEvent);
}

void InputAdapter::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const {}

void InputAdapter::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const {}

void InputAdapter::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    callback_(pointerEvent);
}

void InputAdapter::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent>) const {}

int32_t InputAdapter::AddMonitor(std::function<void(std::shared_ptr<MMI::PointerEvent>)> callback)
{
    auto monitor = std::make_shared<MonitorConsumer>(callback);
    monitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor);
    if (monitorId_ <= 0) {
        FI_HILOGE("Failed to add monitor, error code:%{public}d", monitorId_);
        monitorId_ = -1;
        return RET_ERR;
    }
    return RET_OK;
}

int32_t InputAdapter::AddInterceptor(std::function<void(std::shared_ptr<MMI::PointerEvent>)> pointerCallback,
    std::function<void(std::shared_ptr<MMI::KeyEvent>)> keyCallback, uint32_t deviceTags)
{
    auto interceptor = std::make_shared<InterceptorConsumer>(pointerCallback, keyCallback);
    interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COORDINATION_PRIORITY, deviceTags);
    if (interceptorId_ <= 0) {
        FI_HILOGE("Failed to add interceptor, error code:%{public}d", interceptorId_);
        DeactivateCoordination(isUnchained_);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t InputAdapter::AddFilter(std::function<void(std::shared_ptr<MMI::PointerEvent>)> callback)
{
    int32_t POINTER_DEFAULT_PRIORITY = 220;
    auto filter = std::make_shared<PointerFilter>(callback);
    uint32_t touchTags = CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER);
    FI_HILOGE("touchtags:%{public}d", static_cast<int32_t>(touchTags));
    if (filterId_ >= 0) {
        RemoveFilter(filterId_);
    }
    filterId_ =
        MMI::InputManager::GetInstance()->AddInputEventFilter(filter, POINTER_DEFAULT_PRIORITY, touchTags);
    if (filterId_ < 0) {
        FI_HILOGE("Add Event Filter failed");
    }
    filter->UpdateCurrentFilterId(filterId_);
}

void InputAdapter::RemoveMonitor()
{
    if ((monitorId_ >= MIN_HANDLER_ID) && (monitorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveMonitor(monitorId_);
        monitorId_ = -1;
    }
}

void InputAdapter::RemoveInterceptor()
{
    if ((interceptorId_ >= MIN_HANDLER_ID) && (interceptorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveInterceptor(interceptorId_);
        interceptorId_ = -1;
    }
}

void InputAdapter::RemoveFilter(int32_t filterId)
{
    MMI::InputManager::GetInstance()->RemoveInputEventFilter(filterId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS