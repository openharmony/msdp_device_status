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

#include "coordination_manager_impl.h"

#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "include/util.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationManagerImpl"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

int32_t CoordinationManagerImpl::RegisterCoordinationListener(CoordinationListenerPtr listener, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCoordinationListener_) {
        if (item == listener) {
            FI_HILOGW("The listener already exists");
            return RET_ERR;
        }
    }
    if (!isListeningProcess_) {
        FI_HILOGI("Start monitoring");
        int32_t ret = DeviceStatusClient::GetInstance().RegisterCoordinationListener(isCompatible);
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register, ret:%{public}d", ret);
            return ret;
        }
        isListeningProcess_ = true;
    }
    devCoordinationListener_.push_back(listener);
    return RET_OK;
}

int32_t CoordinationManagerImpl::UnregisterCoordinationListener(
    CoordinationListenerPtr listener, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        devCoordinationListener_.clear();
        goto LISTENER_LABEL;
    }
    for (auto it = devCoordinationListener_.begin(); it != devCoordinationListener_.end(); ++it) {
        if (*it == listener) {
            devCoordinationListener_.erase(it);
            goto LISTENER_LABEL;
        }
    }

LISTENER_LABEL:
    if (isListeningProcess_ && devCoordinationListener_.empty()) {
        isListeningProcess_ = false;
        return DeviceStatusClient::GetInstance().UnregisterCoordinationListener(isCompatible);
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::PrepareCoordination(FuncCoordinationMessage callback, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    prepareCooCallback_ = callback;
    isPrepareCooIsCompatible_ = isCompatible;
    SetMessageCallback(CallbackMessageId::PREPARE, callback);
    int32_t userData = static_cast<int32_t> (CallbackMessageId::PREPARE);
    int32_t ret = DeviceStatusClient::GetInstance().PrepareCoordination(userData, isCompatible);
    if (ret != RET_OK) {
        FI_HILOGE("Prepare coordination failed");
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::UnprepareCoordination(FuncCoordinationMessage callback, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    SetMessageCallback(CallbackMessageId::UNPREPARE, callback);
    int32_t userData = static_cast<int32_t> (CallbackMessageId::UNPREPARE);
    int32_t ret = DeviceStatusClient::GetInstance().UnprepareCoordination(userData, isCompatible);
    if (ret != RET_OK) {
        FI_HILOGE("Unprepare coordination failed");
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::ActivateCoordination(const std::string &remoteNetworkId,
    int32_t startDeviceId, FuncCoordinationMessage callback, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    SetMessageCallback(CallbackMessageId::ACTIVATE, callback);
    int32_t userData = static_cast<int32_t> (CallbackMessageId::ACTIVATE);
    int32_t ret = DeviceStatusClient::GetInstance().ActivateCoordination(
        userData, remoteNetworkId, startDeviceId, isCompatible);
    if (ret != RET_OK) {
        FI_HILOGE("Activate coordination failed");
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::DeactivateCoordination(bool isUnchained, FuncCoordinationMessage callback,
    bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    SetMessageCallback(CallbackMessageId::DEACTIVATE, callback);
    int32_t userData = static_cast<int32_t> (CallbackMessageId::DEACTIVATE);
    int32_t ret = DeviceStatusClient::GetInstance().DeactivateCoordination(userData, isUnchained, isCompatible);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate coordination failed");
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::GetCoordinationState(
    const std::string &networkId, FuncCoordinationState callback, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    SetStateCallback(CallbackMessageId::GET_COORDINATION, callback);
    int32_t userData = static_cast<int32_t> (CallbackMessageId::GET_COORDINATION);
    int32_t ret = DeviceStatusClient::GetInstance().GetCoordinationState(userData, networkId, isCompatible);
    if (ret != RET_OK) {
        FI_HILOGE("Get coordination state failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::GetCoordinationState(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    int32_t ret = DeviceStatusClient::GetInstance().GetCoordinationState(udId, state);
    if (ret != RET_OK) {
        FI_HILOGE("Get coordination state failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

void CoordinationManagerImpl::OnDevCoordinationListener(const std::string networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCoordinationListener_) {
        item->OnCoordinationMessage(networkId, msg);
    }
}

void CoordinationManagerImpl::OnCoordinationMessageEvent(int32_t userData,
    const std::string networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    if (devCoordinationEvent_.find(userData) == devCoordinationEvent_.end()) {
        FI_HILOGE("No callback for CallbackMessageId:%{public}d", userData);
        return;
    }
    CoordinationMsg event = devCoordinationEvent_[userData].msg;
    CHKPV(event);
    event(networkId, msg);
    FI_HILOGD("Coordination msg event callback, userData:%{public}d, networkId:%{public}s",
        userData, GetAnonyString(networkId).c_str());
}

void CoordinationManagerImpl::OnCoordinationStateEvent(int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    if (devCoordinationEvent_.find(userData) == devCoordinationEvent_.end()) {
        FI_HILOGE("No callback for CallbackMessageId:%{public}d", userData);
        return;
    }
    CoordinationState event = devCoordinationEvent_[userData].state;
    CHKPV(event);
    event(state);
    FI_HILOGD("Coordination state event callback, userData:%{public}d", userData);
}

void CoordinationManagerImpl::SetMessageCallback(CallbackMessageId id, FuncCoordinationMessage callback)
{
    CoordinationEvent event;
    event.msg = callback;
    devCoordinationEvent_[static_cast<int32_t>(id)] = event;
    FI_HILOGD("SetMessageCallback for CallbackMessageId:%{public}d", id);
}

void CoordinationManagerImpl::SetStateCallback(CallbackMessageId id, FuncCoordinationState callback)
{
    CoordinationEvent event;
    event.state = callback;
    devCoordinationEvent_[static_cast<int32_t>(id)] = event;
    FI_HILOGD("SetStateCallback for CallbackMessageId:%{public}d", id);
}

int32_t CoordinationManagerImpl::OnCoordinationListener(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    std::string networkId;
    int32_t nType = 0;
    pkt >> userData >> networkId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read type failed");
        return RET_ERR;
    }
    FI_HILOGI("UserData:%{public}d, networkId:%{public}s, nType:%{public}d",
        userData, GetAnonyString(networkId).c_str(), nType);
    OnDevCoordinationListener(networkId, CoordinationMessage(nType));
    return RET_OK;
}

int32_t CoordinationManagerImpl::OnCoordinationMessage(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    std::string networkId;
    int32_t nType = 0;
    pkt >> userData >> networkId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read coordination msg failed");
        return RET_ERR;
    }
    FI_HILOGI("UserData:%{public}d, networkId:%{public}s, nType:%{public}d",
        userData, GetAnonyString(networkId).c_str(), nType);
    OnCoordinationMessageEvent(userData, networkId, CoordinationMessage(nType));
    return RET_OK;
}

int32_t CoordinationManagerImpl::OnCoordinationState(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    bool state = false;
    pkt >> userData >> state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read coordination msg failed");
        return RET_ERR;
    }
    FI_HILOGI("UserData:%{public}d, state:%{public}s", userData, (state ? "true" : "false"));
    OnCoordinationStateEvent(userData, state);
    return RET_OK;
}

int32_t CoordinationManagerImpl::AddHotAreaListener(HotAreaListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devHotAreaListener_) {
        if (item == listener) {
            FI_HILOGW("The listener already exists");
            return RET_ERR;
        }
    }
    if (!isHotAreaListener_) {
        FI_HILOGI("Start monitoring");
        int32_t ret = DeviceStatusClient::GetInstance().AddHotAreaListener();
        if (ret != RET_OK) {
            FI_HILOGE("Failed to add hot area listener, ret:%{public}d", ret);
            return ret;
        }
        isHotAreaListener_ = true;
    }
    devHotAreaListener_.push_back(listener);
    return RET_OK;
}

void CoordinationManagerImpl::OnDevHotAreaListener(int32_t displayX,
    int32_t displayY, HotAreaType type, bool isEdge)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devHotAreaListener_) {
        item->OnHotAreaMessage(displayX, displayY, type, isEdge);
    }
}

int32_t CoordinationManagerImpl::OnHotAreaListener(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    int32_t positionX = 0;
    int32_t positionY = 0;
    int32_t type = 0;
    bool isEdge = false;
    pkt >> positionX >> positionY >> type >> isEdge;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read type failed");
        return RET_ERR;
    }
    OnDevHotAreaListener(positionX, positionY, HotAreaType(type), isEdge);
    return RET_OK;
}

int32_t CoordinationManagerImpl::RemoveHotAreaListener(HotAreaListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        devHotAreaListener_.clear();
    } else {
        devHotAreaListener_.erase(std::remove_if(devHotAreaListener_.begin(), devHotAreaListener_.end(),
            [listener] (auto items) {
                return items == listener;
            })
        );
    }

    if (isHotAreaListener_ && devHotAreaListener_.empty()) {
        isHotAreaListener_ = false;
        return DeviceStatusClient::GetInstance().RemoveHotAreaListener();
    }
    return RET_OK;
}

void CoordinationManagerImpl::OnConnected()
{
    CALL_INFO_TRACE;
    CHKPV(prepareCooCallback_);
    if (PrepareCoordination(prepareCooCallback_, isPrepareCooIsCompatible_) != RET_OK) {
        FI_HILOGE("PrepareCoordination failed");
    }
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS