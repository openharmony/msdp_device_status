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

#include "dsoftbus_handler.h"

#include "devicestatus_define.h"
#include "dsoftbus_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DSoftbusHandler" };
}

DSoftbusHandler::DSoftbusHandler()
{
    dsoftbus_ = std::make_unique<DSoftbusAdapter>();
    observer_ = std::make_shared<DSoftbusObserver>(*this);
    CHKPL(dsoftbus_);
    dsoftbus_->AddObserver(observer_);
}

DSoftbusHandler::~DSoftbusHandler()
{
    CHKPL(dsoftbus_);
    dsoftbus_->RemoveObserver(observer_);
}

void DSoftbusHandler::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    sender_ = sender;
}

int32_t DSoftbusHandler::Enable()
{
    CALL_DEBUG_ENTER;
    CHKPR(dsoftbus_, RET_ERR);
    return dsoftbus_->Enable();
}

void DSoftbusHandler::Disable()
{
    CALL_DEBUG_ENTER;
    CHKPV(dsoftbus_);
    dsoftbus_->Disable();
}

int32_t DSoftbusHandler::OpenSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    CHKPR(dsoftbus_, RET_ERR);
    return dsoftbus_->OpenSession(networkId);
}

void DSoftbusHandler::CloseSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    CHKPV(dsoftbus_);
    dsoftbus_->CloseSession(networkId);
}

int32_t DSoftbusHandler::StartCooperate(const std::string &networkId, const DSoftbusStartCooperate &event)
{
    NetPacket packet(MessageId::START_REMOTE_COOPERATE);
    packet << event.networkId;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    CHKPR(dsoftbus_, RET_ERR);
    return dsoftbus_->SendPacket(networkId, packet);
}

int32_t DSoftbusHandler::StartCooperateResponse(const std::string &networkId,
    const DSoftbusStartCooperateResponse &event)
{
    return RET_ERR;
}

int32_t DSoftbusHandler::StartCooperateFinish(const std::string &networkId,
    const DSoftbusStartCooperateFinished &event)
{
    return RET_ERR;
}

std::string DSoftbusHandler::GetLocalNetworkId()
{
    return IDSoftbusAdapter::GetLocalNetworkId();
}

void DSoftbusHandler::OnBind(const std::string &networkId)
{}

void DSoftbusHandler::OnShutdown(const std::string &networkId)
{
    SendEvent(CooperateEvent(
        CooperateEventType::DSOFTBUS_SESSION_CLOSED,
        DSoftbusSessionClosed {
            .networkId = networkId
        }));
}

void DSoftbusHandler::OnPacket(const std::string &networkId, NetPacket &packet)
{
    switch (packet.GetMsgId()) {
        case MessageId::START_REMOTE_COOPERATE: {
            OnStartCooperate(packet);
            break;
        }
        default: {
            break;
        }
    }
}

void DSoftbusHandler::SendEvent(const CooperateEvent &event)
{
    std::lock_guard guard(lock_);
    sender_.Send(event);
}

void DSoftbusHandler::OnStartCooperate(NetPacket &packet)
{
    DSoftbusStartCooperate event;
    packet >> event.networkId;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to read from data packet");
        return;
    }
    SendEvent(CooperateEvent(
        CooperateEventType::DSOFTBUS_START_COOPERATE,
        event));
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
