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
#include "input_device_cooperate_sm.h"

#include <cstdio>
#include <unistd.h>

#include "device_manager.h"
#include "display_manager.h"
#include "hitrace_meter.h"
#include "input_manager.h"

#include "cooperate_event_manager.h"
#include "coordination_message.h"
#include "device_cooperate_softbus_adapter.h"
#include "device_profile_adapter.h"
#include "devicestatus_define.h"
#include "display_info.h"
#include "input_device_cooperate_state_free.h"
#include "input_device_cooperate_state_in.h"
#include "input_device_cooperate_state_out.h"
#include "input_device_cooperate_util.h"
#include "input_manager.h"
#include "stub.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InputDeviceCooperateSM" };
constexpr int32_t INTERVAL_MS = 2000;
constexpr double PERCENT_CONST = 100.0;
constexpr int32_t MOUSE_ABS_LOCATION = 100;
constexpr int32_t MOUSE_ABS_LOCATION_X = 50;
constexpr int32_t MOUSE_ABS_LOCATION_Y = 50;
constexpr int32_t COORDINATION_PRIORITY = 499;
constexpr int32_t MIN_HANDLER_ID = 1;
} // namespace

InputDeviceCooperateSM::InputDeviceCooperateSM() {}
InputDeviceCooperateSM::~InputDeviceCooperateSM()
{
    RemoveMonitor();
}

void InputDeviceCooperateSM::Init()
{
    preparedNetworkId_ = std::make_pair("", "");
    currentStateSM_ = std::make_shared<InputDeviceCooperateStateFree>();
    DevCooperateSoftbusAdapter->Init();
    auto* context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    context->GetTimerManager().AddTimer(INTERVAL_MS, 1, [this]() {
        this->InitDeviceManager();
    });
    devObserver_ = std::make_shared<DeviceObserver>();
    context->GetDeviceManager().AddDeviceObserver(devObserver_);
    auto monitor = std::make_shared<MonitorConsumer>();
    monitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor); 
}

void InputDeviceCooperateSM::Reset(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    bool needReset = true;
    if (cooperateState_ == CooperateState::STATE_OUT) {
        if (networkId != srcNetworkId_) {
            needReset = false;
        }
    }
    if (cooperateState_ == CooperateState::STATE_IN) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        std::string sinkNetwoekId = context->GetDeviceManager().GetOriginNetworkId(startDhid_);
        if (networkId != sinkNetwoekId) {
            needReset = false;
        }
    }
    if (needReset) {
        preparedNetworkId_ = std::make_pair("", "");
        Reset(true);
    }
}

void InputDeviceCooperateSM::Reset(bool adjustAbsolutionLocation)
{
    CALL_INFO_TRACE;
    startDhid_ = "";
    srcNetworkId_ = "";
    currentStateSM_ = std::make_shared<InputDeviceCooperateStateFree>();
    cooperateState_ = CooperateState::STATE_FREE;
    auto* context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    bool hasPointer = context->GetDeviceManager().HasLocalPointerDevice();
    if (hasPointer && adjustAbsolutionLocation) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
    } else {
        InputMgr->SetPointerVisible(hasPointer);
    }
    isStarting_ = false;
    isStopping_ = false;
    RemoveInterceptor();
}

void InputDeviceCooperateSM::OnCooperateChanged(const std::string &networkId, bool isOpen)
{
    CALL_DEBUG_ENTER;
    CooperationMessage msg = isOpen ? CooperationMessage::STATE_ON : CooperationMessage::STATE_OFF;
    auto *context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, CooperateEventMgr, msg, networkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    if (!isOpen) {
        OnCloseCooperation(networkId, false);
    }
}

void InputDeviceCooperateSM::OnCloseCooperation(const std::string &networkId, bool isLocal)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!preparedNetworkId_.first.empty() && !preparedNetworkId_.second.empty()) {
        if (networkId == preparedNetworkId_.first || networkId == preparedNetworkId_.second) {
            if (cooperateState_ != CooperateState::STATE_FREE) {
                auto* context = CooperateEventMgr->GetIContext();
                CHKPV(context);
                auto dhids = context->GetDeviceManager().GetCooperateDhids(startDhid_);
                DistributedAdapter->StopRemoteInput(preparedNetworkId_.first, preparedNetworkId_.second,
                    dhids, [](bool isSuccess) {
                    FI_HILOGI("Failed to stop remote");
                });
            }
            DistributedAdapter->UnPrepareRemoteInput(preparedNetworkId_.first, preparedNetworkId_.second,
                [](bool isSuccess) {});
        }
    }
    preparedNetworkId_ = std::make_pair("", "");
    if (cooperateState_ == CooperateState::STATE_FREE) {
        return;
    }
    if (isLocal || networkId == srcNetworkId_) {
        Reset(true);
        return;
    }
    auto* context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    std::string originNetworkId = context->GetDeviceManager().GetOriginNetworkId(startDhid_);
    if (originNetworkId == networkId) {
        Reset();
    }
}

void InputDeviceCooperateSM::GetCooperateState(const std::string &deviceId)
{
    CALL_INFO_TRACE;
    bool state = DProfileAdapter->GetCrossingSwitchState(deviceId);
    CooperateEventMgr->OnGetState(state);
}

void InputDeviceCooperateSM::EnableInputDeviceCooperate(bool enabled)
{
    CALL_INFO_TRACE;
    if (enabled) {
        DProfileAdapter->UpdateCrossingSwitchState(enabled, onlineDevice_);
    } else {
        DProfileAdapter->UpdateCrossingSwitchState(enabled, onlineDevice_);
        std::string localNetworkId = COOPERATE::GetLocalDeviceId();
        OnCloseCooperation(localNetworkId, true);
    }
}

int32_t InputDeviceCooperateSM::StartInputDeviceCooperate(
    const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStarting_) {
        FI_HILOGE("In transition state, not process");
        return static_cast<int32_t>(CooperationMessage::COOPERATE_FAIL);
    }
    CHKPR(currentStateSM_, ERROR_NULL_POINTER);
    isStarting_ = true;
    DevCooperateSoftbusAdapter->OpenInputSoftbus(remoteNetworkId);
    int32_t ret = currentStateSM_->StartInputDeviceCooperate(remoteNetworkId, startInputDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Start remote input fail");
        isStarting_ = false;
        return ret;
    }
    UpdateMouseLocation();
    if (cooperateState_ == CooperateState::STATE_FREE) {
        srcNetworkId_ = remoteNetworkId;
    }
    return ret;
}

int32_t InputDeviceCooperateSM::StopInputDeviceCooperate()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStopping_) {
        FI_HILOGE("In transition state, not process");
        return RET_ERR;
    }
    CHKPR(currentStateSM_, ERROR_NULL_POINTER);
    isStopping_ = true;
    std::string stopNetworkId = "";
    if (cooperateState_ == CooperateState::STATE_IN) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPR(context, ERROR_NULL_POINTER);
        stopNetworkId = context->GetDeviceManager().GetOriginNetworkId(startDhid_);
    }
    if (cooperateState_ == CooperateState::STATE_OUT) {
        stopNetworkId = srcNetworkId_;
    }
    int32_t ret = currentStateSM_->StopInputDeviceCooperate(stopNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop input device cooperate fail");
        isStopping_ = false;
    }
    return ret;
}

void InputDeviceCooperateSM::StartRemoteCooperate(const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto *context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, CooperateEventMgr,
                  CooperationMessage::INFO_START, remoteNetworkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    isStarting_ = true;
}

void InputDeviceCooperateSM::StartRemoteCooperateResult(bool isSuccess,
    const std::string& startDhid, int32_t xPercent, int32_t yPercent)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGI("Not in starting");
        return;
    }
    startDhid_ = startDhid;
    CooperationMessage msg =
            isSuccess ? CooperationMessage::INFO_SUCCESS : CooperationMessage::INFO_FAIL;
    auto *context = CooperateEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, CooperateEventMgr, msg, ""));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }

    if (!isSuccess || cooperateState_ == CooperateState::STATE_IN) {
        isStarting_ = false;
        return;
    }
    if (cooperateState_ == CooperateState::STATE_FREE) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        UpdateState(CooperateState::STATE_IN);
    }
    if (cooperateState_ == CooperateState::STATE_OUT) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        UpdateState(CooperateState::STATE_FREE);
    }
    isStarting_ = false;
}

