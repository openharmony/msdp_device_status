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

#include "cooperate_sm.h"

#include <cstdio>
#include <unistd.h>

#include "device_manager.h"
#include "display_info.h"
#include "display_manager.h"
#include "hitrace_meter.h"
#include "input_manager.h"

#include "cooperate_device_manager.h"
#include "cooperate_event_manager.h"
#include "cooperate_message.h"
#include "cooperate_softbus_adapter.h"
#include "cooperate_state_free.h"
#include "cooperate_state_in.h"
#include "cooperate_state_out.h"
#include "cooperate_util.h"
#include "device_profile_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateSM" };
constexpr int32_t INTERVAL_MS { 2000 };
constexpr double PERCENT_CONST { 100.0 };
constexpr float MOUSE_ABS_LOCATION { 100 };
constexpr int32_t MOUSE_ABS_LOCATION_X { 50 };
constexpr int32_t MOUSE_ABS_LOCATION_Y { 50 };
constexpr int32_t COOPERATE_PRIORITY { 499 };
constexpr int32_t MIN_HANDLER_ID { 1 };
constexpr uint32_t P2P_SESSION_CLOSED { 1 };
} // namespace

CooperateSM::CooperateSM() {}
CooperateSM::~CooperateSM()
{
    RemoveMonitor();
    RemoveInterceptor();
}

void CooperateSM::Init()
{
    CALL_INFO_TRACE;
    preparedNetworkId_ = std::make_pair("", "");
    cooperateStates_.emplace(CooperateState::STATE_FREE, std::make_shared<CooperateStateFree>());
    cooperateStates_.emplace(CooperateState::STATE_IN, std::make_shared<CooperateStateIn>());
    cooperateStates_.emplace(CooperateState::STATE_OUT, std::make_shared<CooperateStateOut>());
    auto *context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    context->GetTimerManager().AddTimer(INTERVAL_MS, 1, [this]() {
        this->InitDeviceManager();
        COOR_SOFTBUS_ADAPTER->Init();
    });
    COOR_DEV_MGR->Init();
    runner_ = AppExecFwk::EventRunner::Create(true);
    CHKPL(runner_);
    eventHandler_ = std::make_shared<CooperateEventHandler>(runner_);
}

void CooperateSM::OnSoftbusSessionClosed(const std::string &NetworkId)
{
    CALL_INFO_TRACE;
    CHKPV(eventHandler_);
    std::string taskName = "process_coordinition_reset";
    std::function<void()> handleFunc =
        std::bind(&CooperateSM::OnReset, this, NetworkId);
    eventHandler_->ProxyPostTask(handleFunc, taskName, 0);
}

void CooperateSM::OnReset(const std::string &NetworkId)
{
    CALL_INFO_TRACE;
    Reset(NetworkId);
}

void CooperateSM::OnSessionLost(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    CHKPV(session);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPV(event);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = session;
    COOR_EVENT_MGR->RemoveCooperateEvent(event);
    RemoveMonitor();
    RemoveInterceptor();
    if (currentState_ != CooperateState::STATE_FREE) {
        DeactivateCooperate(COOR_SM->isUnchained_);
    }
    D_INPUT_ADAPTER->UnregisterSessionStateCb();
}

void CooperateSM::Reset(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    bool needReset = true;
    if (currentState_ == CooperateState::STATE_OUT) {
        if (networkId != remoteNetworkId_) {
            needReset = false;
        }
    }
    if (currentState_ == CooperateState::STATE_IN) {
        std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_);
        if (networkId != originNetworkId) {
            needReset = false;
        }
    }
    if (needReset) {
        Reset(true);
        SetPointerVisible();
    }
}

void CooperateSM::Reset(bool adjustAbsolutionLocation)
{
    CALL_INFO_TRACE;
    startDeviceDhid_ = "";
    remoteNetworkId_ = "";
    if (adjustAbsolutionLocation && currentState_ != CooperateState::STATE_FREE) {
        SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
    }
    currentState_ = CooperateState::STATE_FREE;
    isStarting_ = false;
    isStopping_ = false;
    RemoveInterceptor();
}

void CooperateSM::OnCooperateChanged(const std::string &networkId, bool isOpen)
{
    CALL_DEBUG_ENTER;
    CooperateMessage msg = isOpen ? CooperateMessage::PREPARE : CooperateMessage::UNPREPARE;
    auto *context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetTaskScheduler().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, COOR_EVENT_MGR, msg, networkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    if (!isOpen) {
        OnCloseCooperate(networkId, false);
    }
}

void CooperateSM::OnCloseCooperate(const std::string &networkId, bool isLocal)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!preparedNetworkId_.first.empty() && !preparedNetworkId_.second.empty() &&
        ((networkId == preparedNetworkId_.first) || (networkId == preparedNetworkId_.second))) {
        if (currentState_ != CooperateState::STATE_FREE) {
            D_INPUT_ADAPTER->StopRemoteInput(preparedNetworkId_.first, preparedNetworkId_.second,
                COOR_DEV_MGR->GetCooperateDhids(startDeviceDhid_, false), [](bool isSuccess) {
                FI_HILOGI("Failed to stop remote");
            });
        }
        D_INPUT_ADAPTER->UnPrepareRemoteInput(preparedNetworkId_.first, preparedNetworkId_.second,
            [](bool isSuccess) {});
        
    }
    preparedNetworkId_ = std::make_pair("", "");
    if (currentState_ == CooperateState::STATE_FREE) {
        return;
    }
    if (isLocal || (networkId == remoteNetworkId_)) {
        Reset(true);
        SetPointerVisible();
        return;
    }
    if (COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_) == networkId) {
        Reset();
        SetPointerVisible();
    }
}

