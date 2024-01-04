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

#include "event_manager.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "EventManager" };
} // namespace

EventManager::EventManager(IContext *env)
    : env_(env)
{}

void EventManager::StartCooperate(const StartCooperateEvent &event)
{
    std::shared_ptr<EventInfo> evInfo = std::make_shared<EventInfo>();
    evInfo->type = EventType::START;
    evInfo->msgId = MessageId::COORDINATION_MESSAGE;
    evInfo->pid = event.pid;
    evInfo->userData = event.userData;
    evInfo->networkId = event.remoteNetworkId;
    calls_[EventType::START] = evInfo;
}

void EventManager::StartCooperateFinish(const DSoftbusStartCooperateFinished &event)
{}

void EventManager::RemoteStart(const DSoftbusStartCooperate &event)
{}

void EventManager::RemoteStartFinish(const DSoftbusStartCooperateFinished &event)
{
    CoordinationMessage msg { event.success ?
                              CoordinationMessage::ACTIVATE_SUCCESS :
                              CoordinationMessage::ACTIVATE_FAIL };
    OnCooperateMessage(msg, event.networkId);
}

void EventManager::AddCooperateEvent(std::shared_ptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    CHKPV(event);
    std::lock_guard<std::mutex> guard(lock_);
    if (event->type == EventType::LISTENER) {
        auto it = std::find_if(listeners_.begin(), listeners_.end(),
            [event] (const auto &info) {
                return ((info != nullptr) && (info->pid == event->pid));
            });
        if (it != listeners_.end()) {
            *it = event;
        } else {
            listeners_.emplace_back(event);
        }
    } else {
        calls_[event->type] = event;
    }
}

void EventManager::RemoveCooperateEvent(std::shared_ptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    CHKPV(event);
    for (auto it = listeners_.begin(); it != listeners_.end(); ++it) {
        if ((*it)->pid == event->pid) {
            listeners_.erase(it);
            return;
        }
    }
}

int32_t EventManager::OnCooperateMessage(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    for (auto it = listeners_.begin(); it != listeners_.end(); ++it) {
        std::shared_ptr<EventInfo> listener = *it;
        CHKPC(listener);
        NotifyCooperateMessage(listener->pid, listener->msgId, listener->userData, networkId, msg);
    }
    return RET_OK;
}

void EventManager::OnEnable(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    std::shared_ptr<EventInfo> call = calls_[EventType::ENABLE];
    CHKPV(call);
    NotifyCooperateMessage(call->pid, call->msgId, call->userData, networkId, msg);
    calls_[EventType::ENABLE] = nullptr;
}

void EventManager::OnStart(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    std::shared_ptr<EventInfo> call = calls_[EventType::START];
    CHKPV(call);
    NotifyCooperateMessage(call->pid, call->msgId, call->userData, networkId, msg);
    calls_[EventType::START] = nullptr;
}

void EventManager::OnStop(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    std::shared_ptr<EventInfo> call = calls_[EventType::STOP];
    CHKPV(call);
    NotifyCooperateMessage(call->pid, call->msgId, call->userData, networkId, msg);
    calls_[EventType::STOP] = nullptr;
}

void EventManager::OnGetCrossingSwitchState(bool state)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    std::shared_ptr<EventInfo> call = calls_[EventType::STATE];
    CHKPV(call);
    NotifyCooperateState(call->pid, call->msgId, call->userData, state);
    calls_[EventType::STATE] = nullptr;
}

void EventManager::OnErrorMessage(EventType type, CoordinationMessage msg)
{
    std::lock_guard<std::mutex> guard(lock_);
    std::shared_ptr<EventInfo> call = calls_[type];
    CHKPV(call);
    NotifyCooperateMessage(call->pid, call->msgId, call->userData, "", msg);
    calls_[type] = nullptr;
}

void EventManager::NotifyCooperateMessage(int32_t pid, MessageId msgId, int32_t userData,
    const std::string &networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHKPV(env_);
    auto session = env_->GetSocketSessionManager().FindSessionByPid(pid);
    CHKPV(session);
    NetPacket pkt(msgId);
    pkt << userData << networkId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!session->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
    }
}

void EventManager::NotifyCooperateState(int32_t pid, MessageId msgId, int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHKPV(env_);
    auto session = env_->GetSocketSessionManager().FindSessionByPid(pid);
    CHKPV(session);
    NetPacket pkt(msgId);
    pkt << userData << state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!session->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
