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

#include "cooperate_context.h"

#include <algorithm>

#include "display_manager.h"
#include "xcollie/watchdog.h"

#include "ddm_adapter.h"
#include "ddp_adapter.h"
#include "devicestatus_define.h"
#include "dsoftbus_handler.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "CooperateContext"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
const std::string COOPERATE_SWITCH { "currentStatus" };
const std::string THREAD_NAME { "os_Cooperate_EventHandler" };
constexpr double PERCENT { 100.0 };
const uint64_t WATCHDOG_TIMWVAL { 5000 };
} // namespace

class BoardObserver final : public IBoardObserver {
public:
    explicit BoardObserver(Channel<CooperateEvent>::Sender sender) : sender_(sender) {}
    ~BoardObserver() = default;
    DISALLOW_COPY_AND_MOVE(BoardObserver);

    void OnBoardOnline(const std::string &networkId) override
    {
        FI_HILOGD("\'%{public}s\' is online", Utility::Anonymize(networkId).c_str());
        sender_.Send(CooperateEvent(
            CooperateEventType::DDM_BOARD_ONLINE,
            DDMBoardOnlineEvent {
                .networkId = networkId
            }));
    }

    void OnBoardOffline(const std::string &networkId) override
    {
        FI_HILOGD("\'%{public}s\' is offline", Utility::Anonymize(networkId).c_str());
        sender_.Send(CooperateEvent(
            CooperateEventType::DDM_BOARD_OFFLINE,
            DDMBoardOfflineEvent {
                .networkId = networkId
            }));
    }

private:
    Channel<CooperateEvent>::Sender sender_;
};

class DeviceProfileObserver final : public IDeviceProfileObserver {
public:
    DeviceProfileObserver(IContext *env, Channel<CooperateEvent>::Sender sender)
        : env_(env), sender_(sender) {}

    ~DeviceProfileObserver() = default;
    DISALLOW_COPY_AND_MOVE(DeviceProfileObserver);

    void OnProfileChanged(const std::string &networkId) override;

private:
    IContext *env_ { nullptr };
    Channel<CooperateEvent>::Sender sender_;
};

void DeviceProfileObserver::OnProfileChanged(const std::string &networkId)
{
    FI_HILOGI("Profile of \'%{public}s\' has changed", Utility::Anonymize(networkId).c_str());
    bool switchStatus = false;

    auto udId = env_->GetDP().GetUdIdByNetworkId(networkId);
    int32_t ret = env_->GetDP().GetCrossingSwitchState(udId, switchStatus);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to query switch status of \'%{public}s\'", Utility::Anonymize(networkId).c_str());
        return;
    }
    FI_HILOGI("Profile of \'%{public}s\', switch status:%{public}d",
        Utility::Anonymize(networkId).c_str(), switchStatus);
    sender_.Send(CooperateEvent(
        CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        DDPCooperateSwitchChanged {
            .networkId = networkId,
            .normal = switchStatus,
        }));
}

class HotplugObserver final : public IDeviceObserver {
public:
    explicit HotplugObserver(Channel<CooperateEvent>::Sender sender) : sender_(sender) {}
    ~HotplugObserver() = default;

    void OnDeviceAdded(std::shared_ptr<IDevice> dev) override;
    void OnDeviceRemoved(std::shared_ptr<IDevice> dev) override;

private:
    Channel<CooperateEvent>::Sender sender_;
};

void HotplugObserver::OnDeviceAdded(std::shared_ptr<IDevice> dev)
{
    CHKPV(dev);
    sender_.Send(CooperateEvent(
        CooperateEventType::INPUT_HOTPLUG_EVENT,
        InputHotplugEvent {
            .deviceId = dev->GetId(),
            .type = InputHotplugType::PLUG,
        }));
}

void HotplugObserver::OnDeviceRemoved(std::shared_ptr<IDevice> dev)
{
    CHKPV(dev);
    sender_.Send(CooperateEvent(
        CooperateEventType::INPUT_HOTPLUG_EVENT,
        InputHotplugEvent {
            .deviceId = dev->GetId(),
            .type = InputHotplugType::UNPLUG,
        }));
}

Context::Context(IContext *env)
    : dsoftbus_(env), eventMgr_(env), hotArea_(env), mouseLocation_(env),
      inputEventBuilder_(env), inputEventInterceptor_(env), env_(env)
{}

IDDPAdapter& Context::GetDP() const
{
    return env_->GetDP();
}

void Context::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    sender_ = sender;
    dsoftbus_.AttachSender(sender);
}

void Context::AddObserver(std::shared_ptr<ICooperateObserver> observer)
{
    CHKPV(observer);
    observers_.insert(observer);
}

void Context::RemoveObserver(std::shared_ptr<ICooperateObserver> observer)
{
    observers_.erase(observer);
}

void Context::Enable()
{
    CALL_DEBUG_ENTER;
    StartEventHandler();
    EnableDDM();
    EnableDDP();
    EnableDevMgr();
}

void Context::Disable()
{
    CALL_DEBUG_ENTER;
    DisableDevMgr();
    DisableDDP();
    DisableDDM();
    StopEventHandler();
}

int32_t Context::StartEventHandler()
{
    auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
    CHKPR(runner, RET_ERR);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    int ret = HiviewDFX::Watchdog::GetInstance().AddThread("os_Cooperate_EventHandler", eventHandler_,
        WATCHDOG_TIMWVAL);
    if (ret != 0) {
        FI_HILOGW("add watch dog failed");
    }
    return RET_OK;
}

void Context::StopEventHandler()
{
    eventHandler_.reset();
}

int32_t Context::EnableDDM()
{
    boardObserver_ = std::make_shared<BoardObserver>(sender_);
    ddm_.AddBoardObserver(boardObserver_);
    return ddm_.Enable();
}

void Context::DisableDDM()
{
    ddm_.Disable();
    ddm_.RemoveBoardObserver(boardObserver_);
    boardObserver_.reset();
}

int32_t Context::EnableDDP()
{
    dpObserver_ = std::make_shared<DeviceProfileObserver>(env_, sender_);
    env_->GetDP().AddObserver(dpObserver_);
    return RET_OK;
}

void Context::DisableDDP()
{
    env_->GetDP().RemoveObserver(dpObserver_);
    dpObserver_.reset();
}

int32_t Context::EnableDevMgr()
{
    hotplugObserver_ = std::make_shared<HotplugObserver>(sender_);
    env_->GetDeviceManager().AddDeviceObserver(hotplugObserver_);
    return RET_OK;
}

void Context::DisableDevMgr()
{
    env_->GetDeviceManager().RemoveDeviceObserver(hotplugObserver_);
    hotplugObserver_.reset();
}

NormalizedCoordinate Context::NormalizedCursorPosition() const
{
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    if (display == nullptr) {
        FI_HILOGE("No default display");
        return cursorPos_;
    }
    Rectangle displayRect {
        .width = display->GetWidth(),
        .height = display->GetHeight(),
    };
    if ((displayRect.width <= 0) || (displayRect.height <= 0)) {
        FI_HILOGE("Invalid display information");
        return cursorPos_;
    }
    return NormalizedCoordinate {
        .x = static_cast<int32_t>((cursorPos_.x + 1) * PERCENT / displayRect.width),
        .y = static_cast<int32_t>((cursorPos_.y + 1) * PERCENT / displayRect.height),
    };
}

void Context::EnableCooperate(const EnableCooperateEvent &event)
{
    int32_t ret = env_->GetDP().UpdateCrossingSwitchState(true);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to update switch status");
    }
}

void Context::DisableCooperate(const DisableCooperateEvent &event)
{
    int32_t ret = env_->GetDP().UpdateCrossingSwitchState(false);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to update switch status");
    }
}

void Context::StartCooperate(const StartCooperateEvent &event)
{
    remoteNetworkId_ = event.remoteNetworkId;
    startDeviceId_ = event.startDeviceId;
}

void Context::OnPointerEvent(const InputPointerEvent &event)
{
    if ((event.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        ((event.pointerAction == MMI::PointerEvent::POINTER_ACTION_MOVE) ||
         (event.pointerAction == MMI::PointerEvent::POINTER_ACTION_PULL_MOVE))) {
        cursorPos_ = event.position;
    }
}

void Context::RemoteStartSuccess(const DSoftbusStartCooperateFinished &event)
{
    remoteNetworkId_ = event.originNetworkId;
    SetCursorPosition(event.cursorPos);
}

void Context::RelayCooperate(const DSoftbusRelayCooperate &event)
{
    remoteNetworkId_ = event.targetNetworkId;
}

bool Context::IsAllowCooperate()
{
    FI_HILOGI("Notify observers of allow cooperate");
    return std::all_of(observers_.cbegin(), observers_.cend(), [](const auto &observer) {
        return observer->IsAllowCooperate();
    });
}

void Context::OnTransitionOut()
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of transition out");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer, remoteNetworkId = Peer(), cursorPos = NormalizedCursorPosition()] {
                FI_HILOGI("Notify one observer of transition out");
                CHKPV(observer);
                observer->OnTransitionOut(remoteNetworkId, cursorPos);
            });
    }
}