int32_t CooperateSM::GetCooperateState(const std::string &deviceId)
{
    CALL_INFO_TRACE;
    if (deviceId.empty()) {
        FI_HILOGE("DeviceId is empty");
        return static_cast<int32_t>(CooperateMessage::PARAMETER_ERROR);
    }
    bool state = DP_ADAPTER->GetCrossingSwitchState(deviceId);
    COOR_EVENT_MGR->OnGetCrossingSwitchState(state);
    return RET_OK;
}

void CooperateSM::PrepareCooperate()
{
    CALL_INFO_TRACE;
    if (monitorId_ <= 0) {
        auto monitor = std::make_shared<MonitorConsumer>(
            std::bind(&CooperateSM::UpdateLastPointerEventCallback, this, std::placeholders::_1));
        monitorId_ = MMI::InputManager::GetInstance()->AddMonitor(monitor);
        if (monitorId_ <= 0) {
            FI_HILOGE("Failed to add monitor, error code:%{public}d", monitorId_);
            monitorId_ = -1;
            return;
        }
    }
    DP_ADAPTER->UpdateCrossingSwitchState(true, onlineDevice_);
}

void CooperateSM::UnprepareCooperate()
{
    CALL_INFO_TRACE;
    DP_ADAPTER->UpdateCrossingSwitchState(false, onlineDevice_);
    std::string localNetworkId = COOPERATE::GetLocalNetworkId();
    OnCloseCooperate(localNetworkId, true);
    RemoveMonitor();
    D_INPUT_ADAPTER->UnregisterSessionStateCb();
}

int32_t CooperateSM::ActivateCooperate(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStarting_) {
        FI_HILOGE("In transition state, not process");
        return static_cast<int32_t>(CooperateMessage::COOPERATE_FAIL);
    }
    UpdateMouseLocation();
    if (COOR_SOFTBUS_ADAPTER->OpenInputSoftbus(remoteNetworkId) != RET_OK) {
        FI_HILOGE("Open input softbus failed");
        return static_cast<int32_t>(CooperateMessage::COOPERATE_FAIL);
    }
    isStarting_ = true;
    SetSinkNetworkId(remoteNetworkId);
    auto state = GetCurrentState();
    CHKPR(state, ERROR_NULL_POINTER);
    int32_t ret = state->ActivateCooperate(remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Start remote input failed");
        isStarting_ = false;
        return ret;
    }
    if (currentState_ == CooperateState::STATE_FREE) {
        remoteNetworkId_ = remoteNetworkId;
    }
    return ret;
}

int32_t CooperateSM::DeactivateCooperate(bool isUnchained)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (isStopping_) {
        FI_HILOGE("In transition state, not process");
        return RET_ERR;
    }

    isStopping_ = true;
    std::string stopNetworkId;
    if (currentState_ == CooperateState::STATE_IN) {
        stopNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_);
    } else if (currentState_ == CooperateState::STATE_OUT) {
        stopNetworkId = remoteNetworkId_;
    } else {
        stopNetworkId = sinkNetworkId_;
    }
    isUnchained_ = isUnchained;
    FI_HILOGD("isUnchained_:%{public}d, stopNetworkId:%{public}s",
        isUnchained_, stopNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
    auto state = GetCurrentState();
    CHKPR(state, ERROR_NULL_POINTER);
    int32_t ret = state->DeactivateCooperate(stopNetworkId, isUnchained, preparedNetworkId_);
    if (ret != RET_OK) {
        FI_HILOGE("Stop input device cooperate failed");
        isStopping_ = false;
    }
    CHKPR(notifyDragCancelCallback_, ERROR_NULL_POINTER);
    notifyDragCancelCallback_();
    return ret;
}

void CooperateSM::RegisterNotifyDragCancel(std::function<void(void)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    notifyDragCancelCallback_ = callback;
}

void CooperateSM::StartRemoteCooperate(const std::string &remoteNetworkId, bool buttonIsPressed)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto *context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    COOR_SM->SetSinkNetworkId(remoteNetworkId);
    FI_HILOGD("The remoteNetworkId:%{public}s", remoteNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
    int32_t ret = context->GetTaskScheduler().PostAsyncTask(std::bind(&CooperateEventManager::OnCooperateMessage,
        COOR_EVENT_MGR, CooperateMessage::ACTIVATE, remoteNetworkId));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
    isStarting_ = true;
    if (buttonIsPressed) {
        MMI::InputManager::GetInstance()->EnableInputDevice(true);
        StartPointerEventFilter();
        COOR_SOFTBUS_ADAPTER->NotifyFilterAdded(sinkNetworkId_);
    }
    NotifyRemoteNetworkId(remoteNetworkId);
}

