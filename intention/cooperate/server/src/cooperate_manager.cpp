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

#include "cooperate_manager.h"

#include "cooperate_event_manager.h"
#include "cooperate_sm.h"
#include "cooperate_util.h"
#include "distributed_input_adapter.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateManager" };
} // namespace

void CooperateManager::PrepareCooperate()
{
    COOR_SM->PrepareCooperate();
}

void CooperateManager::UnprepareCooperate()
{
    COOR_SM->UnprepareCooperate();
}

int32_t CooperateManager::ActivateCooperate(SessionPtr sess, int32_t userData,
    const std::string& remoteNetworkId, int32_t startDeviceId)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCooperateEvent(event);
    int32_t ret = COOR_SM->ActivateCooperate(remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("ActivateCooperate failed, ret:%{public}d", ret);
        COOR_EVENT_MGR->OnErrorMessage(event->type, static_cast<CooperateMessage>(ret));
    }
    return ret;
}

int32_t CooperateManager::DeactivateCooperate(SessionPtr sess, int32_t userData, bool isUnchained)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCooperateEvent(event);
    int32_t ret = COOR_SM->DeactivateCooperate(isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate cooperate manager failed, ret:%{public}d", ret);
        COOR_EVENT_MGR->OnErrorMessage(event->type, static_cast<CooperateMessage>(ret));
    }
    return ret;
}

int32_t CooperateManager::GetCooperateState(SessionPtr sess, int32_t userData, const std::string &deviceId)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCooperateEvent(event);
    int32_t ret = COOR_SM->GetCooperateState(deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Get cooperate manager state failed");
    }
    return ret;
}

int32_t CooperateManager::RegisterCooperateListener(SessionPtr sess)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    COOR_EVENT_MGR->AddCooperateEvent(event);
    return RET_OK;
}

int32_t CooperateManager::UnregisterCooperateListener(SessionPtr sess)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    COOR_EVENT_MGR->RemoveCooperateEvent(event);
    return RET_OK;
}

void CooperateManager::Dump(int32_t fd)
{
    COOR_SM->Dump(fd);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS