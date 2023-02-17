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
#include "coordination_sm.h"

#include <cstdio>
#include <unistd.h>

#include "device_manager.h"
#include "display_manager.h"
#include "hitrace_meter.h"
#include "input_manager.h"

#include "coordination_event_manager.h"
#include "coordination_message.h"
#include "coordination_softbus_adapter.h"
#include "device_profile_adapter.h"
#include "display_info.h"
#include "coordination_state_free.h"
#include "coordination_state_in.h"
#include "coordination_state_out.h"
#include "coordination_util.h"
#include "input_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationSM" };
constexpr int32_t INTERVAL_MS = 2000;
constexpr double PERCENT_CONST = 100.0;
constexpr int32_t MOUSE_ABS_LOCATION = 100;
constexpr int32_t MOUSE_ABS_LOCATION_X = 50;
constexpr int32_t MOUSE_ABS_LOCATION_Y = 50;
constexpr int32_t COORDINATION_PRIORITY = 499;
constexpr int32_t MIN_HANDLER_ID = 1;
} // namespace

CoordinationSM::CoordinationSM() {}
CoordinationSM::~CoordinationSM()
{
    RemoveMonitor();
    RemoveInterceptor();
}

void CoordinationSM::Init()
{
    CALL_INFO_TRACE;
    preparedNetworkId_ = std::make_pair("", "");
    currentStateSM_ = std::make_shared<CoordinationStateFree>();
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    context->GetTimerManager().AddTimer(INTERVAL_MS, 1, [this]() {
        this->InitDeviceManager();
        CooSoftbusAdapter->Init();
    });
    devObserver_ = std::make_shared<DeviceObserver>();
    context->GetDeviceManager().AddDeviceObserver(devObserver_);
}

void CoordinationSM::OnSessionLost(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    CHKPV(session);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPV(event);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = session;
    CoordinationEventMgr->RemoveCoordinationEvent(event);
    RemoveMonitor();
    RemoveInterceptor();
}

void CoordinationSM::Reset(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    bool needReset = true;
    if (coordinationState_ == CoordinationState::STATE_OUT) {
        if (networkId != srcNetworkId_) {
            needReset = false;
        }
    }
    if (coordinationState_ == CoordinationState::STATE_IN) {
        auto* context = CoordinationEventMgr->GetIContext();
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

void CoordinationSM::Reset(bool adjustAbsolutionLocation)
{
    CALL_INFO_TRACE;
    startDhid_ = "";
    srcNetworkId_ = "";
    currentStateSM_ = std::make_shared<CoordinationStateFree>();
    coordinationState_ = CoordinationState::STATE_FREE;
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    bool hasPointer = context->GetDeviceManager().HasLocalPointerDevice();
    if (hasPointer && adjustAbsolutionLocation) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
    } else {
        OHOS::MMI::InputManager::GetInstance()->SetPointerVisible(hasPointer);
    }
    isStarting_ = false;
    isStopping_ = false;
    RemoveInterceptor();
}

void CoordinationSM::OnCoordinationChanged(const std::string &networkId, bool isOpen)
{
    CALL_DEBUG_ENTER;
    CoordinationMessage msg = isOpen ? CoordinationMessage::STATE_ON : CoordinationMessage::STATE_OFF;
    auto *context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CoordinationEventManager::OnCoordinationMessage, CoordinationEventMgr, msg, networkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    if (!isOpen) {
        OnCloseCoordination(networkId, false);
    }
}

void CoordinationSM::OnCloseCoordination(const std::string &networkId, bool isLocal)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!preparedNetworkId_.first.empty() && !preparedNetworkId_.second.empty()) {
        if (networkId == preparedNetworkId_.first || networkId == preparedNetworkId_.second) {
            if (coordinationState_ != CoordinationState::STATE_FREE) {
                auto* context = CoordinationEventMgr->GetIContext();
                CHKPV(context);
                auto dhids = context->GetDeviceManager().GetCoordinationDhids(startDhid_);
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
    if (coordinationState_ == CoordinationState::STATE_FREE) {
        return;
    }
    if (isLocal || networkId == srcNetworkId_) {
        Reset(true);
        return;
    }
    auto* context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    std::string originNetworkId = context->GetDeviceManager().GetOriginNetworkId(startDhid_);
    if (originNetworkId == networkId) {
        Reset();
    }
}

void CoordinationSM::GetCoordinationState(const std::string &deviceId)
{
    CALL_INFO_TRACE;
    bool state = DProfileAdapter->GetCrossingSwitchState(deviceId);
    CoordinationEventMgr->OnGetState(state);
}

void CoordinationSM::EnableCoordination(bool enabled)
{
    CALL_INFO_TRACE;
    if (enabled) {
        if (monitorId_ <= 0) {
            auto monitor = std::make_shared<MonitorConsumer>(
                std::bind(&CoordinationSM::UpdateLastPointerEventCallback, this, std::placeholders::_1));
            monitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor);
            if (monitorId_ <= 0) {
                FI_HILOGE("Failed to add monitor, Error code:%{public}d", monitorId_);
                monitorId_ = -1;
                return;
            }
        }
        DProfileAdapter->UpdateCrossingSwitchState(enabled, onlineDevice_);
    } else {
        DProfileAdapter->UpdateCrossingSwitchState(enabled, onlineDevice_);
        std::string localNetworkId = COORDINATION::GetLocalDeviceId();
        OnCloseCoordination(localNetworkId, true);
        RemoveMonitor();
    }
}

int32_t CoordinationSM::StartCoordination(
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStarting_) {
        FI_HILOGE("In transition state, not process");
        return static_cast<int32_t>(CoordinationMessage::COORDINATION_FAIL);
    }
    CHKPR(currentStateSM_, ERROR_NULL_POINTER);
    isStarting_ = true;
    CooSoftbusAdapter->OpenInputSoftbus(remoteNetworkId);
    int32_t ret = currentStateSM_->StartCoordination(remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Start remote input fail");
        isStarting_ = false;
        return ret;
    }
    UpdateMouseLocation();
    if (coordinationState_ == CoordinationState::STATE_FREE) {
        srcNetworkId_ = remoteNetworkId;
    }
    return ret;
}