void CooperateSM::StartPointerEventFilter()
{
    CALL_INFO_TRACE;
    int32_t POINTER_DEFAULT_PRIORITY = 220;
    auto filter = std::make_shared<PointerFilter>();
    uint32_t touchTags = CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER);
    FI_HILOGE("touchtags:%{public}d", static_cast<int32_t>(touchTags));
    if (filterId_ >= 0) {
        MMI::InputManager::GetInstance()->RemoveInputEventFilter(filterId_);
    }
    filterId_ =
        MMI::InputManager::GetInstance()->AddInputEventFilter(filter, POINTER_DEFAULT_PRIORITY, touchTags);
    if (filterId_ < 0) {
        FI_HILOGE("Add Event Filter failed");
    }
    filter->UpdateCurrentFilterId(filterId_);
}

void CooperateSM::StartRemoteCooperateResult(bool isSuccess, const std::string &startDeviceDhid, int32_t xPercent,
    int32_t yPercent)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGI("Not in starting");
        return;
    }
    startDeviceDhid_ = startDeviceDhid;
    CooperateMessage msg = isSuccess ? CooperateMessage::ACTIVATE_SUCCESS : CooperateMessage::ACTIVATE_FAIL;
    auto *context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetTaskScheduler().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, COOR_EVENT_MGR, msg, ""));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }

    if (!isSuccess || (currentState_ == CooperateState::STATE_IN)) {
        isStarting_ = false;
        return;
    }
    if (currentState_ == CooperateState::STATE_FREE) {
        NotifyMouseLocation(xPercent, yPercent);
        UpdateState(CooperateState::STATE_IN);
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
        NotifyRemoteNetworkId(COOR_DEV_MGR->GetOriginNetworkId(startDeviceDhid_));
        StateChangedNotify(CooperateState::STATE_FREE, CooperateState::STATE_IN);
#else
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        MMI::InputManager::GetInstance()->SetPointerVisible(true);
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
    }
    if (currentState_ == CooperateState::STATE_OUT) {
        NotifyMouseLocation(xPercent, yPercent);
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
        NotifyRemoteNetworkId(remoteNetworkId_);
        StateChangedNotify(CooperateState::STATE_OUT, CooperateState::STATE_FREE);
#else
        SetAbsolutionLocation(MOUSE_ABS_LOCATION - xPercent, yPercent);
        MMI::InputManager::GetInstance()->SetPointerVisible(true);
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
        UpdateState(CooperateState::STATE_FREE);
    }
    isStarting_ = false;
}

void CooperateSM::StopRemoteCooperate(bool isUnchained)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    isStopping_ = true;
    isUnchained_ = isUnchained;
}

void CooperateSM::StopRemoteCooperateResult(bool isSuccess)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStopping_) {
        FI_HILOGI("Not in stopping");
        return;
    }
    if (isSuccess) {
        Reset(true);
        SetPointerVisible();
    }
    if (!preparedNetworkId_.first.empty() && !preparedNetworkId_.second.empty() && isUnchained_) {
        FI_HILOGI("The sink preparedNetworkId isn't empty, first:%{public}s, second:%{public}s",
            preparedNetworkId_.first.c_str(), preparedNetworkId_.second.c_str());
        bool ret = UnchainCooperate(preparedNetworkId_.first, preparedNetworkId_.second);
        if (ret) {
            COOR_SM->NotifyChainRemoved();
            std::string localNetworkId = COOPERATE::GetLocalNetworkId();
            FI_HILOGD("localNetworkId:%{public}s", localNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
            COOR_SOFTBUS_ADAPTER->NotifyUnchainedResult(localNetworkId, sinkNetworkId_, ret);
        } else {
            FI_HILOGE("Failed to unchain cooperate");
        }
        isUnchained_ = false;
    }
    isStopping_ = false;
}

void CooperateSM::StartCooperateOtherResult(const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    remoteNetworkId_ = remoteNetworkId;
}

void CooperateSM::OnStartFinish(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStarting_) {
        FI_HILOGE("Not in starting");
        return;
    }

    if (!isSuccess) {
        FI_HILOGE("Start distributed failed, startDevice:%{public}d", startDeviceId);
        NotifyRemoteStartFail(remoteNetworkId);
    } else {
        startDeviceDhid_ = COOR_DEV_MGR->GetDhid(startDeviceId);
        if (currentState_ == CooperateState::STATE_FREE) {
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
            NotifyRemoteNetworkId(remoteNetworkId);
            NotifyMouseLocation(mouseLocation_.first, mouseLocation_.second);
            StateChangedNotify(CooperateState::STATE_FREE, CooperateState::STATE_OUT);
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
        } else if (currentState_ == CooperateState::STATE_IN) {
            std::string originNetworkId = COOR_DEV_MGR->GetOriginNetworkId(startDeviceId);
            if (!originNetworkId.empty() && (remoteNetworkId != originNetworkId)) {
                COOR_SOFTBUS_ADAPTER->StartCooperateOtherResult(originNetworkId, remoteNetworkId);
            }
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
            NotifyRemoteNetworkId(originNetworkId);
            NotifyMouseLocation(mouseLocation_.first, mouseLocation_.second);
            StateChangedNotify(CooperateState::STATE_IN, CooperateState::STATE_FREE);
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
            SetPointerVisible();
        }
        NotifyRemoteStartSuccess(remoteNetworkId, startDeviceDhid_);
        if (currentState_ == CooperateState::STATE_FREE) {
            UpdateState(CooperateState::STATE_OUT);
        } else if (currentState_ == CooperateState::STATE_IN) {
            UpdateState(CooperateState::STATE_FREE);
        } else {
            FI_HILOGI("Current state is out");
        }
    }
    isStarting_ = false;
}

