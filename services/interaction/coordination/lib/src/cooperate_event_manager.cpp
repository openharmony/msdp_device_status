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
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CooperateEventManager" };
} // namespace

CooperateEventManager::CooperateEventManager() {}
CooperateEventManager::~CooperateEventManager() {}

void CooperateEventManager::AddCoordinationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    if (event->type == EventType::LISTENER) {
        remoteCoordinationCallbacks_.emplace_back(event);
    } else {
        coordinationCallbacks_[event->type] = event;
    }
}

void CooperateEventManager::RemoveCoordinationEvent(sptr<EventInfo> event)
{
    CALL_DEBUG_ENTER;
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

int32_t CooperateEventManager::OnCoordinationMessage(CoordinationMessage msg, const std::string &deviceId)
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
        NotifyCoordinationMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    }
    return RET_OK;
}

void CooperateEventManager::OnEnable(CoordinationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::ENABLE];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    coordinationCallbacks_[EventType::ENABLE] =  nullptr;
}

void CooperateEventManager::OnStart(CoordinationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::START];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    coordinationCallbacks_[EventType::START] =  nullptr;
}

void CooperateEventManager::OnStop(CoordinationMessage msg, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::STOP];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, deviceId, msg);
    coordinationCallbacks_[EventType::STOP] =  nullptr;
}

void CooperateEventManager::OnGetState(bool state)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[EventType::STATE];
    CHKPV(info);
    NotifyCoordinationState(info->sess, info->msgId, info->userData, state);
    coordinationCallbacks_[EventType::STATE] =  nullptr;
}

void CooperateEventManager::OnErrorMessage(EventType type, CoordinationMessage msg)
{
    std::lock_guard<std::mutex> guard(lock_);
    sptr<EventInfo> info = coordinationCallbacks_[type];
    CHKPV(info);
    NotifyCoordinationMessage(info->sess, info->msgId, info->userData, "", msg);
    coordinationCallbacks_[type] =  nullptr;
}

void CooperateEventManager::SetIContext(IContext *context)
{
    context_ = context;
}

IContext* CooperateEventManager::GetIContext() const
{
    return context_;
}

void CooperateEventManager::NotifyCoordinationMessage(
    SessionPtr sess, MessageId msgId, int32_t userData, const std::string &deviceId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess);
    NetPacket pkt(msgId);
    pkt << userData << deviceId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}

void CooperateEventManager::NotifyCoordinationState(SessionPtr sess, MessageId msgId, int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess);
    NetPacket pkt(msgId);
    pkt << userData << state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