int32_t CoordinationSM::StopCoordination()
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
    if (coordinationState_ == CoordinationState::STATE_IN) {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPR(context, ERROR_NULL_POINTER);
        stopNetworkId = context->GetDeviceManager().GetOriginNetworkId(startDhid_);
    }
    if (coordinationState_ == CoordinationState::STATE_OUT) {
        stopNetworkId = srcNetworkId_;
    }
    int32_t ret = currentStateSM_->StopCoordination(stopNetworkId);
    if (ret != RET_OK) {
        FI_HILOGE("Stop input device coordination fail");
        isStopping_ = false;
    }
    return ret;
}

void CoordinationSM::StartRemoteCoordination(const std::string &remoteNetworkId, bool buttonIsPressed)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto *context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CoordinationEventManager::OnCoordinationMessage, CoordinationEventMgr,
                  CoordinationMessage::INFO_START, remoteNetworkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    isStarting_ = true;
    if (buttonIsPressed) {
        StartPointerEventFilter();
    }
}

void CoordinationSM::StartPointerEventFilter()
{
    CALL_INFO_TRACE;
    int32_t POINTER_DEFAULT_PRIORITY = 220;
    auto filter = std::make_shared<PointerFilter>();
    uint32_t touchTags = CapabilityToTags(MMI::INPUT_DEV_CAP_MAX);
    filterId_ = OHOS::MMI::InputManager::GetInstance()->AddInputEventFilter(filter, POINTER_DEFAULT_PRIORITY,
        touchTags);
    if (0 > filterId_) {
        FI_HILOGE("Add Event Filter Failed.");
    }
    filter->UpdateCurrentFilterId(filterId_);
}

void CoordinationSM::StartRemoteCoordinationResult(bool isSuccess,
    const std::string& startDhid, int32_t xPercent, int32_t yPercent)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGI("Not in starting");
        return;
    }
    startDhid_ = startDhid;
    CoordinationMessage msg =
            isSuccess ? CoordinationMessage::INFO_SUCCESS : CoordinationMessage::INFO_FAIL;
    auto *context = CoordinationEventMgr->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetDelegateTasks().PostAsyncTask(
        std::bind(&CoordinationEventManager::OnCoordinationMessage, CoordinationEventMgr, msg, ""));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }

    if (!isSuccess || coordinationState_ == CoordinationState::STATE_IN) {
        isStarting_ = false;
        return;
    }
    if (coordinationState_ == CoordinationState::STATE_FREE) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        UpdateState(CoordinationState::STATE_IN);
    }
    if (coordinationState_ == CoordinationState::STATE_OUT) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        UpdateState(CoordinationState::STATE_FREE);
    }
    isStarting_ = false;
}

void CoordinationSM::StopRemoteCoordination()
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    isStopping_ = true;
}

void CoordinationSM::StopRemoteCoordinationResult(bool isSuccess)
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

