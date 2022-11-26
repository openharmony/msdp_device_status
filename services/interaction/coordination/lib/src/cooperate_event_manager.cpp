/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "cooperate_event_manager.h"
#include "devicestatus_hilog_wrapper.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
} // namespace

CooperateEventManager::CooperateEventManager() {}
CooperateEventManager::~CooperateEventManager() {}

void CooperateEventManager::AddCooperationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    if (event->type == EventType::LISTENER) {
        remoteCooperateCallbacks_.emplace_back(event);
    } else {
        cooperateCallbacks_[event->type] = event;
    }
}

void CooperateEventManager::RemoveCooperationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    if (remoteCooperateCallbacks_.empty() || event == nullptr) {
        DEV_HILOGE(SERVICE, "Remove listener failed");
        return;
    }
    for (auto it = remoteCooperateCallbacks_.begin(); it != remoteCooperateCallbacks_.end(); ++it) {
        if ((*it)->sess == event->sess) {
            remoteCooperateCallbacks_.erase(it);
            return;
        }
    }
}

int32_t CooperateEventManager::OnCooperateMessage(CooperationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    if (remoteCooperateCallbacks_.empty()) {
        DEV_HILOGE(SERVICE, "No listener, send cooperate message failed");
        return RET_ERR;
    }
    for (auto it = remoteCooperateCallbacks_.begin(); it != remoteCooperateCallbacks_.end(); ++it) {
        sptr<EventInfo> info = *it;
        CHKPC(info, SERVICE);
        NotifyCooperateMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    }
    return RET_OK;
}

void CooperateEventManager::OnEnable(CooperationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = cooperateCallbacks_[EventType::ENABLE];
    CHKPV(info, SERVICE);
    NotifyCooperateMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    cooperateCallbacks_[EventType::ENABLE] =  nullptr;
}

void CooperateEventManager::OnStart(CooperationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = cooperateCallbacks_[EventType::START];
    CHKPV(info, SERVICE);
    NotifyCooperateMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    cooperateCallbacks_[EventType::START] =  nullptr;
}

void CooperateEventManager::OnStop(CooperationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = cooperateCallbacks_[EventType::STOP];
    CHKPV(info, SERVICE);
    NotifyCooperateMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    cooperateCallbacks_[EventType::STOP] =  nullptr;
}

void CooperateEventManager::OnGetState(bool state)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = cooperateCallbacks_[EventType::STATE];
    CHKPV(info, SERVICE);
    NotifyCooperateState(info->sess, info->msgId, info->userData, state);
    cooperateCallbacks_[EventType::STATE] =  nullptr;
}

void CooperateEventManager::OnErrorMessage(EventType type, CooperationMessage msg)
{
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = cooperateCallbacks_[type];
    CHKPV(info, SERVICE);
    NotifyCooperateMessage(info->sess, info->msgId, info->userData, "", msg);
    cooperateCallbacks_[type] =  nullptr;
}

void CooperateEventManager::SetIInputContext(IInputContext* context)
{
    context_ = context;
}

IInputContext* CooperateEventManager::GetIInputContext() const
{
    return context_;
}

void CooperateEventManager::NotifyCooperateMessage(
    SessionPtr sess, MmiMessageId msgId, int32_t userData, const std::string &deviceId, CooperationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess, SERVICE);
    NetPacket pkt(msgId);
    pkt << userData << deviceId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        DEV_HILOGE(SERVICE, "Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        DEV_HILOGE(SERVICE, "Sending failed");
        return;
    }
}

void CooperateEventManager::NotifyCooperateState(SessionPtr sess, MmiMessageId msgId, int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess, SERVICE);
    NetPacket pkt(msgId);
    pkt << userData << state;
    if (pkt.ChkRWError()) {
        DEV_HILOGE(SERVICE, "Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        DEV_HILOGE(SERVICE, "Sending failed");
        return;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