void InputDeviceCooperateSM::StopRemoteCooperate()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    isStopping_ = true;
}

void InputDeviceCooperateSM::StopRemoteCooperateResult(bool isSuccess)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStopping_) {
        FI_HILOGI("Not in stopping");
        return;
    }
    if (isSuccess) {
        Reset(true);
    }
    isStopping_ = false;
}

void InputDeviceCooperateSM::StartCooperateOtherResult(const std::string& srcNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    srcNetworkId_ = srcNetworkId;
}

void InputDeviceCooperateSM::OnStartFinish(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startInputDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGE("Not in starting");
        return;
    }

    if (!isSuccess) {
        FI_HILOGE("Start distributed fail, startInputDevice: %{public}d", startInputDeviceId);
        NotifyRemoteStartFail(remoteNetworkId);
    } else {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        startDhid_ = context->GetDeviceManager().GetDhid(startInputDeviceId);
        NotifyRemoteStartSuccess(remoteNetworkId, startDhid_);
        if (cooperateState_ == CooperateState::STATE_FREE) {
            UpdateState(CooperateState::STATE_OUT);
        } else if (cooperateState_ == CooperateState::STATE_IN) {
            std::string sink = context->GetDeviceManager().GetOriginNetworkId(startInputDeviceId);
            if (!sink.empty() && remoteNetworkId != sink) {
                DevCooperateSoftbusAdapter->StartCooperateOtherResult(sink, remoteNetworkId);
            }
            UpdateState(CooperateState::STATE_FREE);
        } else {
            FI_HILOGI("Current state is out");
        }
    }
    isStarting_ = false;
}

void InputDeviceCooperateSM::OnStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStopping_) {
        FI_HILOGE("Not in stopping");
        return;
    }
    NotifyRemoteStopFinish(isSuccess, remoteNetworkId);
    if (isSuccess) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        if (context->GetDeviceManager().HasLocalPointerDevice()) {
            SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
        }
        if (cooperateState_ == CooperateState::STATE_IN || cooperateState_ == CooperateState::STATE_OUT) {
            UpdateState(CooperateState::STATE_FREE);
        } else {
            FI_HILOGI("Current state is free");
        }
    }
    DevCooperateSoftbusAdapter->CloseInputSoftbus(remoteNetworkId);
    isStopping_ = false;
}

void InputDeviceCooperateSM::NotifyRemoteStartFail(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    DevCooperateSoftbusAdapter->StartRemoteCooperateResult(remoteNetworkId, false, "",  0, 0);
    CooperateEventMgr->OnStart(CooperationMessage::INFO_FAIL);
}

void InputDeviceCooperateSM::NotifyRemoteStartSuccess(const std::string &remoteNetworkId, const std::string& startDhid)
{
    CALL_DEBUG_ENTER;
    DevCooperateSoftbusAdapter->StartRemoteCooperateResult(remoteNetworkId,
        true, startDhid, mouseLocation_.first, mouseLocation_.second);
    CooperateEventMgr->OnStart(CooperationMessage::INFO_SUCCESS);
}

void InputDeviceCooperateSM::NotifyRemoteStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    DevCooperateSoftbusAdapter->StopRemoteCooperateResult(remoteNetworkId, isSuccess);
    if (!isSuccess) {
        CooperateEventMgr->OnStop(CooperationMessage::COOPERATE_FAIL);
    } else {
        CooperateEventMgr->OnStop(CooperationMessage::STOP_SUCCESS);
    }
}

bool InputDeviceCooperateSM::UpdateMouseLocation()
{
    CALL_DEBUG_ENTER;
    auto display = OHOS::Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        return false;
    }
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    if (width == 0 || height == 0) {
        FI_HILOGE("display width or height is 0");
        return false;
    }
    int32_t xPercent = x_ * MOUSE_ABS_LOCATION / width;
    int32_t yPercent = y_ * MOUSE_ABS_LOCATION / height;
    FI_HILOGI("displayWidth: %{public}d, displayHeight: %{public}d, "
        "physicalX: %{public}d, physicalY: %{public}d,",
        width, height, x_, y_);
    mouseLocation_ = std::make_pair(xPercent, yPercent);
    return true;
}