void CoordinationSM::StartCoordinationOtherResult(const std::string& srcNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    srcNetworkId_ = srcNetworkId;
}

void CoordinationSM::OnStartFinish(bool isSuccess,
    const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGE("Not in starting");
        return;
    }

    if (!isSuccess) {
        FI_HILOGE("Start distributed fail, startDevice: %{public}d", startDeviceId);
        NotifyRemoteStartFail(remoteNetworkId);
    } else {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        startDhid_ = context->GetDeviceManager().GetDhid(startDeviceId);
        NotifyRemoteStartSuccess(remoteNetworkId, startDhid_);
        if (coordinationState_ == CoordinationState::STATE_FREE) {
            UpdateState(CoordinationState::STATE_OUT);
        } else if (coordinationState_ == CoordinationState::STATE_IN) {
            std::string sink = context->GetDeviceManager().GetOriginNetworkId(startDeviceId);
            if (!sink.empty() && remoteNetworkId != sink) {
                CooSoftbusAdapter->StartCoordinationOtherResult(sink, remoteNetworkId);
            }
            UpdateState(CoordinationState::STATE_FREE);
        } else {
            FI_HILOGI("Current state is out");
        }
    }
    isStarting_ = false;
}

void CoordinationSM::OnStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStopping_) {
        FI_HILOGE("Not in stopping");
        return;
    }
    NotifyRemoteStopFinish(isSuccess, remoteNetworkId);
    if (isSuccess) {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        if (context->GetDeviceManager().HasLocalPointerDevice()) {
            SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
        }
        if (coordinationState_ == CoordinationState::STATE_IN || coordinationState_ == CoordinationState::STATE_OUT) {
            UpdateState(CoordinationState::STATE_FREE);
        } else {
            FI_HILOGI("Current state is free");
        }
    }
    CooSoftbusAdapter->CloseInputSoftbus(remoteNetworkId);
    isStopping_ = false;
}

void CoordinationSM::NotifyRemoteStartFail(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    CooSoftbusAdapter->StartRemoteCoordinationResult(remoteNetworkId, false, "",  0, 0);
    CoordinationEventMgr->OnStart(CoordinationMessage::INFO_FAIL);
}

void CoordinationSM::NotifyRemoteStartSuccess(const std::string &remoteNetworkId, const std::string& startDhid)
{
    CALL_DEBUG_ENTER;
    CooSoftbusAdapter->StartRemoteCoordinationResult(remoteNetworkId,
        true, startDhid, mouseLocation_.first, mouseLocation_.second);
    CoordinationEventMgr->OnStart(CoordinationMessage::INFO_SUCCESS);
}

void CoordinationSM::NotifyRemoteStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    CooSoftbusAdapter->StopRemoteCoordinationResult(remoteNetworkId, isSuccess);
    if (!isSuccess) {
        CoordinationEventMgr->OnStop(CoordinationMessage::COORDINATION_FAIL);
    } else {
        CoordinationEventMgr->OnStop(CoordinationMessage::STOP_SUCCESS);
    }
}

bool CoordinationSM::UpdateMouseLocation()
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
    int32_t xPercent = displayX_ * MOUSE_ABS_LOCATION / width;
    int32_t yPercent = displayY_ * MOUSE_ABS_LOCATION / height;
    FI_HILOGI("displayWidth: %{public}d, displayHeight: %{public}d, "
        "physicalX: %{public}d, physicalY: %{public}d,",
        width, height, displayX_, displayY_);
    mouseLocation_ = std::make_pair(xPercent, yPercent);
    return true;
}

void CoordinationSM::UpdateState(CoordinationState state)
{
    FI_HILOGI("state: %{public}d", state);
    switch (state) {
        case CoordinationState::STATE_FREE: {
            Reset();
            break;
        }
        case CoordinationState::STATE_IN: {
            currentStateSM_ = std::make_shared<CoordinationStateIn>(startDhid_);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COORDINATION_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD));
            if (interceptorId_ <= 0) {
                FI_HILOGE("Failed to add interceptor, Error code:%{public}d", interceptorId_);
                StopCoordination();
                return;
            }
            break;
        }
        case CoordinationState::STATE_OUT: {
            auto* context = CoordinationEventMgr->GetIContext();
            CHKPV(context);
            OHOS::MMI::InputManager::GetInstance()->SetPointerVisible(false);
            currentStateSM_ = std::make_shared<CoordinationStateOut>(startDhid_);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COORDINATION_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD) | CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER));
            if (interceptorId_ <= 0) {
                FI_HILOGE("Failed to add interceptor, Error code:%{public}d", interceptorId_);
                StopCoordination();
                return;
            }
            break;
        }
        default:
            break;
    }
    coordinationState_ = state;
}

