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

#include "cooperate_client.h"

#include "cooperate_params.h"
#include "default_params.h"
#include "devicestatus_define.h"
#include "devicestatus_func_callback.h"

#undef LOG_TAG
#define LOG_TAG "CooperateClient"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

int32_t CooperateClient::RegisterListener(ITunnelClient &tunnel,
    CooperateListenerPtr listener, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCooperateListener_) {
        if (item == listener) {
            FI_HILOGE("The listener already exists");
            return RET_ERR;
        }
    }
    if (!isListeningProcess_) {
        FI_HILOGI("Start monitoring");
        DefaultParam param;
        DefaultReply reply;

        int32_t ret = tunnel.AddWatch(Intention::COOPERATE, CooperateRequestID::REGISTER_LISTENER, param, reply);
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register, ret:%{public}d", ret);
            return ret;
        }
        isListeningProcess_ = true;
    }
    devCooperateListener_.push_back(listener);
    return RET_OK;
}

int32_t CooperateClient::UnregisterListener(ITunnelClient &tunnel,
    CooperateListenerPtr listener, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        devCooperateListener_.clear();
        goto listenerLabel;
    }
    for (auto it = devCooperateListener_.begin(); it != devCooperateListener_.end(); ++it) {
        if (*it == listener) {
            devCooperateListener_.erase(it);
            goto listenerLabel;
        }
    }

listenerLabel:
    if (isListeningProcess_ && devCooperateListener_.empty()) {
        isListeningProcess_ = false;
        DefaultParam param;
        DefaultReply reply;
        return tunnel.RemoveWatch(Intention::COOPERATE, CooperateRequestID::UNREGISTER_LISTENER, param, reply);
    }
    return RET_OK;
}

int32_t CooperateClient::Enable(ITunnelClient &tunnel,
    CooperateMessageCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    DefaultParam param { GenerateRequestID() };
    DefaultReply reply;

    int32_t ret = tunnel.Enable(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Prepare cooperate failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
    return RET_OK;
}

int32_t CooperateClient::Disable(ITunnelClient &tunnel,
    CooperateMessageCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    DefaultParam param { GenerateRequestID() };
    DefaultReply reply;

    int32_t ret = tunnel.Disable(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Unprepare cooperate failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
    return RET_OK;
}

int32_t CooperateClient::Start(ITunnelClient &tunnel, const std::string &remoteNetworkId,
    int32_t startDeviceId, CooperateMessageCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    StartCooperateParam param { GenerateRequestID(), remoteNetworkId, startDeviceId, isCheckPermission };
    DefaultReply reply;

    int32_t ret = tunnel.Start(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Activate cooperate failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
    return RET_OK;
}

int32_t CooperateClient::Stop(ITunnelClient &tunnel,
    bool isUnchained, CooperateMessageCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    StopCooperateParam param { GenerateRequestID(), isUnchained, isCheckPermission };
    DefaultReply reply;

    int32_t ret = tunnel.Stop(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate cooperate failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
    return RET_OK;
}

int32_t CooperateClient::GetCooperateState(ITunnelClient &tunnel,
    const std::string &networkId, CooperateStateCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    GetCooperateStateParam param { GenerateRequestID(), networkId, isCheckPermission };
    DefaultReply reply;

    int32_t ret = tunnel.GetParam(Intention::COOPERATE, CooperateRequestID::GET_COOPERATE_STATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Get cooperate state failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
    return RET_OK;
}

int32_t CooperateClient::GetCooperateState(ITunnelClient &tunnel, const std::string &udId, bool &state)
{
    return RET_ERR;
}

int32_t CooperateClient::RegisterEventListener(ITunnelClient &tunnel,
    const std::string &networkId, std::shared_ptr<IEventListener> listener)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    if (eventListener_.find(networkId) != eventListener_.end()) {
        FI_HILOGE("The listener already exists");
        return RET_ERR;
    }
    RegisterEventListenerParam param { networkId };
    DefaultReply reply;
    if (int32_t ret = tunnel.AddWatch(Intention::COOPERATE, CooperateRequestID::REGISTER_EVENT_LISTENER, param, reply);
        ret != RET_OK) {
        FI_HILOGE("RegisterEventListener failed, ret:%{public}d", ret);
        return ret;
    }
    eventListener_.emplace(networkId, listener);
    FI_HILOGI("RegisterEventListener for networkId:%{public}s successfully", networkId);
    return RET_OK;
}

int32_t CooperateClient::UnregisterEventListener(ITunnelClient &tunnel,
    const std::string &networkId, std::shared_ptr<IEventListener> listener = nullptr)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    if (eventListener_.find(networkId) == eventListener_.end()) {
        FI_HILOGE("No listener registered");
        return RET_ERR;
    }
    UnregisterEventListenerParam param { networkId };
    DefaultReply reply;
    if (int32_t ret = tunnel.RemoveWatch(Intention::COOPERATE,
        CooperateRequestID::UNREGISTER_EVENT_LISTENER, param, reply); ret != RET_OK) {
        FI_HILOGE("UnregisterEventListener failed, ret:%{public}d", ret);
        return ret;
    }
    eventListener_.erase(networkId);
    return RET_OK;
}

int32_t CooperateClient::AddHotAreaListener(ITunnelClient &tunnel, HotAreaListenerPtr listener)
{
    return RET_ERR;
}

int32_t CooperateClient::RemoveHotAreaListener(ITunnelClient &tunnel, HotAreaListenerPtr listener)
{
    return RET_ERR;
}

int32_t CooperateClient::GenerateRequestID()
{
    static int32_t requestId { 0 };

    if (requestId == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("Request ID exceeds the maximum");
        requestId = 0;
    }
    return requestId++;
}

int32_t CooperateClient::OnCoordinationListener(const StreamClient &client, NetPacket &pkt)
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
    OnDevCooperateListener(networkId, CoordinationMessage(nType));
    return RET_OK;
}

void CooperateClient::OnDevCooperateListener(const std::string &networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCooperateListener_) {
        item->OnCoordinationMessage(networkId, msg);
    }
}

int32_t CooperateClient::OnCoordinationMessage(const StreamClient &client, NetPacket &pkt)
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
    OnCooperateMessageEvent(userData, networkId, CoordinationMessage(nType));
    return RET_OK;
}

void CooperateClient::OnCooperateMessageEvent(int32_t userData,
    const std::string &networkId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto iter = devCooperateEvent_.find(userData);
    if (iter == devCooperateEvent_.end()) {
        return;
    }
    CooperateMessageCallback callback = iter->second.msgCb;
    CHKPV(callback);
    callback(networkId, msg);
    devCooperateEvent_.erase(iter);
}

int32_t CooperateClient::OnCoordinationState(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    bool state = false;

    pkt >> userData >> state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read coordination msg failed");
        return RET_ERR;
    }
    OnCooperateStateEvent(userData, state);
    return RET_OK;
}

void CooperateClient::OnCooperateStateEvent(int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto iter = devCooperateEvent_.find(userData);
    if (iter == devCooperateEvent_.end()) {
        return;
    }
    CooperateStateCallback event = iter->second.stateCb;
    CHKPV(event);
    event(state);
    devCooperateEvent_.erase(iter);
    FI_HILOGD("Coordination state event callback, userData:%{public}d, state:(%{public}d)", userData, state);
}

int32_t CooperateClient::OnHotAreaListener(const StreamClient &client, NetPacket &pkt)
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

void CooperateClient::OnDevHotAreaListener(int32_t displayX,
    int32_t displayY, HotAreaType type, bool isEdge)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devHotAreaListener_) {
        item->OnHotAreaMessage(displayX, displayY, type, isEdge);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