void InputDeviceCooperateSM::UpdateState(CooperateState state)
{
    FI_HILOGI("state: %{public}d", state);
    switch (state) {
        case CooperateState::STATE_FREE: {
            Reset();
            break;
        }
        case CooperateState::STATE_IN: {
            currentStateSM_ = std::make_shared<InputDeviceCooperateStateIn>(startDhid_);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COORDINATION_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD));
            if (interceptorId_ <= 0) {
                StopInputDeviceCooperate();
                return;
            }
            break;
        }
        case CooperateState::STATE_OUT: {
            auto* context = CooperateEventMgr->GetIContext();
            CHKPV(context);
            InputMgr->SetPointerVisible(false);
            currentStateSM_ = std::make_shared<InputDeviceCooperateStateOut>(startDhid_);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COORDINATION_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD) | CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER));
            if (interceptorId_ <= 0) {
                StopInputDeviceCooperate();
                return;
            }
            break;
        }
        default:
            break;
    }
    cooperateState_ = state;
}

CooperateState InputDeviceCooperateSM::GetCurrentCooperateState() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return cooperateState_;
}

void InputDeviceCooperateSM::UpdatePreparedDevices(const std::string &srcNetworkId, const std::string &sinkNetworkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    preparedNetworkId_ = std::make_pair(srcNetworkId, sinkNetworkId);
}

std::pair<std::string, std::string> InputDeviceCooperateSM::GetPreparedDevices() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return preparedNetworkId_;
}

bool InputDeviceCooperateSM::IsStarting() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStarting_;
}

bool InputDeviceCooperateSM::IsStopping() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStopping_;
}

void InputDeviceCooperateSM::OnKeyboardOnline(const std::string &dhid)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(currentStateSM_);
    currentStateSM_->OnKeyboardOnline(dhid);
}

void InputDeviceCooperateSM::OnPointerOffline(const std::string &dhid, const std::string &sinkNetworkId,
    const std::vector<std::string> &keyboards)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (cooperateState_ == CooperateState::STATE_FREE) {
        Reset();
        return;
    }
    if (cooperateState_ == CooperateState::STATE_IN && startDhid_ == dhid) {
        Reset();
        return;
    }
    if (cooperateState_ == CooperateState::STATE_OUT && startDhid_ == dhid) {
        std::string src = srcNetworkId_;
        if (src.empty()) {
            src = preparedNetworkId_.first;
        }
        DistributedAdapter->StopRemoteInput(src, sinkNetworkId, keyboards, [this, src](bool isSuccess) {});
        Reset();
    }
}

bool InputDeviceCooperateSM::HandleEvent(libinput_event *event)
{
    FI_HILOGI("current state :%{public}d", cooperateState_);
    CHKPF(event);
    auto type = libinput_event_get_type(event);
    switch (type) {
        case LIBINPUT_EVENT_POINTER_MOTION:
        case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
        case LIBINPUT_EVENT_POINTER_BUTTON:
        case LIBINPUT_EVENT_POINTER_AXIS: {
            if (!CheckPointerEvent(event)) {
                return false;
            }
            break;
        }
        default: {
            break;
        }
    }
    return true;
}

bool InputDeviceCooperateSM::CheckPointerEvent(struct libinput_event *event)
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStopping_ || isStarting_) {
        FI_HILOGE("In transition state, not process");
        return false;
    }
    auto inputDevice = libinput_event_get_device(event);
    auto* context = CooperateEventMgr->GetIContext();
    CHKPF(context);
    IDeviceManager &devMgr = context->GetDeviceManager();

    if (cooperateState_ == CooperateState::STATE_IN) {
        if (!context->IsRemote(inputDevice)) {
            CHKPF(currentStateSM_);
            isStopping_ = true;
            std::string sink = devMgr.GetOriginNetworkId(startDhid_);
            int32_t ret = currentStateSM_->StopInputDeviceCooperate(sink);
            if (ret != RET_OK) {
                FI_HILOGE("Stop input device cooperate fail");
                isStopping_ = false;
            }
            return false;
        }
    } else if (cooperateState_ == CooperateState::STATE_OUT) {
        int32_t deviceId = context->FindInputDeviceId(inputDevice);
        std::string dhid = devMgr.GetDhid(deviceId);
        if (startDhid_ != dhid) {
            FI_HILOGI("Move other mouse, stop input device cooperate");
            CHKPF(currentStateSM_);
            isStopping_ = true;
            int32_t ret = currentStateSM_->StopInputDeviceCooperate(srcNetworkId_);
            if (ret != RET_OK) {
                FI_HILOGE("Stop input device cooperate fail");
                isStopping_ = false;
            }
        }
        return false;
    } else {
        if (context->IsRemote(inputDevice)) {
            return false;
        }
    }
    return true;
}

bool InputDeviceCooperateSM::InitDeviceManager()
{
    CALL_DEBUG_ENTER;
    initCallback_ = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DisHardware.InitDeviceManager(MMI_DINPUT_PKG_NAME, initCallback_);
    if (ret != 0) {
        FI_HILOGE("Init device manager failed, ret:%{public}d", ret);
        return false;
    }
    stateCallback_ = std::make_shared<MmiDeviceStateCallback>();
    ret = DisHardware.RegisterDevStateCallback(MMI_DINPUT_PKG_NAME, "", stateCallback_);
    if (ret != 0) {
        FI_HILOGE("Register devStateCallback failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

void InputDeviceCooperateSM::OnDeviceOnline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    onlineDevice_.push_back(networkId);
    DProfileAdapter->RegisterCrossingStateListener(networkId,
        std::bind(&InputDeviceCooperateSM::OnCooperateChanged,
        InputDevCooSM, std::placeholders::_1, std::placeholders::_2));
}

void InputDeviceCooperateSM::OnDeviceOffline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    DProfileAdapter->UnregisterCrossingStateListener(networkId);
    Reset(networkId);
    std::lock_guard<std::mutex> guard(mutex_);
    if (!onlineDevice_.empty()) {
        auto it = std::find(onlineDevice_.begin(), onlineDevice_.end(), networkId);
        if (it != onlineDevice_.end()) {
            onlineDevice_.erase(it);
        }
    }
}

void InputDeviceCooperateSM::Dump(int32_t fd, const std::vector<std::string> &args)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    dprintf(fd, "Keyboard and mouse crossing information:");
    dprintf(fd, "State machine status: %d\t", cooperateState_);
    dprintf(fd, "Peripheral keyboard and mouse information: startDhid_  srcNetworkId_:\t");
    dprintf(fd, "%s", startDhid_.c_str());
    dprintf(fd, "%s", srcNetworkId_.c_str());
    dprintf(fd, "Run successfully");
}

void InputDeviceCooperateSM::RemoveMonitor()
{
    if ((monitorId_ >= MIN_HANDLER_ID) && (monitorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveMonitor(monitorId_);
        monitorId_ = -1;
    }
}

void InputDeviceCooperateSM::RemoveInterceptor()
{
    if ((interceptorId_ >= MIN_HANDLER_ID) && (interceptorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveInterceptor(interceptorId_);
        interceptorId_ = -1;
    }
}

bool InputDeviceCooperateSM::IsNeedFilterOut(const std::string &deviceId,
    const std::shared_ptr<MMI::KeyEvent> keyEvent)
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
    for (const auto &item : businessEvent.pressedKeys) {
        FI_HILOGI("pressedKeys :%{public}d", item);
    }
    return DistributedAdapter->IsNeedFilterOut(deviceId, businessEvent);
}

void InputDeviceCooperateSM::DeviceInitCallBack::OnRemoteDied()
{
    CALL_INFO_TRACE;
}

void InputDeviceCooperateSM::MmiDeviceStateCallback::OnDeviceOnline(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    InputDevCooSM->OnDeviceOnline(deviceInfo.deviceId);
}

void InputDeviceCooperateSM::MmiDeviceStateCallback::OnDeviceOffline(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
    InputDevCooSM->OnDeviceOffline(deviceInfo.deviceId);
}

void InputDeviceCooperateSM::MmiDeviceStateCallback::OnDeviceChanged(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void InputDeviceCooperateSM::MmiDeviceStateCallback::OnDeviceReady(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void InputDeviceCooperateSM::SetAbsolutionLocation(double xPercent, double yPercent)
{
    CALL_INFO_TRACE;
    auto display = OHOS::Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        FI_HILOGD("display is nullptr");
        return;
    }
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    int32_t physicalX = static_cast<int32_t>(width * xPercent / PERCENT_CONST);
    int32_t physicalY = static_cast<int32_t>(height * yPercent / PERCENT_CONST);
    FI_HILOGD("width:%{public}d, height:%{public}d, physicalX:%{public}d,"
        "physicalX:%{public}d, x_:%{public}d, y_:%{public}d",width,height,physicalX,physicalY,x_,y_);
    InputMgr->SetPointerLocation(physicalX, physicalY);
}

void InputDeviceCooperateSM::DeviceObserver::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    CHKPV(device);
    if (device->IsKeyboard()) {
        InputDevCooSM->OnKeyboardOnline(device->GetDhid());
    }
}

void InputDeviceCooperateSM::DeviceObserver::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    CHKPV(device);
    if (device->IsPointerDevice()) {
        auto *context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        InputDevCooSM->OnPointerOffline(device->GetDhid(), device->GetNetworkId(),
                                        context->GetDeviceManager().GetCooperateDhids(device->GetId()));
    }
}

void InputDeviceCooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(keyEvent);
    int32_t keyCode = keyEvent->GetKeyCode();
    if (keyCode == MMI::KeyEvent::KEYCODE_BACK || keyCode == MMI::KeyEvent::KEYCODE_VOLUME_UP
        || keyCode == MMI::KeyEvent::KEYCODE_VOLUME_DOWN || keyCode == MMI::KeyEvent::KEYCODE_POWER) {
        keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
        MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        return;
    }
    CooperateState state = InputDevCooSM->GetCurrentCooperateState();
    FI_HILOGI("Get current cooperate state:%{public}d", state);
    if (state == CooperateState::STATE_IN) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = keyEvent->GetDeviceId();
        if (context->GetDeviceManager().IsRemote(deviceId)) {
            auto networkId = context->GetDeviceManager().GetOriginNetworkId(deviceId);
            if (!InputDevCooSM->IsNeedFilterOut(networkId, keyEvent)) {
                keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
                MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
            }
        }
    } else if (state == CooperateState::STATE_OUT) {
        std::string networkId = COOPERATE::GetLocalDeviceId();
        if (InputDevCooSM->IsNeedFilterOut(networkId, keyEvent)) {
            keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
            MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        }
    }
    return;
}

void InputDeviceCooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    CooperateState state = InputDevCooSM->GetCurrentCooperateState();
    if (state == CooperateState::STATE_OUT) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = pointerEvent->GetDeviceId();
        std::string dhid = context->GetDeviceManager().GetDhid(deviceId);
        if (InputDevCooSM->startDhid_ != dhid) {
            FI_HILOGI("Move other mouse, stop input device cooperate");
            CHKPV(InputDevCooSM->currentStateSM_);
            InputDevCooSM->StopInputDeviceCooperate();
        }
    }
}

void InputDeviceCooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

void InputDeviceCooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
}

void InputDeviceCooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    if (pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        MMI::PointerEvent::PointerItem pointerItem;
        pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
        InputDevCooSM->x_ = pointerItem.GetDisplayX();
        InputDevCooSM->y_ = pointerItem.GetDisplayY();
    }
    CooperateState state = InputDevCooSM->GetCurrentCooperateState();
    if (state == CooperateState::STATE_IN) {
        auto* context = CooperateEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = pointerEvent->GetDeviceId();
        if (!context->GetDeviceManager().IsRemote(deviceId)) {
            CHKPV(InputDevCooSM->currentStateSM_);
            InputDevCooSM->StopInputDeviceCooperate();
        }
    }
}

void InputDeviceCooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