CoordinationState CoordinationSM::GetCurrentCoordinationState() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return coordinationState_;
}

void CoordinationSM::UpdatePreparedDevices(const std::string &srcNetworkId, const std::string &sinkNetworkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    preparedNetworkId_ = std::make_pair(srcNetworkId, sinkNetworkId);
}

std::pair<std::string, std::string> CoordinationSM::GetPreparedDevices() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return preparedNetworkId_;
}

bool CoordinationSM::IsStarting() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStarting_;
}

bool CoordinationSM::IsStopping() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStopping_;
}

void CoordinationSM::OnKeyboardOnline(const std::string &dhid)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    CHKPV(currentStateSM_);
    currentStateSM_->OnKeyboardOnline(dhid);
}

void CoordinationSM::OnPointerOffline(const std::string &dhid, const std::string &sinkNetworkId,
    const std::vector<std::string> &keyboards)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (coordinationState_ == CoordinationState::STATE_FREE) {
        Reset();
        return;
    }
    if (coordinationState_ == CoordinationState::STATE_IN && startDhid_ == dhid) {
        Reset();
        return;
    }
    if (coordinationState_ == CoordinationState::STATE_OUT && startDhid_ == dhid) {
        std::string src = srcNetworkId_;
        if (src.empty()) {
            src = preparedNetworkId_.first;
        }
        DistributedAdapter->StopRemoteInput(src, sinkNetworkId, keyboards, [this, src](bool isSuccess) {});
        Reset();
    }
}

bool CoordinationSM::InitDeviceManager()
{
    CALL_DEBUG_ENTER;
    initCallback_ = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DisHardware.InitDeviceManager(FI_PKG_NAME, initCallback_);
    if (ret != 0) {
        FI_HILOGE("Init device manager failed, ret:%{public}d", ret);
        return false;
    }
    stateCallback_ = std::make_shared<DmDeviceStateCallback>();
    ret = DisHardware.RegisterDevStateCallback(FI_PKG_NAME, "", stateCallback_);
    if (ret != 0) {
        FI_HILOGE("Register devStateCallback failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

void CoordinationSM::OnDeviceOnline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    onlineDevice_.push_back(networkId);
    DProfileAdapter->RegisterCrossingStateListener(networkId,
        std::bind(&CoordinationSM::OnCoordinationChanged,
        CooSM, std::placeholders::_1, std::placeholders::_2));
}

void CoordinationSM::OnDeviceOffline(const std::string &networkId)
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

std::string CoordinationSM::GetDeviceCoordinationState(CoordinationState value) const
{
    std::string state;
    switch (value) {
        case CoordinationState::STATE_FREE: {
            state = "free";
            break;
        }
        case CoordinationState::STATE_IN: {
            state = "in";
            break;
        }
        case CoordinationState::STATE_OUT: {
            state = "out";
            break;
        }
        default: {
            state = "unknown";
            FI_HILOGW("Coordination status unknown");
            break;
        }
    }
    return state;
}

void CoordinationSM::Dump(int32_t fd)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    dprintf(fd, "Coordination information:\n");
    dprintf(fd,
            "coordinationState:%s | startDhid:%s | srcNetworkId:%s | isStarting:%s | isStopping:%s\n"
            "physicalX:%d | physicalY:%d | displayX:%d | displayY:%d\n",
            GetDeviceCoordinationState(coordinationState_).c_str(), startDhid_.c_str(), srcNetworkId_.c_str(),
            isStarting_ ? "true" : "false", isStopping_ ? "true" : "false",
            mouseLocation_.first, mouseLocation_.second, displayX_, displayY_);
    if (onlineDevice_.empty()) {
        dprintf(fd, "onlineDevice:%s\n", "None");
        return;
    }
    for (const auto &item : onlineDevice_) {
        dprintf(fd, "onlineDevice:%s\n", item.c_str());
    }
}

void CoordinationSM::UpdateLastPointerEventCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    lastPointerEvent_ = pointerEvent;
}

std::shared_ptr<MMI::PointerEvent> CoordinationSM::GetLastPointerEvent() const
{
    return lastPointerEvent_;
}

