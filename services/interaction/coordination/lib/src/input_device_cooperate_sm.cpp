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
#include "hitrace_meter.h"

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
#include "stub.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InputDeviceCooperateSM" };
constexpr int32_t INTERVAL_MS = 2000;
constexpr int32_t MOUSE_ABS_LOCATION = 100;
constexpr int32_t MOUSE_ABS_LOCATION_X = 50;
constexpr int32_t MOUSE_ABS_LOCATION_Y = 50;
} // namespace

InputDeviceCooperateSM::InputDeviceCooperateSM() {}
InputDeviceCooperateSM::~InputDeviceCooperateSM() {}

void InputDeviceCooperateSM::Init(DelegateTasksCallback delegateTasksCallback)
{
    CHKPL(delegateTasksCallback);
    delegateTasksCallback_ = delegateTasksCallback;
    preparedNetworkId_ = std::make_pair("", "");
    currentStateSM_ = std::make_shared<InputDeviceCooperateStateFree>();
    DevCooperateSoftbusAdapter->Init();
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPV(context);
    context->AddTimer(INTERVAL_MS, 1, [this]() {
        this->InitDeviceManager();
    });
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
        auto* context = CooperateEventMgr->GetIInputContext();
        CHKPV(context);
        std::string sinkNetwoekId = context->GetOriginNetworkId(startDhid_);
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
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPV(context);
    bool hasPointer = context->HasLocalPointerDevice();
    if (hasPointer && adjustAbsolutionLocation) {
        context->SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
    } else {
        context->SetPointerVisible(getpid(), hasPointer);
    }
    isStarting_ = false;
    isStopping_ = false;
}

void InputDeviceCooperateSM::OnCooperateChanged(const std::string &networkId, bool isOpen)
{
    CALL_DEBUG_ENTER;
    CooperationMessage msg = isOpen ? CooperationMessage::STATE_ON : CooperationMessage::STATE_OFF;
    delegateTasksCallback_(std::bind(&CooperateEventManager::OnCooperateMessage, CooperateEventMgr, msg, networkId));
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
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPV(context);
    std::string originNetworkId = context->GetOriginNetworkId(startDhid_);
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

int32_t InputDeviceCooperateSM::StartInputDeviceCooperateMonitor()
{
    inputDevCooperateCb_ = std::make_shared<InputDevCooperateCallback>();
    return InputMgr->AddMonitor(inputDevCooperateCb_);
}

void InputDeviceCooperateSM::EnableInputDeviceCooperate(bool enabled)
{
    CALL_INFO_TRACE;
    if (enabled) {
        monitorId_ = StartInputDeviceCooperateMonitor();
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
        auto* context = CooperateEventMgr->GetIInputContext();
        CHKPR(context, ERROR_NULL_POINTER);
        stopNetworkId = context->GetOriginNetworkId(startDhid_);
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

void InputDeviceCooperateSM::StartRemoteCooperate(const std::string &remoteNetworkId, bool buttonIsPressed)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(delegateTasksCallback_);
    delegateTasksCallback_(std::bind(&CooperateEventManager::OnCooperateMessage,
        CooperateEventMgr, CooperationMessage::INFO_START, remoteNetworkId));
    isStarting_ = true;
    if (buttonIsPressed == true) {
        StartPointerEventFilter();
    }
}

void InputDeviceCooperateSM::StartPointerEventFilter()
{
    CALL_INFO_TRACE;
    int32_t POINTER_DEFAULT_PRIORITY = 220;
    auto filter = std::make_shared<PointerFilter>();
    filterId_ = InputMgr->AddInputEventFilter(filter, POINTER_DEFAULT_PRIORITY);
    filter->UpdateCurrentFilterId(filterId_);
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
    delegateTasksCallback_(std::bind(&CooperateEventManager::OnCooperateMessage, CooperateEventMgr, msg, ""));

    if (!isSuccess || cooperateState_ == CooperateState::STATE_IN) {
        isStarting_ = false;
        return;
    }
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPV(context);
    if (cooperateState_ == CooperateState::STATE_FREE) {
        context->SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        UpdateState(CooperateState::STATE_IN);
    }
    if (cooperateState_ == CooperateState::STATE_OUT) {
        context->SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
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
        auto* context = CooperateEventMgr->GetIInputContext();
        CHKPV(context);
        startDhid_ = context->GetDhid(startInputDeviceId);
        NotifyRemoteStartSuccess(remoteNetworkId, startDhid_);
        if (cooperateState_ == CooperateState::STATE_FREE) {
            UpdateState(CooperateState::STATE_OUT);
        } else if (cooperateState_ == CooperateState::STATE_IN) {
            std::string sink = context->GetOriginNetworkId(startInputDeviceId);
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
        auto* context = CooperateEventMgr->GetIInputContext();
        CHKPV(context);
        if (context->HasLocalPointerDevice()) {
            context->SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
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
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPF(context);
    auto pointerEvent = context->GetPointerEvent();
    CHKPF(pointerEvent);
    int32_t displayId = pointerEvent->GetTargetDisplayId();
    auto displayGroupInfo = context->GetDisplayGroupInfo();
    MMI::DisplayInfo physicalDisplayInfo;
    for (auto &it : displayGroupInfo.displaysInfo) {
        if (it.id == displayId) {
            physicalDisplayInfo = it;
            break;
        }
    }
    int32_t displayWidth = physicalDisplayInfo.width;
    int32_t displayHeight = physicalDisplayInfo.height;
    if (displayWidth == 0 || displayHeight == 0) {
        FI_HILOGE("display width or height is 0");
        return false;
    }
    auto mouseInfo = context->GetMouseInfo();
    int32_t xPercent = mouseInfo.physicalX * MOUSE_ABS_LOCATION / displayWidth;
    int32_t yPercent = mouseInfo.physicalY * MOUSE_ABS_LOCATION / displayHeight;
    FI_HILOGI("displayWidth: %{public}d, displayHeight: %{public}d, "
        "physicalX: %{public}d, physicalY: %{public}d,",
        displayWidth, displayHeight, mouseInfo.physicalX, mouseInfo.physicalY);
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
            break;
        }
        case CooperateState::STATE_OUT: {
            auto* context = CooperateEventMgr->GetIInputContext();
            CHKPV(context);
            context->SetPointerVisible(getpid(), false);
            currentStateSM_ = std::make_shared<InputDeviceCooperateStateOut>(startDhid_);
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
    auto* context = CooperateEventMgr->GetIInputContext();
    CHKPF(context);
    if (cooperateState_ == CooperateState::STATE_IN) {
        if (!context->IsRemote(inputDevice)) {
            CHKPF(currentStateSM_);
            isStopping_ = true;
            std::string sink = context->GetOriginNetworkId(startDhid_);
            int32_t ret = currentStateSM_->StopInputDeviceCooperate(sink);
            if (ret != RET_OK) {
                FI_HILOGE("Stop input device cooperate fail");
                isStopping_ = false;
            }
            return false;
        }
    } else if (cooperateState_ == CooperateState::STATE_OUT) {
        int32_t deviceId = context->FindInputDeviceId(inputDevice);
        std::string dhid = context->GetDhid(deviceId);
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

std::shared_ptr<InputDevCooperateCallback> InputDeviceCooperateSM::GetCooperateCallback()
{
    return inputDevCooperateCb_;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
