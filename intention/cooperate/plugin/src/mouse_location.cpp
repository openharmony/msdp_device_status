/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "mouse_location.h"

#include "devicestatus_define.h"
#include "dsoftbus_handler.h"
#include "utility.h"

#undef LOG_TAG
#define LOG_TAG "MouseLocation"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

void MouseLocation::AddListener(const RegisterEventListenerEvent &event)
{
    if (auto localNetworkId = env_->GetDP().GetLocalNetworkId(); localNetworkId == event.networkId) {
        FI_HILOGI("Add local mouse location listener");
        localNetworkId_ = event.networkId;
        localListeners_.insert(event.pid);
        return;
    }
    DSoftbusSubscribeMouseLocation softbusEvent;
    softbusEvent.networkId = event.networkId;
    SubscribeMouseLocation(event.networkId, softbusEvent);
    listeners_[event.networkId].insert(event.pid);
}

void MouseLocation::RemoveListener(const UnregisterEventListenerEvent &event)
{
    if (auto localNetworkId = env_->GetDP().GetLocalNetworkId(); localNetworkId == event.networkId) {
        FI_HILOGI("Remove local mouse location listener");
        localListeners_.erase(event.pid);
        return;
    }
    DSoftbusSubscribeMouseLocation softbusEvent;
    softbusEvent.networkId = event.networkId;
    UnSubscribeMouseLocation(event.networkId, softbusEvent);
    if (listeners_.find(event.networkId) != listeners_.end()) {
        FI_HILOGE("No listener for networkId:%{public}s", Utility::Anonymize(event.networkId));
        return;
    }
    listeners_.erase(event.networkId);
}

void MouseLocation::OnSubscribeMouseLocation(const DSoftbusSubscribeMouseLocation &notice)
{
    remoteSubscribers_.insert(notice.networkId);
    FI_HILOGI("Subscriber for networkId:%{public}s successfully", Utility::Anonymize(notice.networkId));
    DSoftbusRelaySubscribeMouseLocation event = {
        .networkId = notice.remoteNetworkId,
        .remoteNetworkId = notice.networkId,
        .result = true,
    };
    RelaySubscribeMouseLocation(notice.networkId, event);
}

void MouseLocation::OnUnSubscribeMouseLocation(const DSoftbusUnSubscribeMouseLocation &notice)
{
    if (remoteSubscribers_.find(notice.networkId) == remoteSubscribers_.end()) {
        FI_HILOGE("No subscriber for networkId:%{public}s stored in remote subscriber",
            Utility::Anonymize(notice.networkId));
        return;
    }
    remoteSubscribers_.erase(notice.networkId);
    DSoftbusRelayUnSubscribeMouseLocation event = {
        .networkId = notice.remoteNetworkId,
        .remoteNetworkId = notice.networkId,
        .result = true,
    };
    RelaySubscribeMouseLocation(notice.networkId, event);
}

void MouseLocation::OnRelaySubscribeMouseLocation(const DSoftbusRelaySubscribeMouseLocation &notice)
{
    if (!notice.result) {
        FI_HILOGE("SubscribeMouseLocation failed, networkId:%{public}s, remoteNetworkId:%{public}s",
            Utility::Anonymize(notice.networkId), Utility::Anonymize(notice.remoteNetworkId));
        return;
    }
    FI_HILOGI("SubscribeMouseLocation successfully, networkId:%{public}s, remoteNetworkId:%{public}s",
        Utility::Anonymize(notice.networkId), Utility::Anonymize(notice.remoteNetworkId));
}

void MouseLocation::OnRelayUnSubscribeMouseLocation(const DSoftbusRelayUnSubscribeMouseLocation &notice)
{
    if (!notice.result) {
        FI_HILOGE("UnSubscribeMouseLocation failed, networkId:%{public}s, remoteNetworkId:%{public}s",
            Utility::Anonymize(notice.networkId), Utility::Anonymize(notice.remoteNetworkId));
        return;
    }
    FI_HILOGI("UnSubscribeMouseLocation successfully, networkId:%{public}s, remoteNetworkId:%{public}s",
        Utility::Anonymize(notice.networkId), Utility::Anonymize(notice.remoteNetworkId));
}

void MouseLocation::OnRemoteMouseLocation(const DSoftbusSyncMouseLocation &notice)
{
    CALL_DEBUG_ENTER;
    if (listeners_.find(notice.networkId) == listeners_.end()) {
        FI_HILOGE("No listener stored in listeners");
        return;
    }
    LocationInfo locationInfo {
        .displayX = notice.mouseLocation.displayX,
        .displayY = notice.mouseLocation.displayY,
        .displayWidth = notice.mouseLocation.displayWidth,
        .displayHeight = notice.mouseLocation.displayHeight
        };
    for (const auto pid : listeners_[notice.networkId]) {
        ReportMouseLocationToListener(notice.networkId, locationInfo, pid);
    }
}