void CooperateSM::OnStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!isStopping_) {
        FI_HILOGE("Not in stopping");
        return;
    }
    NotifyRemoteStopFinish(isSuccess, remoteNetworkId);
    if (isSuccess) {
        if (COOR_DEV_MGR->HasLocalPointerDevice()) {
            MMI::InputManager::GetInstance()->SetPointerVisible(true);
            SetAbsolutionLocation(MOUSE_ABS_LOCATION_X, MOUSE_ABS_LOCATION_Y);
        }
        if ((currentState_ == CooperateState::STATE_IN) || (currentState_ == CooperateState::STATE_OUT)) {
            UpdateState(CooperateState::STATE_FREE);
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
            NotifyRemoteNetworkId(remoteNetworkId);
            StateChangedNotify(currentState_, CooperateState::STATE_FREE);
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
        } else {
            FI_HILOGI("Current state is free");
        }
    }
    if (!preparedNetworkId_.first.empty() && !preparedNetworkId_.second.empty() && isUnchained_) {
        FI_HILOGI("The local preparedNetworkId isn't empty, first:%{public}s, second:%{public}s",
            preparedNetworkId_.first.c_str(), preparedNetworkId_.second.c_str());
        bool ret = UnchainCooperate(preparedNetworkId_.first, preparedNetworkId_.second);
        if (ret) {
            COOR_SM->NotifyChainRemoved();
            std::string localNetworkId = COOPERATE::GetLocalNetworkId();
            FI_HILOGD("localNetworkId:%{public}s", localNetworkId.substr(0, SUBSTR_NETWORKID_LEN).c_str());
            COOR_SOFTBUS_ADAPTER->NotifyUnchainedResult(localNetworkId, remoteNetworkId, ret);
        } else {
            FI_HILOGE("Failed to unchain cooperate");
        }
    }
    if (!isUnchained_) {
        COOR_SOFTBUS_ADAPTER->CloseInputSoftbus(remoteNetworkId);
    }
    COOR_SM->SetPointerVisible();
    isUnchained_ = false;
    isStopping_ = false;
}

void CooperateSM::NotifyRemoteStartFail(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    COOR_SOFTBUS_ADAPTER->StartRemoteCooperateResult(remoteNetworkId, false, "", 0, 0);
    COOR_EVENT_MGR->OnStart(CooperateMessage::ACTIVATE_FAIL);
}

void CooperateSM::NotifyRemoteStartSuccess(const std::string &remoteNetworkId, const std::string &startDeviceDhid)
{
    CALL_DEBUG_ENTER;
    COOR_SOFTBUS_ADAPTER->StartRemoteCooperateResult(remoteNetworkId, true, startDeviceDhid, mouseLocation_.first,
        mouseLocation_.second);
    COOR_EVENT_MGR->OnStart(CooperateMessage::ACTIVATE_SUCCESS);
}

void CooperateSM::NotifyRemoteStopFinish(bool isSuccess, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    COOR_SOFTBUS_ADAPTER->StopRemoteCooperateResult(remoteNetworkId, isSuccess);
    if (!isSuccess) {
        COOR_EVENT_MGR->OnStop(CooperateMessage::COOPERATE_FAIL);
    } else {
        COOR_EVENT_MGR->OnStop(CooperateMessage::DEACTIVATE_SUCCESS);
    }
}

bool CooperateSM::UpdateMouseLocation()
{
    CALL_DEBUG_ENTER;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPF(display);
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    if ((width == 0) || (height == 0)) {
        FI_HILOGE("display width or height is 0");
        return false;
    }
    int32_t xPercent = displayX_ * MOUSE_ABS_LOCATION / width;
    int32_t yPercent = displayY_ * MOUSE_ABS_LOCATION / height;
    FI_HILOGI("displayWidth:%{public}d, displayHeight:%{public}d, "
        "physicalX:%{public}d, physicalY:%{public}d,",
        width, height, displayX_, displayY_);
    mouseLocation_ = std::make_pair(xPercent, yPercent);
    return true;
}

bool CooperateSM::UnchainCooperate(const std::string &localNetworkId, const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    int32_t ret = D_INPUT_ADAPTER->UnPrepareRemoteInput(localNetworkId, remoteNetworkId, [](bool isSuccess) {});
    if (ret != RET_OK) {
        FI_HILOGE("Failed to call distributed UnprepareRemoteInput");
        return false;
    }
    preparedNetworkId_ = std::make_pair("", "");
    return true;
}

void CooperateSM::UpdateState(CooperateState state)
{
    FI_HILOGI("state:%{public}d", state);
    currentState_ = state;
    switch (state) {
        case CooperateState::STATE_FREE: {
            Reset();
            MMI::InputManager::GetInstance()->EnableInputDevice(false);
            break;
        }
        case CooperateState::STATE_IN: {
            MMI::InputManager::GetInstance()->SetPointerVisible(false);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            MMI::InputManager::GetInstance()->EnableInputDevice(true);
            auto state = GetCurrentState();
            CHKPV(state);
            state->SetStartDeviceDhid(startDeviceDhid_);
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COOPERATE_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD));
            if (interceptorId_ <= 0) {
                FI_HILOGE("Failed to add interceptor, error code:%{public}d", interceptorId_);
                DeactivateCooperate(isUnchained_);
                return;
            }
            COOR_SOFTBUS_ADAPTER->ConfigTcpAlive();
            preparedNetworkId_ = std::make_pair("", "");
            RegisterSessionCallback();
            break;
        }
        case CooperateState::STATE_OUT: {
            MMI::InputManager::GetInstance()->SetPointerVisible(false);
            auto interceptor = std::make_shared<InterceptorConsumer>();
            interceptorId_ = MMI::InputManager::GetInstance()->AddInterceptor(interceptor, COOPERATE_PRIORITY,
                CapabilityToTags(MMI::INPUT_DEV_CAP_KEYBOARD) | CapabilityToTags(MMI::INPUT_DEV_CAP_POINTER));
            auto state = GetCurrentState();
            CHKPV(state);
            state->SetStartDeviceDhid(startDeviceDhid_);
            if (interceptorId_ <= 0) {
                FI_HILOGE("Failed to add interceptor, error code:%{public}d", interceptorId_);
                DeactivateCooperate(isUnchained_);
                return;
            }
            COOR_SOFTBUS_ADAPTER->ConfigTcpAlive();
            RegisterSessionCallback();
            break;
        }
        default:
            break;
    }
}

CooperateState CooperateSM::GetCurrentCooperateState() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return currentState_;
}

void CooperateSM::UpdatePreparedDevices(const std::string &remoteNetworkId, const std::string &originNetworkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    preparedNetworkId_ = std::make_pair(remoteNetworkId, originNetworkId);
}

std::pair<std::string, std::string> CooperateSM::GetPreparedDevices() const
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    return preparedNetworkId_;
}

bool CooperateSM::IsStarting() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStarting_;
}

bool CooperateSM::IsStopping() const
{
    std::lock_guard<std::mutex> guard(mutex_);
    return isStopping_;
}

void CooperateSM::OnKeyboardOnline(const std::string &dhid)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    auto state = GetCurrentState();
    CHKPV(state);
    state->OnKeyboardOnline(dhid, preparedNetworkId_);
}

void CooperateSM::OnPointerOffline(const std::string &dhid, const std::vector<std::string> &keyboards)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    if (currentState_ == CooperateState::STATE_FREE) {
        FI_HILOGI("Current state:free");
        return;
    }
    if ((currentState_ == CooperateState::STATE_IN) && (startDeviceDhid_ == dhid)) {
        Reset(true);
        SetPointerVisible();
        return;
    }
    if ((currentState_ == CooperateState::STATE_OUT) && (startDeviceDhid_ == dhid)) {
        std::string remoteNetworkId = remoteNetworkId_;
        if (remoteNetworkId.empty()) {
            remoteNetworkId = preparedNetworkId_.first;
        }
        std::string localNetworkId = COOPERATE::GetLocalNetworkId();
        D_INPUT_ADAPTER->StopRemoteInput(remoteNetworkId, localNetworkId, keyboards,
            [this, remoteNetworkId](bool isSuccess) {});
        Reset(true);
        SetPointerVisible();
    }
}

void CooperateSM::OnKeyboardOffline(const std::string &dhid)
{
    CALL_INFO_TRACE;
    if (currentState_ == CooperateState::STATE_OUT) {
        std::string remoteNetworkId = remoteNetworkId_;
        if (remoteNetworkId.empty()) {
            remoteNetworkId = preparedNetworkId_.first;
        }
        std::string localNetworkId = COOPERATE::GetLocalNetworkId();
        std::vector<std::string> inputDeviceDhids;
        inputDeviceDhids.push_back(dhid);
        D_INPUT_ADAPTER->StopRemoteInput(remoteNetworkId, localNetworkId, inputDeviceDhids,
            [this, remoteNetworkId](bool isSuccess) {});
    }
}

