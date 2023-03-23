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
#include "distributed_input_adapter.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Coordination" };
} // namespace

void Coordination::EnableCoordination(bool enabled)
{
    CooSM->EnableCoordination(enabled);
}

int32_t Coordination::StartCoordination(SessionPtr sess, int32_t userData,
    const std::string& sinkDeviceId, int32_t srcDeviceId)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    int32_t ret = CooSM->StartCoordination(sinkDeviceId, srcDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("StartCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t Coordination::StopCoordination(SessionPtr sess, int32_t userData)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    int32_t ret = CooSM->StopCoordination();
    if (ret != RET_OK) {
        FI_HILOGE("StopCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t Coordination::GetCoordinationState(SessionPtr sess, int32_t userData, const std::string &deviceId)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    int32_t ret = CooSM->GetCoordinationState(deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("GetCoordinationState faild");
        return ret;
    }
    return RET_OK;
}

int32_t Coordination::RegisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    CoordinationEventMgr->AddCoordinationEvent(event);
    return RET_OK;
}

int32_t Coordination::UnregisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    CoordinationEventMgr->RemoveCoordinationEvent(event);
    return RET_OK;
}

void Coordination::Dump(int32_t fd)
{
    CooSM->Dump(fd);
}

ICoordination* CreateICoordination(IContext *context)
{
    if (context == nullptr) {
        FI_HILOGE("Parameter error");
        return nullptr;
    }
    CoordinationEventMgr->SetIContext(context);
    ICoordination *coord = new (std::nothrow) Coordination();
    if (coord == nullptr) {
        FI_HILOGE("Create ICoordination failed");
        return nullptr;
    }
    return coord;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS