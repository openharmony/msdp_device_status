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

#ifndef COOPERATE_EVENT_MANAGER_H
#define COOPERATE_EVENT_MANAGER_H

#include <list>
#include <mutex>
#include <string>

#include "nocopyable.h"

#include "coordination_message.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class EventManager final {
public:
    enum EventType { LISTENER, ENABLE, START, STOP, STATE };
    struct EventInfo {
        EventType type { LISTENER };
        MessageId msgId { MessageId::INVALID };
        int32_t pid { -1 };
        int32_t userData { -1 };
        std::string networkId;
        CoordinationMessage msg { CoordinationMessage::PREPARE };
        bool state { false };
    };

    EventManager(IContext *env);
    ~EventManager() = default;
    DISALLOW_COPY_AND_MOVE(EventManager);

    void AddCooperateEvent(std::shared_ptr<EventInfo> event);
    void RemoveCooperateEvent(std::shared_ptr<EventInfo> event);
    int32_t OnCooperateMessage(CoordinationMessage msg, const std::string &networkId = "");
    void OnEnable(CoordinationMessage msg, const std::string &networkId = "");
    void OnStart(CoordinationMessage msg, const std::string &networkId = "");
    void OnStop(CoordinationMessage msg, const std::string &networkId = "");
    void OnGetCrossingSwitchState(bool state);
    void OnErrorMessage(EventType type, CoordinationMessage msg);

private:
    void NotifyCooperateMessage(int32_t pid, MessageId msgId, int32_t userData,
        const std::string &networkId, CoordinationMessage msg);
    void NotifyCooperateState(int32_t pid, MessageId msgId, int32_t userData, bool state);

private:
    IContext *env_ { nullptr };
    std::mutex lock_;
    std::list<std::shared_ptr<EventInfo>> listeners_;
    std::map<EventType, std::shared_ptr<EventInfo>> calls_ {
        { EventType::ENABLE, nullptr },
        { EventType::START, nullptr },
        { EventType::STOP, nullptr },
        { EventType::STATE, nullptr }
    };
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_EVENT_MANAGER_H