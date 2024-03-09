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

#include "coordination_event_manager.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationEventManager"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CoordinationEventManager::CoordinationEventManager() {}
CoordinationEventManager::~CoordinationEventManager() {}

void CoordinationEventManager::AddCoordinationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    CHKPV(event);
    std::lock_guard<std::mutex> guard(lock_);
    if (event->type == EventType::LISTENER) {
        auto it = std::find_if(remoteCoordinationCallbacks_.begin(), remoteCoordinationCallbacks_.end(),
            [event] (auto info) {
                return (*info).sess == event->sess;
            });
        if (it != remoteCoordinationCallbacks_.end()) {
            *it = event;
        } else {
            remoteCoordinationCallbacks_.emplace_back(event);
        }
    } else {
        coordinationCallbacks_[event->type] = event;
    }
}

void CoordinationEventManager::RemoveCoordinationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    if (remoteCoordinationCallbacks_.empty() || event == nullptr) {
        FI_HILOGE("Remove listener failed");
        return;
    }
    for (auto it = remoteCoordinationCallbacks_.begin(); it != remoteCoordinationCallbacks_.end(); ++it) {
        if ((*it)->sess == event->sess) {
            remoteCoordinationCallbacks_.erase(it);
            return;
        }
    }
}

int32_t CoordinationEventManager::OnCoordinationMessage(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    if (remoteCoordinationCallbacks_.empty()) {
        FI_HILOGW("The current listener is empty, unable to invoke the listening interface");
        return RET_ERR;
    }
    for (auto it = remoteCoordinationCallbacks_.begin(); it != remoteCoordinationCallbacks_.end(); ++it) {
        sptr<EventInfo> info = *it;
        CHKPC(info);
        NotifyCoordinationMessage(info->sess, info->msgId, info->userData, networkId, msg);
    }
    return RET_OK;
}

void CoordinationEventManager::OnEnable(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::ENABLE];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, networkId, msg);
    coordinationCallbacks_[EventType::ENABLE] = nullptr;
}

void CoordinationEventManager::OnStart(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::START];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, networkId, msg);
    coordinationCallbacks_[EventType::START] = nullptr;
}

void CoordinationEventManager::OnStop(CoordinationMessage msg, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::STOP];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, networkId, msg);
    coordinationCallbacks_[EventType::STOP] = nullptr;
}

void CoordinationEventManager::OnGetCrossingSwitchState(bool state)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::STATE];
    CHKPV(info);
    NotifyCoordinationState(info->sess, info->msgId, info->userData, state);
    coordinationCallbacks_[EventType::STATE] = nullptr;
}

void CoordinationEventManager::OnErrorMessage(EventType type, CoordinationMessage msg)
{
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[type];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, "", msg);
    coordinationCallbacks_[type] = nullptr;
}

void CoordinationEventManager::SetIContext(IContext *context)
{
    context_ = context;
}

IContext* CoordinationEventManager::GetIContext() const
{
    return context_;
}

void CoordinationEventManager::NotifyCoordinationMessage(
    SessionPtr sess, MessageId msgId, int32_t userData, const std::string &networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess);
    NetPacket pkt(msgId);
    pkt << userData << networkId << static_cast<int32_t>(msg);
    FI_HILOGI("UserData:%{public}d, networkId:%{public}s, msg:%{public}d",
        userData, GetAnonyString(networkId).c_str(), static_cast<int32_t>(msg));
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}

void CoordinationEventManager::NotifyCoordinationState(SessionPtr sess, MessageId msgId, int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess);
    NetPacket pkt(msgId);
    pkt << userData << state;
    FI_HILOGI("UserData:%{public}d, state:%{public}s", userData, (state ? "true" : "false"));
    if (pkt.ChkRWError()) {
        FI_HILOGE("Coordination state, packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Coordination state, sending failed");
        return;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
