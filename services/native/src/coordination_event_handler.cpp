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
#include "coordination_event_handler.h"

#include "cooperate_event_manager.h"
#include "devicestatus_define.h"
#include "input_device_cooperate_sm.h"
#include "uds_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationEventHandler" };
} // namespace
void CoordinationEventHandler::Init(UDSServer& udsServer)
{
    udsServer_ = &udsServer;
}

int32_t CoordinationEventHandler::OnRegisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_ADD_LISTENER;
    CooperateEventMgr->AddCooperationEvent(event);
    return RET_OK;
}

int32_t CoordinationEventHandler::OnUnregisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    CooperateEventMgr->RemoveCooperationEvent(event);
    return RET_OK;
}

int32_t CoordinationEventHandler::OnEnableInputDeviceCoordination(int32_t pid, int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    InputDevCooSM->EnableInputDeviceCooperate(enabled);
    std::string deviceId =  "";
    CooperationMessage msg =
        enabled ? CooperationMessage::OPEN_SUCCESS : CooperationMessage::CLOSE_SUCCESS;
    NetPacket pkt(MmiMessageId::COOPERATION_MESSAGE);
    pkt << userData << deviceId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}

int32_t CoordinationEventHandler::OnStartInputDeviceCoordination(int32_t pid,
    int32_t userData, const std::string &sinkDeviceId, int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_MESSAGE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    int32_t ret = InputDevCooSM->StartInputDeviceCooperate(sinkDeviceId, srcInputDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("OnStartInputDeviceCoordination failed, ret:%{public}d", ret);
        CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationEventHandler::OnStopInputDeviceCoordination(int32_t pid, int32_t userData)
{
    CALL_DEBUG_ENTER;
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_MESSAGE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    int32_t ret = InputDevCooSM->StopInputDeviceCooperate();
    if (ret != RET_OK) {
        FI_HILOGE("OnStopInputDeviceCoordination failed, ret:%{public}d", ret);
        CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationEventHandler::OnGetInputDeviceCoordinationState(
    int32_t pid, int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = udsServer_->GetSession(udsServer_->GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_GET_STATE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    InputDevCooSM->GetCooperateState(deviceId);
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS