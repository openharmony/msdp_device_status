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

#include "coordination.h"

#include "coordination_event_manager.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "distributed_input_adapter.h"
#include "proto.h"

#undef LOG_TAG
#define LOG_TAG "Coordination"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

void Coordination::PrepareCoordination()
{
    COOR_SM->PrepareCoordination();
}

void Coordination::UnprepareCoordination()
{
    COOR_SM->UnprepareCoordination();
}

int32_t Coordination::ActivateCoordination(SessionPtr sess, int32_t userData,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->ActivateCoordination(remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("ActivateCoordination failed, ret:%{public}d", ret);
        COOR_EVENT_MGR->OnErrorMessage(event->type, static_cast<CoordinationMessage>(ret));
    }
    return ret;
}

int32_t Coordination::DeactivateCoordination(SessionPtr sess, int32_t userData, bool isUnchained)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->DeactivateCoordination(isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate coordination failed, ret:%{public}d", ret);
        COOR_EVENT_MGR->OnErrorMessage(event->type, static_cast<CoordinationMessage>(ret));
    }
    return ret;
}

int32_t Coordination::GetCoordinationState(SessionPtr sess, int32_t userData, const std::string &networkId)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->GetCoordinationState(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to get coordination state");
    }
    return ret;
}

int32_t Coordination::RegisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    return RET_OK;
}

int32_t Coordination::UnregisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    COOR_EVENT_MGR->RemoveCoordinationEvent(event);
    return RET_OK;
}

void Coordination::Dump(int32_t fd)
{
    COOR_SM->Dump(fd);
}

ICoordination* CreateICoordination(IContext *context)
{
    CHKPP(context);
    COOR_EVENT_MGR->SetIContext(context);
    ICoordination *coord = new (std::nothrow) Coordination();
    CHKPP(coord);
    return coord;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS