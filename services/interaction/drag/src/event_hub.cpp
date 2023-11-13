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

#include "event_hub.h"

#include <map>

#include "want.h"

#include "drag_manager.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "EventHub" };
static std::map<std::string, EventId> g_actionMap = {
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON, EventId::EVENT_SCREEN_ON },
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF, EventId::EVENT_SCREEN_OFF },
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_LOCKED, EventId::EVENT_SCREEN_LOCK },
    { EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_UNLOCKED, EventId::EVENT_SCREEN_UNLOCK },
    { EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_LOW, EventId::EVENT_BATTERY_LOW },
    { EventFwk::CommonEventSupport::COMMON_EVENT_BATTERY_OKAY, EventId::EVENT_BATTERY_OKAY },
};

std::shared_ptr<EventHub> EventHub::GetEventHub(IContext* context)
{
    CALL_DEBUG_ENTER;
    auto skill = std::make_shared<EventFwk::MatchingSkills>();
    for (auto &actionPair : g_actionMap) {
        skill->AddEvent(actionPair.first);
    }
    auto info = std::make_shared<EventFwk::CommonEventSubscribeInfo>(*skill);
    auto eventHub = std::make_shared<EventHub>(*info, context);
    return eventHub;
}

void EventHub::RegisterEvent(std::shared_ptr<EventHub> eventHub)
{
    CALL_DEBUG_ENTER;
    bool result = EventFwk::CommonEventManager::SubscribeCommonEvent(eventHub);
    if (result != true) {
        FI_HILOGE("Failed to subscribe common event");
    }
}

void EventHub::UnRegisterEvent(std::shared_ptr<EventHub> eventHub)
{
    CALL_DEBUG_ENTER;
    bool result = EventFwk::CommonEventManager::UnSubscribeCommonEvent(eventHub);
    if (result != true) {
        FI_HILOGE("Failed to unSubscribe common event");
    }
}

void EventHub::OnReceiveEvent(const EventFwk::CommonEventData &event)
{
    const auto want = event.GetWant();
    const auto action = want.GetAction();
    if (g_actionMap.find(action) == g_actionMap.end()) {
        return;
    }
    EventId eventId = g_actionMap[action];
    FI_HILOGD("Receive action:%{public}s, eventId:%{public}d", action.c_str(), static_cast<int32_t>(eventId));
    if (eventId != EventId::EVENT_SCREEN_LOCK) {
        return;
    }
    CHKPV(context_);
    auto fun = [] (IContext* context) -> int32_t {
        if (context->GetDragManager().GetDragState() == DragState::START) {
            DragDropResult dropResult { DragResult::DRAG_CANCEL, false, -1 };
            context->GetDragManager().StopDrag(dropResult);
        }
        return RET_OK;
    };
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(std::bind(fun, context_));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
}
}  // namespace DeviceStatus
}  // namespace Msdp
}  // namespace OHOS