void Context::OnTransitionIn()
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of transition in");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer, remoteNetworkId = Peer(), cursorPos = NormalizedCursorPosition()] {
                FI_HILOGI("Notify one observer of transition in");
                CHKPV(observer);
                observer->OnTransitionIn(remoteNetworkId, cursorPos);
            });
    }
}

void Context::OnBack()
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of come back");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer, remoteNetworkId = Peer(), cursorPos = NormalizedCursorPosition()] {
                FI_HILOGI("Notify one observer of come back");
                CHKPV(observer);
                observer->OnBack(remoteNetworkId, cursorPos);
            });
    }
}

void Context::OnRelay(const std::string &networkId)
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of relay cooperation");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer, remoteNetworkId = Peer(), cursorPos = NormalizedCursorPosition()] {
                FI_HILOGI("Notify one observer of relay cooperation");
                CHKPV(observer);
                observer->OnRelay(remoteNetworkId, cursorPos);
            });
    }
}

void Context::CloseDistributedFileConnection(const std::string &remoteNetworkId)
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of device offline");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer, remoteNetworkId = Peer()] {
                FI_HILOGI("Notify one observer of device offline, remoteNetworkId:%{public}s",
                    Utility::Anonymize(remoteNetworkId).c_str());
                CHKPV(observer);
                observer->CloseDistributedFileConnection(remoteNetworkId);
            });
    }
}

void Context::OnResetCooperation()
{
    CHKPV(eventHandler_);
    FI_HILOGI("Notify observers of reset cooperation");
    for (const auto &observer : observers_) {
        eventHandler_->PostTask(
            [observer] {
                FI_HILOGI("Notify one observer of reset cooperation");
                CHKPV(observer);
                observer->OnReset();
            });
    }
}

void Context::SetCursorPosition(const Coordinate &cursorPos)
{
    double xPercent = (PERCENT - std::clamp<double>(cursorPos.x, 0.0, PERCENT)) / PERCENT;
    double yPercent = std::clamp<double>(cursorPos.y, 0.0, PERCENT) / PERCENT;

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPV(display);
    cursorPos_.x = static_cast<int32_t>(xPercent * display->GetWidth());
    cursorPos_.y = static_cast<int32_t>(yPercent * display->GetHeight());
    env_->GetInput().SetPointerLocation(cursorPos_.x, cursorPos_.y);
    FI_HILOGI("Set cursor position (%{public}d,%{public}d)(%{public}d,%{public}d)(%{public}d,%{public}d)",
        cursorPos.x, cursorPos.y, cursorPos_.x, cursorPos_.y, display->GetWidth(), display->GetHeight());
}

void Context::UpdateCursorPosition()
{
    env_->GetInput().SetPointerLocation(cursorPos_.x, cursorPos_.y);
    FI_HILOGI("Update cursor position (%{public}d,%{public}d)", cursorPos_.x, cursorPos_.y);
}

void Context::ResetCursorPosition()
{
    constexpr Coordinate defaultCursorPos {
        .x = 50,
        .y = 50,
    };
    SetCursorPosition(defaultCursorPos);
}

#ifdef ENABLE_PERFORMANCE_CHECK
void Context::StartTrace(const std::string &name)
{
    std::lock_guard guard { lock_ };
    if (traces_.find(name) != traces_.end()) {
        return;
    }
    traces_.emplace(name, std::chrono::steady_clock::now());
    FI_HILOGI("[PERF] Start tracing \'%{public}s\'", name.c_str());
}

void Context::FinishTrace(const std::string &name)
{
    std::lock_guard guard { lock_ };
    if (auto iter = traces_.find(name); iter != traces_.end()) {
        FI_HILOGI("[PERF] Finish tracing \'%{public}s\', elapsed:%{public}lld ms", name.c_str(),
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - iter->second).count());
        traces_.erase(iter);
    }
}
#endif // ENABLE_PERFORMANCE_CHECK

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
