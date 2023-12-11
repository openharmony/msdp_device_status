/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef COORDINATION_EVENT_MANAGER_H
#define COORDINATION_EVENT_MANAGER_H

#include <list>
#include <mutex>
#include <string>

#include "nocopyable.h"
#include "refbase.h"
#include "singleton.h"
#include "stream_session.h"

#include "coordination_message.h"
#include "fi_log.h"
#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationEventManager final {
    DECLARE_DELAYED_SINGLETON(CoordinationEventManager);
public:
    enum EventType { LISTENER, ENABLE, START, STOP, STATE };
    struct EventInfo : public RefBase {
        EventType type { LISTENER };
        SessionPtr sess { nullptr };
        MessageId msgId { MessageId::INVALID };
        int32_t userData { -1 };
        std::string networkId;
        CoordinationMessage msg { CoordinationMessage::PREPARE };
        bool state { false };
    };

    DISALLOW_COPY_AND_MOVE(CoordinationEventManager);

    void AddCoordinationEvent(sptr<EventInfo> event);
    void RemoveCoordinationEvent(sptr<EventInfo> event);
    int32_t OnCoordinationMessage(CoordinationMessage msg, const std::string &networkId = "");
    void OnEnable(CoordinationMessage msg, const std::string &networkId = "");
    void OnStart(CoordinationMessage msg, const std::string &networkId = "");
    void OnStop(CoordinationMessage msg, const std::string &networkId = "");
    void OnGetCrossingSwitchState(bool state);
    void OnErrorMessage(EventType type, CoordinationMessage msg);
    void SetIContext(IContext *context);
    IContext* GetIContext() const;

private:
    void NotifyCoordinationMessage(SessionPtr sess, MessageId msgId, int32_t userData,
        const std::string &networkId, CoordinationMessage msg);
    void NotifyCoordinationState(SessionPtr sess, MessageId msgId, int32_t userData, bool state);

private:
    std::mutex lock_;
    std::list<sptr<EventInfo>> remoteCoordinationCallbacks_;
    std::map<EventType, sptr<EventInfo>> coordinationCallbacks_{
        { EventType::ENABLE, nullptr },
        { EventType::START, nullptr },
        { EventType::STOP, nullptr },
        { EventType::STATE, nullptr }
    };
    IContext *context_ { nullptr };
};

#define COOR_EVENT_MGR OHOS::DelayedSingleton<CoordinationEventManager>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_EVENT_MANAGER_H
