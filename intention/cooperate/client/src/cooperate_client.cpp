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

#ifdef ENABLE_PERFORMANCE_CHECK
#include <algorithm>
#include <numeric>
#endif // ENABLE_PERFORMANCE_CHECK

#include "cooperate_params.h"
#include "default_params.h"
#include "devicestatus_define.h"
#include "devicestatus_func_callback.h"
#include "utility.h"

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
#ifdef ENABLE_PERFORMANCE_CHECK
    DumpPerformaceInfo();
#endif // ENABLE_PERFORMANCE_CHECK
    return RET_OK;
}

int32_t CooperateClient::Start(ITunnelClient &tunnel, const std::string &remoteNetworkId,
    int32_t startDeviceId, CooperateMessageCallback callback, bool isCheckPermission)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event { callback };
    auto userData = GenerateRequestID();
#ifdef ENABLE_PERFORMANCE_CHECK
    StartTrace(userData);
#endif // ENABLE_PERFORMANCE_CHECK
    StartCooperateParam param { userData, remoteNetworkId, startDeviceId, isCheckPermission };
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
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    GetCooperateStateSyncParam param { udId };
    BoolenReply reply;
    if (tunnel.GetParam(Intention::COOPERATE, CooperateRequestID::GET_COOPERATE_STATE_SYNC, param, reply) != RET_OK) {
        FI_HILOGE("Get cooperate state failed udId: %{public}s", Utility::Anonymize(udId));
        return RET_ERR;
    }
    FI_HILOGI("GetCooperateState for udId: %{public}s successfully,state: %{public}s",
        Utility::Anonymize(udId), state ? "true" : "false");
    state = reply.state;
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
#ifdef ENABLE_PERFORMANCE_CHECK
    if (CoordinationMessage(nType) == CoordinationMessage::ACTIVATE_SUCCESS) {
        FinishTrace(userData);
    }
#endif // ENABLE_PERFORMANCE_CHECK
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

#ifdef ENABLE_PERFORMANCE_CHECK
void CooperateClient::StartTrace(int32_t userData)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard { performanceLock_ };
    performanceInfo_.traces_.emplace(userData, std::chrono::steady_clock::now());
    performanceInfo_.activateNum += 1;
    FI_HILOGI("[PERF] Start tracing \'%{public}d\'", userData);
}

void CooperateClient::FinishTrace(int32_t userData)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard { performanceLock_ };
    int32_t hundred = { 100 };
    if (auto iter = performanceInfo_.traces_.find(userData); iter != performanceInfo_.traces_.end()) {
        auto curDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - iter->second).count();
        FI_HILOGI("[PERF] Finish tracing \'%{public}d\', elapsed: %{public}lld ms", userData, curDuration);
        performanceInfo_.traces_.erase(iter);
        performanceInfo_.successNum += 1;
        performanceInfo_.failNum = performanceInfo_.traces_.size();
        performanceInfo_.successRate = (performanceInfo_.successNum * hundred) / performanceInfo_.activateNum;
        performanceInfo_.minDuration = std::min(static_cast<int32_t> (curDuration), performanceInfo_.minDuration);
        performanceInfo_.maxDuration = std::max(static_cast<int32_t> (curDuration), performanceInfo_.maxDuration);
        performanceInfo_.durationList.push_back(curDuration);
    } else {
        FI_HILOGW("[PERF] Finish tracing with something wrong");
    }
}

void CooperateClient::DumpPerformaceInfo()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard { performanceLock_ };
    int32_t sumDuration = std::accumulate(
        performanceInfo_.durationList.begin(), performanceInfo_.durationList.end(), 0);
    performanceInfo_.averageDuration = sumDuration / performanceInfo_.durationList.size();
    FI_HILOGI("[PERF] performanceInfo:"
        "activateNum: %{public}d successNum: %{public}d failNum: %{public}d successRate: %{public}d "
        "averageDuration: %{public}d maxDuration: %{public}d minDuration: %{public}d ",
        performanceInfo_.activateNum, performanceInfo_.successNum, performanceInfo_.failNum,
        performanceInfo_.successRate, performanceInfo_.averageDuration, performanceInfo_.maxDuration,
        performanceInfo_.minDuration);
    std::string durationStr;
    for (const auto &duraion : performaceInfo_.durationList) {
        durationStr += std::to_string(duraion) + ", ";
    }
    FI_HILOGI("[PERF] Duration: %{public}s", durationStr.c_str());
}
#endif // ENABLE_PERFORMANCE_CHECK
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