void CoordinationSM::RemoveMonitor()
{
    if ((monitorId_ >= MIN_HANDLER_ID) && (monitorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveMonitor(monitorId_);
        monitorId_ = -1;
    }
}

void CoordinationSM::RemoveInterceptor()
{
    if ((interceptorId_ >= MIN_HANDLER_ID) && (interceptorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveInterceptor(interceptorId_);
        interceptorId_ = -1;
    }
}

bool CoordinationSM::IsNeedFilterOut(const std::string &deviceId,
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
    FI_HILOGI("businessEvent.keyCode:%{public}d, keyAction:%{public}d",
        businessEvent.keyCode, businessEvent.keyAction);
    for (const auto &item : businessEvent.pressedKeys) {
        FI_HILOGI("pressedKeys :%{public}d", item);
    }
    return DistributedAdapter->IsNeedFilterOut(deviceId, businessEvent);
}

void CoordinationSM::DeviceInitCallBack::OnRemoteDied()
{
    CALL_INFO_TRACE;
}

void CoordinationSM::DmDeviceStateCallback::OnDeviceOnline(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CooSM->OnDeviceOnline(deviceInfo.deviceId);
}

void CoordinationSM::DmDeviceStateCallback::OnDeviceOffline(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
    CooSM->OnDeviceOffline(deviceInfo.deviceId);
}

void CoordinationSM::DmDeviceStateCallback::OnDeviceChanged(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void CoordinationSM::DmDeviceStateCallback::OnDeviceReady(
    const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void CoordinationSM::SetAbsolutionLocation(double xPercent, double yPercent)
{
    CALL_INFO_TRACE;
    auto display = OHOS::Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        FI_HILOGE("display is nullptr");
        return;
    }
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    int32_t physicalX = static_cast<int32_t>(width * xPercent / PERCENT_CONST);
    int32_t physicalY = static_cast<int32_t>(height * yPercent / PERCENT_CONST);
    FI_HILOGD("width:%{public}d, height:%{public}d, physicalX:%{public}d, physicalY:%{public}d",
        width, height, physicalX, physicalY);
    OHOS::MMI::InputManager::GetInstance()->SetPointerLocation(physicalX, physicalY);
}

void CoordinationSM::DeviceObserver::OnDeviceAdded(std::shared_ptr<IDevice> device)
{
    CHKPV(device);
    if (device->IsKeyboard()) {
        CooSM->OnKeyboardOnline(device->GetDhid());
    }
}

void CoordinationSM::DeviceObserver::OnDeviceRemoved(std::shared_ptr<IDevice> device)
{
    CHKPV(device);
    if (device->IsPointerDevice()) {
        auto *context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        CooSM->OnPointerOffline(device->GetDhid(), device->GetNetworkId(),
            context->GetDeviceManager().GetCoordinationDhids(device->GetId()));
    }
}

void CoordinationSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
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
    CoordinationState state = CooSM->GetCurrentCoordinationState();
    if (state == CoordinationState::STATE_IN) {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = keyEvent->GetDeviceId();
        if (context->GetDeviceManager().IsRemote(deviceId)) {
            auto networkId = context->GetDeviceManager().GetOriginNetworkId(deviceId);
            if (!CooSM->IsNeedFilterOut(networkId, keyEvent)) {
                keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
                MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
            }
        }
    } else if (state == CoordinationState::STATE_OUT) {
        std::string networkId = COORDINATION::GetLocalDeviceId();
        if (CooSM->IsNeedFilterOut(networkId, keyEvent)) {
            keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
            MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        }
    }
}

void CoordinationSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    CoordinationState state = CooSM->GetCurrentCoordinationState();
    if (state == CoordinationState::STATE_OUT) {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = pointerEvent->GetDeviceId();
        std::string dhid = context->GetDeviceManager().GetDhid(deviceId);
        if (CooSM->startDhid_ != dhid) {
            FI_HILOGI("Move other mouse, stop input device coordination");
            CHKPV(CooSM->currentStateSM_);
            CooSM->StopCoordination();
        }
    }
}

void CoordinationSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}

void CoordinationSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
}

void CoordinationSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    if (callback_) {
        callback_(pointerEvent);
    }
    if (pointerEvent->GetSourceType() == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        MMI::PointerEvent::PointerItem pointerItem;
        pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
        CooSM->displayX_ = pointerItem.GetDisplayX();
        CooSM->displayY_ = pointerItem.GetDisplayY();
    }
    CoordinationState state = CooSM->GetCurrentCoordinationState();
    if (state == CoordinationState::STATE_IN) {
        auto* context = CoordinationEventMgr->GetIContext();
        CHKPV(context);
        int32_t deviceId = pointerEvent->GetDeviceId();
        if (!context->GetDeviceManager().IsRemote(deviceId)) {
            CHKPV(CooSM->currentStateSM_);
            CooSM->StopCoordination();
        }
    }
}

void CoordinationSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const
{
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
