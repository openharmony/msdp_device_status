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
#include <string_view>

#include "ddm_adapter.h"
#include "ddp_adapter.h"
#include "devicestatus_define.h"
#include "display_manager.h"
#include "dsoftbus_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateContext" };
const std::string COOPERATE_SWITCH { "COOPERATE_SWITCH" };
} // namespace

class BoardObserver final : public IBoardObserver {
public:
    explicit BoardObserver(Channel<CooperateEvent>::Sender sender) : sender_(sender) {}
    ~BoardObserver() = default;
    DISALLOW_COPY_AND_MOVE(BoardObserver);

    void OnBoardOnline(const std::string &networkId) override
    {
        sender_.Send(CooperateEvent(
            CooperateEventType::DDM_BOARD_ONLINE,
            DDMBoardOnlineEvent {
                .networkId = networkId
            }));
    }

    void OnBoardOffline(const std::string &networkId) override
    {
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
    DeviceProfileObserver(std::shared_ptr<IDDPAdapter> ddp, Channel<CooperateEvent>::Sender sender)
        : ddp_(ddp), sender_(sender) {}

    ~DeviceProfileObserver() = default;
    DISALLOW_COPY_AND_MOVE(DeviceProfileObserver);

    void OnProfileChanged(const std::string &networkId) override;

private:
    std::weak_ptr<IDDPAdapter> ddp_;
    Channel<CooperateEvent>::Sender sender_;
};

void DeviceProfileObserver::OnProfileChanged(const std::string &networkId)
{
    std::shared_ptr<IDDPAdapter> ddp = ddp_.lock();
    CHKPV(ddp);
    bool switchStatus = false;

    int32_t ret = ddp->GetProperty(networkId, COOPERATE_SWITCH, switchStatus);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to query switch status of \'%{public}s\'", networkId.c_str());
        return;
    }
    sender_.Send(CooperateEvent(
        CooperateEventType::DDP_COOPERATE_SWITCH_CHANGED,
        DDPCooperateSwitchChanged {
            .networkId = networkId,
            .status = switchStatus,
        }));
}

Context::Context(IContext *env)
    : eventMgr_(env)
{
    devMgr_ = std::make_shared<InputDeviceManager>(env);
    ddm_ = std::make_shared<DDMAdapter>();
    ddp_ = std::make_shared<DDPAdapter>();
    dsoftbus_ = std::make_shared<DSoftbusAdapter>();
}

void Context::Enable()
{
    CALL_DEBUG_ENTER;
    EnableDevMgr();
    EnableDSoftbus();
    EnableDDM();
    EnableDDP();
}

void Context::Disable()
{
    CALL_DEBUG_ENTER;
    DisableDDP();
    DisableDDM();
    DisableDSoftbus();
    DisableDevMgr();
}

int32_t Context::EnableDevMgr()
{
    devMgr_->AttachSender(sender_);
    devMgr_->Enable();
    return RET_OK;
}

void Context::DisableDevMgr()
{
    devMgr_->Disable();
}

int32_t Context::EnableDSoftbus()
{
    dsoftbus_->AttachSender(sender_);
    return dsoftbus_->Enable();
}

void Context::DisableDSoftbus()
{
    dsoftbus_->Disable();
}

int32_t Context::EnableDDM()
{
    boardObserver_ = std::make_shared<BoardObserver>(sender_);
    ddm_->AddBoardObserver(boardObserver_);
    return ddm_->Enable();
}

void Context::DisableDDM()
{
    ddm_->Disable();
    ddm_->RemoveBoardObserver(boardObserver_);
    boardObserver_.reset();
}

int32_t Context::EnableDDP()
{
    dpObserver_ = std::make_shared<DeviceProfileObserver>(ddp_, sender_);
    ddp_->AddObserver(dpObserver_);
    return RET_OK;
}

void Context::DisableDDP()
{
    ddp_->RemoveObserver(dpObserver_);
    dpObserver_.reset();
}

void Context::StartCooperate(const StartCooperateEvent &event)
{
    remoteNetworkId_ = event.remoteNetworkId;
    startDeviceId_ = event.startDeviceId;
    startDeviceDhid_ = devMgr_->GetDhid(startDeviceId_);
    originNetworkId_ = devMgr_->GetOriginNetworkId(startDeviceId_);
}

void Context::RemoteStart(const DSoftbusStartCooperate &event)
{
    remoteNetworkId_ = event.networkId;
}

void Context::RemoteStartSuccess(const DSoftbusStartCooperateFinished &event)
{
    startDeviceDhid_ = event.startDeviceDhid;
    originNetworkId_ = devMgr_->GetOriginNetworkId(startDeviceDhid_);
    SetCursorPosition(event);
}

void Context::SetCursorPosition(const DSoftbusStartCooperateFinished &event)
{
    constexpr double PERCENT { 100.0 };
    double xPercent = (PERCENT - std::clamp<double>(event.cursorPos.x, 0.0, PERCENT)) / PERCENT;
    double yPercent = std::clamp<double>(event.cursorPos.y, 0.0, PERCENT) / PERCENT;

    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPV(display);
    cursorPos_.x = static_cast<int32_t>(xPercent * display->GetWidth());
    cursorPos_.y = static_cast<int32_t>(yPercent * display->GetHeight());
    MMI::InputManager::GetInstance()->SetPointerLocation(cursorPos_.x, cursorPos_.y);
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