bool CooperateSM::InitDeviceManager()
{
    CALL_DEBUG_ENTER;
    initCallback_ = std::make_shared<DeviceInitCallBack>();
    int32_t ret = DIS_HARDWARE.InitDeviceManager(FI_PKG_NAME, initCallback_);
    if (ret != 0) {
        FI_HILOGE("Init device manager failed, ret:%{public}d", ret);
        return false;
    }
    stateCallback_ = std::make_shared<DmDeviceStateCallback>();
    ret = DIS_HARDWARE.RegisterDevStateCallback(FI_PKG_NAME, "", stateCallback_);
    if (ret != 0) {
        FI_HILOGE("Register devStateCallback failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

void CooperateSM::OnDeviceOnline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    std::lock_guard<std::mutex> guard(mutex_);
    onlineDevice_.push_back(networkId);
    DP_ADAPTER->RegisterCrossingStateListener(networkId,
        std::bind(&CooperateSM::OnCooperateChanged, COOR_SM, std::placeholders::_1, std::placeholders::_2));
    COOR_SOFTBUS_ADAPTER->Init();
}

void CooperateSM::OnDeviceOffline(const std::string &networkId)
{
    CALL_INFO_TRACE;
    DP_ADAPTER->UnregisterCrossingStateListener(networkId);
    Reset(networkId);
    preparedNetworkId_ = std::make_pair("", "");
    std::lock_guard<std::mutex> guard(mutex_);
    if (!onlineDevice_.empty()) {
        auto it = std::find(onlineDevice_.begin(), onlineDevice_.end(), networkId);
        if (it != onlineDevice_.end()) {
            onlineDevice_.erase(it);
        }
    }
    if ((currentState_ == CooperateState::STATE_IN) && (sinkNetworkId_ == networkId)) {
        COOR_EVENT_MGR->OnCooperateMessage(CooperateMessage::SESSION_CLOSED);
    }
    if ((currentState_ == CooperateState::STATE_OUT) && (remoteNetworkId_ == networkId)) {
        COOR_EVENT_MGR->OnCooperateMessage(CooperateMessage::SESSION_CLOSED);
    }
}

std::string CooperateSM::GetDeviceCooperateState(CooperateState value) const
{
    std::string state;
    switch (value) {
        case CooperateState::STATE_FREE: {
            state = "free";
            break;
        }
        case CooperateState::STATE_IN: {
            state = "in";
            break;
        }
        case CooperateState::STATE_OUT: {
            state = "out";
            break;
        }
        default: {
            state = "unknown";
            FI_HILOGW("Cooperate status unknown");
            break;
        }
    }
    return state;
}

void CooperateSM::Dump(int32_t fd)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    dprintf(fd, "Cooperate information:\n");
    dprintf(fd,
        "cooperateState:%s | startDeviceDhid:%s | remoteNetworkId:%s | isStarting:%s | isStopping:%s\n"
        "physicalX:%d | physicalY:%d | displayX:%d | displayY:%d | interceptorId:%d | monitorId:%d | filterId:%d\n",
        GetDeviceCooperateState(currentState_).c_str(), startDeviceDhid_.c_str(),
        remoteNetworkId_.substr(0, SUBSTR_NETWORKID_LEN).c_str(), isStarting_ ? "true" : "false",
        isStopping_ ? "true" : "false", mouseLocation_.first, mouseLocation_.second, displayX_,
        displayY_, interceptorId_, monitorId_, filterId_);
    if (onlineDevice_.empty()) {
        dprintf(fd, "onlineDevice:%s\n", "None");
        return;
    }
    for (const auto &item : onlineDevice_) {
        dprintf(fd, "onlineDevice:%s\n", item.c_str());
    }
}

void CooperateSM::UpdateLastPointerEventCallback(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    lastPointerEvent_ = pointerEvent;
}

std::shared_ptr<MMI::PointerEvent> CooperateSM::GetLastPointerEvent() const
{
    return lastPointerEvent_;
}

void CooperateSM::RemoveMonitor()
{
    if ((monitorId_ >= MIN_HANDLER_ID) && (monitorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveMonitor(monitorId_);
        monitorId_ = -1;
    }
}

void CooperateSM::RemoveInterceptor()
{
    if ((interceptorId_ >= MIN_HANDLER_ID) && (interceptorId_ < std::numeric_limits<int32_t>::max())) {
        MMI::InputManager::GetInstance()->RemoveInterceptor(interceptorId_);
        interceptorId_ = -1;
    }
}

bool CooperateSM::IsNeedFilterOut(const std::string &deviceId, const std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CALL_DEBUG_ENTER;
    std::vector<OHOS::MMI::KeyEvent::KeyItem> KeyItems = keyEvent->GetKeyItems();
    std::vector<int32_t> KeyItemsForDInput;
    KeyItemsForDInput.reserve(KeyItems.size());
    for (const auto &item : KeyItems) {
        KeyItemsForDInput.push_back(item.GetKeyCode());
    }
    DistributedHardware::DistributedInput::BusinessEvent businessEvent;
    businessEvent.keyCode = keyEvent->GetKeyCode();
    businessEvent.keyAction = keyEvent->GetKeyAction();
    businessEvent.pressedKeys = KeyItemsForDInput;
    FI_HILOGI("businessEvent.keyCode:%{public}d, keyAction:%{public}d",
        businessEvent.keyCode, businessEvent.keyAction);
    for (const auto &item : businessEvent.pressedKeys) {
        FI_HILOGI("pressedKeys:%{public}d", item);
    }
    return D_INPUT_ADAPTER->IsNeedFilterOut(deviceId, businessEvent);
}

void CooperateSM::DeviceInitCallBack::OnRemoteDied()
{
    CALL_INFO_TRACE;
}

void CooperateSM::DmDeviceStateCallback::OnDeviceOnline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    COOR_SM->OnDeviceOnline(deviceInfo.networkId);
}

void CooperateSM::DmDeviceStateCallback::OnDeviceOffline(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
    COOR_SM->OnDeviceOffline(deviceInfo.networkId);
}

void CooperateSM::DmDeviceStateCallback::OnDeviceChanged(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void CooperateSM::DmDeviceStateCallback::OnDeviceReady(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_INFO_TRACE;
}

void CooperateSM::SetAbsolutionLocation(double xPercent, double yPercent)
{
    CALL_INFO_TRACE;
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPV(display);
    int32_t width = display->GetWidth();
    int32_t height = display->GetHeight();
    int32_t physicalX = static_cast<int32_t>(width * xPercent / PERCENT_CONST);
    int32_t physicalY = static_cast<int32_t>(height * yPercent / PERCENT_CONST);
    FI_HILOGD("width:%{public}d, height:%{public}d, physicalX:%{public}d, physicalY:%{public}d", width, height,
        physicalX, physicalY);
    MMI::InputManager::GetInstance()->SetPointerLocation(physicalX, physicalY);
}

void CooperateSM::OnInterceptorInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CHKPV(eventHandler_);
    CHKPV(keyEvent);
    std::string taskName = "process_interceptor_keyevent";
    std::function<void()> handleFunc =
        std::bind(&CooperateSM::OnPostInterceptorKeyEvent, this, keyEvent);
    eventHandler_->ProxyPostTask(handleFunc, taskName, 0);
}

void CooperateSM::OnInterceptorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CHKPV(eventHandler_);
    CHKPV(pointerEvent);
    std::string taskName = "process_interceptor_pointerevent";
    std::function<void()> handleFunc =
        std::bind(&CooperateSM::OnPostInterceptorPointerEvent, this, pointerEvent);
    eventHandler_->ProxyPostTask(handleFunc, taskName, 0);
}

