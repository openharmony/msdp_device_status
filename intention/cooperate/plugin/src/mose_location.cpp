/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mouse_location.h"

#include "display_manager.h"

#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "HotArea"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr int32_t HOT_AREA_WIDTH { 100 };
constexpr int32_t HOT_AREA_MARGIN { 200 };
}; // namespace

void MouseLocation::AddListener(const RegisterEventListenerEvent &event)
{

}

void MouseLocation::RemoveListener(const UnregisterEventListenerEvent &event)
{

}

void MouseLocation::OnPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{

}

void MouseLocation::OnMouseLocationMessage()
{

}

void MouseLocation::NotifySubscriber()
{
    
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