void MouseLocation::ProcessData(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    LocationInfo locationInfo;
    TransferToLocationInfo(pointerEvent, locationInfo);
    if (HasLocalListener()) {
        for (const auto pid : localListeners_) {
            ReportMouseLocationToListener(localNetworkId_, locationInfo, pid);
        }
    }
    if (!HasRemoteSubscriber()) {
        FI_HILOGD("No remote subscriber");
        return;
    }
    for (const auto &networkId : remoteSubscribers_) {
        SyncLocationToRemote(networkId, locationInfo);
    }
}

void MouseLocation::SyncLocationToRemote(const std::string &networkId, const LocationInfo &locationInfo)
{
    DSoftbusSyncMouseLocation softbusEvent;
    softbusEvent.networkId = networkId;
    softbusEvent.remoteNetworkId = "";
    softbusEvent.mouseLocation = {
        .displayX = locationInfo.displayX,
        .displayY = locationInfo.displayY,
        .displayWidth = locationInfo.displayWidth,
        .displayHeight = locationInfo.displayHeight
    };
    SyncMouseLocation(softbusEvent.networkId, softbusEvent);
}

int32_t MouseLocation::RelaySubscribeMouseLocation(const std::string &networkId,
    const DSoftbusRelaySubscribeMouseLocation &event)
{
    CALL_INFO_TRACE;
    NetPacket packet(MessageId::DSOFTBUS_RELAY_SUBSCRIBE_MOUSE_LOCATION);
    packet << event.networkId << event.remoteNetworkId << event.result;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    int32_t ret = env_->GetDSoftbus().SendPacket(networkId, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to SendPacket");
    }
    return ret;
}

int32_t MouseLocation::RelayUnSubscribeMouseLocation(const std::string &networkId,
    const DSoftbusRelayUnSubscribeMouseLocation &event)
{
    CALL_INFO_TRACE;
    NetPacket packet(MessageId::DSOFTBUS_RELAY_UNSUBSCRIBE_MOUSE_LOCATION);
    packet << event.networkId << event.remoteNetworkId << event.result;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    int32_t ret = env_->GetDSoftbus().SendPacket(networkId, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to SendPacket");
    }
    return ret;
}

int32_t MouseLocation::SubscribeMouseLocation(const std::string &networkId, const DSoftbusSubscribeMouseLocation &event)
{
    CALL_INFO_TRACE;
    NetPacket packet(MessageId::DSOFTBUS_SUBSCRIBE_MOUSE_LOCATION);
    packet << event.networkId << event.remoteNetworkId;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    int32_t ret = env_->GetDSoftbus().SendPacket(networkId, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to SendPacket");
    }
    return ret;
}

int32_t MouseLocation::UnSubscribeMouseLocation(const std::string &networkId,
    const DSoftbusUnSubscribeMouseLocation &event)
{
    CALL_INFO_TRACE;
    NetPacket packet(MessageId::DSOFTBUS_UNSUBSCRIBE_MOUSE_LOCATION);
    packet << event.networkId << event.remoteNetworkId;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    int32_t ret = env_->GetDSoftbus().SendPacket(networkId, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to SendPacket");
    }
    return ret;
}

int32_t MouseLocation::SyncMouseLocation(const std::string &networkId, const DSoftbusSyncMouseLocation &event)
{
    CALL_INFO_TRACE;
    NetPacket packet(MessageId::DSOFTBUS_SUBSCRIBE_MOUSE_LOCATION);
    packet << event.networkId << event.remoteNetworkId << event.mouseLocation.displayX <<
        event.mouseLocation.displayY << event.mouseLocation.displayWidth << event.mouseLocation.displayHeight;
    if (packet.ChkRWError()) {
        FI_HILOGE("Failed to write data packet");
        return RET_ERR;
    }
    int32_t ret = env_->GetDSoftbus().SendPacket(networkId, packet);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to SendPacket");
    }
    return ret;
}

void MouseLocation::ReportMouseLocationToListener(const std::string &networkId, const LocationInfo &locationInfo,
    int32_t pid)
{
    auto session = env_->GetSocketSessionManager().FindSessionByPid(pid);
    CHKPV(session);
    NetPacket pkt(MessageId::MOUSE_LOCATION_ADD_LISTENER);
    pkt << networkId << locationInfo.displayX << locationInfo.displayY <<
        locationInfo.displayWidth << locationInfo.displayHeight;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!session->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}

void MouseLocation::TransferToLocationInfo(std::shared_ptr<MMI::PointerEvent> pointerEvent, LocationInfo &locationInfo)
{
    CHKPV(pointerEvent);
    MMI::PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        FI_HILOGE("Corrupted pointer event");
        return;
    }
    auto display = Rosen::DisplayManager::GetInstance().GetDefaultDisplay();
    CHKPV(display);
    locationInfo = {
        .displayX = pointerItem.GetDisplayX(),
        .displayY = pointerItem.GetDisplayY(),
        .displayWidth = display->GetWidth(),
        .displayHeight = display->GetHeight(),
    };
}

bool MouseLocation::HasRemoteSubscriber()
{
    return !remoteSubscribers_.empty();
}

bool MouseLocation::HasLocalListener()
{
    return !localListeners_.empty();
}

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