void CooperateSM::OnMonitorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CHKPV(eventHandler_);
    CHKPV(pointerEvent);
    std::string taskName = "process_monitor_pointerevent";
    std::function<void()> handleFunc =
        std::bind(&CooperateSM::OnPostMonitorInputEvent, this, pointerEvent);
    eventHandler_->ProxyPostTask(handleFunc, taskName, 0);
}

void CooperateSM::OnPostInterceptorKeyEvent(std::shared_ptr<MMI::KeyEvent> keyEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(keyEvent);
    int32_t keyCode = keyEvent->GetKeyCode();
    CooperateState state = GetCurrentCooperateState();
    int32_t deviceId = keyEvent->GetDeviceId();
    if ((keyCode == MMI::KeyEvent::KEYCODE_BACK) || (keyCode == MMI::KeyEvent::KEYCODE_VOLUME_UP) ||
        (keyCode == MMI::KeyEvent::KEYCODE_VOLUME_DOWN) || (keyCode == MMI::KeyEvent::KEYCODE_POWER)) {
        if ((state == CooperateState::STATE_OUT) || (!COOR_DEV_MGR->IsRemote(deviceId))) {
            keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
            MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        }
        return;
    }
    if (state == CooperateState::STATE_IN) {
        if (COOR_DEV_MGR->IsRemote(deviceId)) {
            auto networkId = COOR_DEV_MGR->GetOriginNetworkId(deviceId);
            if (!IsNeedFilterOut(networkId, keyEvent)) {
                keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
                MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
            }
        } else {
            keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
            MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        }
    } else if (state == CooperateState::STATE_OUT) {
        std::string networkId = COOPERATE::GetLocalNetworkId();
        if (IsNeedFilterOut(networkId, keyEvent)) {
            keyEvent->AddFlag(MMI::AxisEvent::EVENT_FLAG_NO_INTERCEPT);
            MMI::InputManager::GetInstance()->SimulateInputEvent(keyEvent);
        }
    }
}

void CooperateSM::OnPostInterceptorPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    if (pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGD("Not mouse event, skip");
        return;
    }
    CooperateState state = GetCurrentCooperateState();
    if (state == CooperateState::STATE_OUT) {
        int32_t deviceId = pointerEvent->GetDeviceId();
        std::string dhid = COOR_DEV_MGR->GetDhid(deviceId);
        if (startDeviceDhid_ != dhid) {
            FI_HILOGI("Move other mouse, stop input device cooperate");
            DeactivateCooperate(isUnchained_);
        }
    }
}

void CooperateSM::OnPostMonitorInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    CHKPV(pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    displayX_ = pointerItem.GetDisplayX();
    displayY_ = pointerItem.GetDisplayY();
    CooperateState state = GetCurrentCooperateState();
    if (state == CooperateState::STATE_IN) {
        int32_t deviceId = pointerEvent->GetDeviceId();
        if (!COOR_DEV_MGR->IsRemote(deviceId)) {
            DeactivateCooperate(isUnchained_);
        }
    }
}

void CooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const
{
    COOR_SM->OnInterceptorInputEvent(keyEvent);
}

void CooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    COOR_SM->OnInterceptorInputEvent(pointerEvent);
}

void CooperateSM::InterceptorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const {}

void CooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const {}

void CooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    CHKPV(pointerEvent);
    if (pointerEvent->GetSourceType() != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGD("Not mouse event, skip");
        return;
    }
    if (callback_) {
        callback_(pointerEvent);
    }
    COOR_SM->OnMonitorInputEvent(pointerEvent);
}

void CooperateSM::MonitorConsumer::OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const {}

void CooperateSM::RegisterStateChange(CooStateChangeType type,
    std::function<void(CooperateState, CooperateState)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    stateChangedCallbacks_[type] = callback;
}

void CooperateSM::RegisterRemoteNetworkId(std::function<void(std::string)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    remoteNetworkIdCallback_ = callback;
}

void CooperateSM::RegisterMouseLocation(std::function<void(int32_t, int32_t)> callback)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback);
    mouseLocationCallback_ = callback;
}

void CooperateSM::StateChangedNotify(CooperateState oldState, CooperateState newState)
{
    CALL_DEBUG_ENTER;
    if ((oldState == CooperateState::STATE_FREE) && (newState == CooperateState::STATE_IN)) {
        ChangeNotify(CooStateChangeType::STATE_FREE_TO_IN, oldState, newState);
        return;
    }
    if ((oldState == CooperateState::STATE_FREE) && (newState == CooperateState::STATE_OUT)) {
        ChangeNotify(CooStateChangeType::STATE_FREE_TO_OUT, oldState, newState);
        return;
    }
    if ((oldState == CooperateState::STATE_IN) && (newState == CooperateState::STATE_FREE)) {
        ChangeNotify(CooStateChangeType::STATE_IN_TO_FREE, oldState, newState);
        return;
    }
    if ((oldState == CooperateState::STATE_OUT) && (newState == CooperateState::STATE_FREE)) {
        ChangeNotify(CooStateChangeType::STATE_OUT_TO_FREE, oldState, newState);
    }
}

void CooperateSM::ChangeNotify(CooStateChangeType type, CooperateState oldState, CooperateState newState)
{
    auto item = stateChangedCallbacks_[type];
    if (item != nullptr) {
        item(oldState, newState);
    }
}

void CooperateSM::NotifyRemoteNetworkId(const std::string &remoteNetworkId)
{
    if (remoteNetworkIdCallback_ != nullptr) {
        remoteNetworkIdCallback_(remoteNetworkId);
    }
}

void CooperateSM::NotifyMouseLocation(int32_t x, int32_t y)
{
    if (mouseLocationCallback_ != nullptr) {
        mouseLocationCallback_(x, y);
    }
}

void CooperateSM::SetUnchainStatus(bool isUnchained)
{
    CALL_DEBUG_ENTER;
    isUnchained_ = isUnchained;
    isStopping_ = false;
}

void CooperateSM::NotifyChainRemoved()
{
    CALL_DEBUG_ENTER;
    CooperateMessage msg = CooperateMessage::SESSION_CLOSED;
    auto *context = COOR_EVENT_MGR->GetIContext();
    CHKPV(context);
    int32_t ret = context->GetTaskScheduler().PostAsyncTask(
        std::bind(&CooperateEventManager::OnCooperateMessage, COOR_EVENT_MGR, msg, ""));
    if (ret != RET_OK) {
        FI_HILOGE("Posting async task failed");
    }
}

void CooperateSM::NotifyUnchainedResult(const std::string &remoteNetworkId, bool isSuccess)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("Notify unchained result, isSuccess:%{public}d", isSuccess);
    if (isSuccess) {
        COOR_SM->NotifyChainRemoved();
    }
    isUnchained_ = false;
    isStopping_ = false;
    preparedNetworkId_ = std::make_pair("", "");
    COOR_SOFTBUS_ADAPTER->CloseInputSoftbus(remoteNetworkId);
}

void CooperateSM::SetSinkNetworkId(const std::string &sinkNetworkId)
{
    CALL_DEBUG_ENTER;
    sinkNetworkId_ = sinkNetworkId;
}

void CooperateSM::SetPointerVisible()
{
    bool hasPointer = COOR_DEV_MGR->HasLocalPointerDevice();
    FI_HILOGD("hasPointer:%{public}s", hasPointer ? "true" : "false");
    MMI::InputManager::GetInstance()->SetPointerVisible(hasPointer);
}

std::shared_ptr<ICooperateState> CooperateSM::GetCurrentState()
{
    auto it = cooperateStates_.find(currentState_);
    if (it == cooperateStates_.end()) {
        FI_HILOGE("currentState_ not found");
        return nullptr;
    }
    return it->second;
}

void CooperateSM::RegisterSessionCallback()
{
    CALL_DEBUG_ENTER;
    D_INPUT_ADAPTER->RegisterSessionStateCb([this](uint32_t status) {
        FI_HILOGI("Recv session callback status:%{public}u", status);
        if (status == P2P_SESSION_CLOSED) {
            preparedNetworkId_ = std::pair("", "");
            COOR_EVENT_MGR->OnCooperateMessage(CooperateMessage::SESSION_CLOSED);
            Reset();
        }
    });
}

bool PointerFilter::OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const
{
    FI_HILOGD("PointerFilter OnInputEvent enter");
    CHKPF(pointerEvent);
    if (pointerEvent->GetPointerAction() == MMI::PointerEvent::POINTER_ACTION_BUTTON_DOWN) {
        FI_HILOGI("Current event is down");
        auto *context = COOR_EVENT_MGR->GetIContext();
        CHKPF(context);
        int32_t ret = context->GetTaskScheduler().PostAsyncTask(
            std::bind(&MMI::InputManager::RemoveInputEventFilter, MMI::InputManager::GetInstance(), filterId_));
        if (ret != RET_OK) {
            FI_HILOGE("Posting async task failed");
        }
        filterId_ = -1;
        return true;
    }
    return false;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
