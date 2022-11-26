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

#include "cooperate_event_manager.h"
#include "distributed_input_adapter.h"
#include "input_device_cooperate_sm.h"
#include "input_device_cooperate_util.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
} // namespace

DInput::DInput()
{
}

DInput::~DInput()
{
}

void DInput::Init(DelegateTasksCallback delegateTasksCallback)
{
    InputDevCooSM->Init(delegateTasksCallback);
}

void DInput::RegisterEventCallback(SimulateEventCallback callback)
{
    DistributedAdapter->RegisterEventCallback(callback);
}

void DInput::EnableInputDeviceCooperate(bool enabled)
{
    InputDevCooSM->EnableInputDeviceCooperate(enabled);
}

int32_t DInput::OnStartInputDeviceCooperate(SessionPtr sess, int32_t userData,
    const std::string& sinkDeviceId, int32_t srcInputDeviceId)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
	CHKPR(event, SERVICE, RET_ERR);
	event->type = CooperateEventManager::EventType::START;
	event->sess = sess;
	event->msgId = MmiMessageId::COOPERATION_MESSAGE;
	event->userData = userData;
	CooperateEventMgr->AddCooperationEvent(event);
	int32_t ret = InputDevCooSM->StartInputDeviceCooperate(sinkDeviceId, srcInputDeviceId);
	if (ret != RET_OK) {
		DEV_HILOGE(SERVICE, "OnStartInputDeviceCooperate failed, ret:%{public}d", ret);
		CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
		return ret;
	}
	return RET_OK;
}

int32_t DInput::OnStopDeviceCooperate(SessionPtr sess, int32_t userData)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
	CHKPR(event, SERVICE, RET_ERR);
	event->type = CooperateEventManager::EventType::STOP;
	event->sess = sess;
	event->msgId = MmiMessageId::COOPERATION_MESSAGE;
	event->userData = userData;
	CooperateEventMgr->AddCooperationEvent(event);
	int32_t ret = InputDevCooSM->StopInputDeviceCooperate();
	if (ret != RET_OK) {
		DEV_HILOGE(SERVICE, "OnStopDeviceCooperate failed, ret:%{public}d", ret);
		CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
		return ret;
	}
	return RET_OK;
}

int32_t DInput::OnGetInputDeviceCooperateState(SessionPtr sess, int32_t userData,
    const std::string& deviceId)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
	CHKPR(event, SERVICE, RET_ERR);
	event->type = CooperateEventManager::EventType::STATE;
	event->sess = sess;
	event->msgId = MmiMessageId::COOPERATION_GET_STATE;
	event->userData = userData;
	CooperateEventMgr->AddCooperationEvent(event);
	InputDevCooSM->GetCooperateState(deviceId);
	return RET_OK;
}

int32_t DInput::OnRegisterCooperateListener(SessionPtr sess)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
	CHKPR(event, SERVICE, RET_ERR);
	event->type = CooperateEventManager::EventType::LISTENER;
	event->sess = sess;
	event->msgId = MmiMessageId::COOPERATION_ADD_LISTENER;
	CooperateEventMgr->AddCooperationEvent(event);
	return RET_OK;
}

int32_t DInput::OnUnregisterCooperateListener(SessionPtr sess)
{
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
	CHKPR(event, SERVICE, RET_ERR);
	event->type = CooperateEventManager::EventType::LISTENER;
	event->sess = sess;
	CooperateEventMgr->RemoveCooperationEvent(event);
	return RET_OK;
}

void DInput::OnKeyboardOnline(const std::string& dhid)
{
    InputDevCooSM->OnKeyboardOnline(dhid);
}

void DInput::OnPointerOffline(const std::string& dhid, const std::string& sinkNetworkId,
    const std::vector<std::string>& keyboards)
{
    InputDevCooSM->OnPointerOffline(dhid, sinkNetworkId, keyboards);
}

bool DInput::HandleEvent(libinput_event* event)
{
    return InputDevCooSM->HandleEvent(event);
}

bool DInput::CheckKeyboardWhiteList(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
	auto* context = CooperateEventMgr->GetIInputContext();
    CHKPF(context, SERVICE);
    CooperateState state = InputDevCooSM->GetCurrentCooperateState();
	DEV_HILOGI(SERVICE, "Get current cooperate state:%{public}d", state);
	if (state == CooperateState::STATE_IN) {
		int32_t deviceId = keyEvent->GetDeviceId();
		if (context->IsRemote(deviceId)) {
			auto networkId = context->GetOriginNetworkId(deviceId);
			return !IsNeedFilterOut(networkId, keyEvent);
		}
	} else if (state == CooperateState::STATE_OUT) {
		std::string networkId = COOPERATE::GetLocalDeviceId();
		if (!IsNeedFilterOut(networkId, keyEvent)) {
			if (keyEvent->GetKeyAction() == MMI::KeyEvent::KEY_ACTION_UP) {
				context->SelectAutoRepeat(keyEvent);
			}
			return false;
		}
		context->SetJumpInterceptState(true);
	} else {
		DEV_HILOGW(SERVICE, "Get current cooperate state:STATE_FREE(%{public}d)", state);
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
	DEV_HILOGI(SERVICE, "businessEvent.keyCode :%{public}d, keyAction :%{public}d",
		businessEvent.keyCode, businessEvent.keyAction);
	for (const auto& item : businessEvent.pressedKeys) {
		DEV_HILOGI(SERVICE, "pressedKeys :%{public}d", item);
	}
	return DistributedAdapter->IsNeedFilterOut(deviceId, businessEvent);
}

std::string DInput::GetLocalDeviceId()
{
	return COOPERATE::GetLocalDeviceId();
}

void DInput::Dump(int32_t fd, const std::vector<std::string>& args)
{
    InputDevCooSM->Dump(fd, args);
}

IDInput* CreateIDInpt(IInputContext* context)
{
    if (context == nullptr) {
		DEV_HILOGE(SERVICE, "Parameter error");
        return nullptr;
    }
    CooperateEventMgr->SetIInputContext(context);
    IDInput* input = new (std::nothrow) DInput();
	if (input == nullptr) {
		DEV_HILOGE(SERVICE, "Create IDInpt failed");
		return nullptr;
	}
	return input;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS