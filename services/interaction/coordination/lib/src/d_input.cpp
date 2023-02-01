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

#include "d_input.h"

#include "coordination_event_manager.h"
#include "distributed_input_adapter.h"
#include "coordination_sm.h"
#include "coordination_util.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DInput" };
} // namespace

DInput::DInput()
{
}

DInput::~DInput()
{
}

void DInput::Init()
{
    CooSM->Init();
}

void DInput::RegisterEventCallback(SimulateEventCallback callback)
{
    DistributedAdapter->RegisterEventCallback(callback);
}

void DInput::EnableCoordination(bool enabled)
{
    CooSM->EnableCoordination(enabled);
}

int32_t DInput::OnStartCoordination(SessionPtr sess, int32_t userData,
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
        FI_HILOGE("OnStartCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DInput::OnStopCoordination(SessionPtr sess, int32_t userData)
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
        FI_HILOGE("OnStopCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DInput::OnGetCoordinationState(SessionPtr sess, int32_t userData,
    const std::string& deviceId)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    CooSM->GetCoordinationState(deviceId);
    return RET_OK;
}

int32_t DInput::OnRegisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    CoordinationEventMgr->AddCoordinationEvent(event);
    return RET_OK;
}

int32_t DInput::OnUnregisterCoordinationListener(SessionPtr sess)
{
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    CoordinationEventMgr->RemoveCoordinationEvent(event);
    return RET_OK;
}

void DInput::OnKeyboardOnline(const std::string& dhid)
{
    CooSM->OnKeyboardOnline(dhid);
}

void DInput::OnPointerOffline(const std::string& dhid, const std::string& sinkNetworkId,
    const std::vector<std::string>& keyboards)
{
    CooSM->OnPointerOffline(dhid, sinkNetworkId, keyboards);
}

bool DInput::CheckKeyboardWhiteList(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPF(context);
    IDeviceManager &devMgr = context->GetDeviceManager();
    CoordinationState state = CooSM->GetCurrentCoordinationState();
    FI_HILOGI("Get current coordination state:%{public}d", state);

    if (state == CoordinationState::STATE_IN) {
        int32_t deviceId = keyEvent->GetDeviceId();
        if (devMgr.IsRemote(deviceId)) {
            auto networkId = devMgr.GetOriginNetworkId(deviceId);
            return !IsNeedFilterOut(networkId, keyEvent);
        }
    } else if (state == CoordinationState::STATE_OUT) {
        std::string networkId = COORDINATION::GetLocalDeviceId();
        if (!IsNeedFilterOut(networkId, keyEvent)) {
            if (keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_UP) {
                context->SelectAutoRepeat(keyEvent);
            }
            return false;
        }
        context->SetJumpInterceptState(true);
    } else {
        FI_HILOGW("Get current coordination state:STATE_FREE(%{public}d)", state);
    }
    return true;
}

bool DInput::IsNeedFilterOut(const std::string& deviceId, const std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CALL_DEBUG_ENTER;
    std::vector<OHOS::MMI::KeyEvent::KeyItem> KeyItems = keyEvent->GetKeyItems();
    std::vector<int32_t> KeyItemsForDInput;
    KeyItemsForDInput.reserve(KeyItems.size());
    for (const auto& item : KeyItems) {
        KeyItemsForDInput.push_back(item.GetKeyCode());
    }
    OHOS::DistributedHardware::DistributedInput::BusinessEvent businessEvent;
    businessEvent.keyCode = keyEvent->GetKeyCode();
    businessEvent.keyAction = keyEvent->GetKeyAction();
    businessEvent.pressedKeys = KeyItemsForDInput;
    FI_HILOGI("businessEvent.keyCode :%{public}d, keyAction :%{public}d",
        businessEvent.keyCode, businessEvent.keyAction);
    for (const auto& item : businessEvent.pressedKeys) {
        FI_HILOGI("pressedKeys :%{public}d", item);
    }
    return DistributedAdapter->IsNeedFilterOut(deviceId, businessEvent);
}

std::string DInput::GetLocalDeviceId()
{
    return COORDINATION::GetLocalDeviceId();
}

void DInput::Dump(int32_t fd)
{
    CooSM->Dump(fd);
}

IDInput* CreateIDInpt(IContext *context)
{
    if (context == nullptr) {
        FI_HILOGE("Parameter error");
        return nullptr;
    }
    CoordinationEventMgr->SetIContext(context);
    IDInput* input = new (std::nothrow) DInput();
    if (input == nullptr) {
        FI_HILOGE("Create IDInpt failed");
        return nullptr;
    }
    return input;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS