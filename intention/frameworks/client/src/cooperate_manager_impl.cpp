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

#include "cooperate_manager_impl.h"

#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateManagerImpl" };
} // namespace
int32_t CooperateManagerImpl::RegisterCooperateListener(CooperateListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    CHKPR(listener, RET_ERR);
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCooperateListener_) {
        if (item == listener) {
            FI_HILOGW("The listener already exists");
            return RET_ERR;
        }
    }
    if (!isListeningProcess_) {
        FI_HILOGI("Start monitoring");
        DefaultCooperateParam param;
        DefaultCooperateReply reply;

        int32_t ret = IntentionClient::GetInstance().AddWatch(Intention::COOPERATE, CooperateParam::REGISTER, param, reply);
        if (ret != RET_OK) {
            FI_HILOGE("Failed to register, ret:%{public}d", ret);
            return ret;
        }
        isListeningProcess_ = true;
    }
    devCooperateListener_.push_back(listener);
    return RET_OK;
}

int32_t CooperateManagerImpl::UnregisterCooperateListener(CooperateListenerPtr listener)
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
        DefaultCooperateParam param;
        DefaultCooperateReply reply;
        return IntentionClient::GetInstance().RemoveWatch(Intention::COOPERATE, CooperateParam::REGISTER, param, reply);
    }
    return RET_OK;
}

int32_t CooperateManagerImpl::PrepareCooperate(FuncCooperateMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event;
    event.msg = callback;
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    DefaultCooperateParam param { userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().AddWatch(Intention::COOPERATE, CooperateParam::PREPARE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Prepare cooperate failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

int32_t CooperateManagerImpl::UnprepareCooperate(FuncCooperateMessage callback)
{
    CALL_DEBUG_ENTER;
    CooperateEvent event;
    event.msg = callback;
    std::lock_guard<std::mutex> guard(mtx_);
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    DefaultCooperateParam param { userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().RemoveWatch(Intention::COOPERATE, CooperateParam::PREPARE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Unprepare cooperate failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

int32_t CooperateManagerImpl::ActivateCooperate(const std::string &remoteNetworkId,
    int32_t startDeviceId, FuncCooperateMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event;
    event.msg = callback;
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    StartCooperateParam param { userData_, remoteNetworkId, startDeviceId };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().Start(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Activate cooperate failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

int32_t CooperateManagerImpl::DeactivateCooperate(bool isUnchained, FuncCooperateMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event;
    event.msg = callback;
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    StartCooperateParam param { userData_, isUnchained };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().Start(Intention::COOPERATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate cooperate failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

int32_t CooperateManagerImpl::GetCooperateState(
    const std::string &deviceId, FuncCooperateState callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event;
    event.state = callback;
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    GetCooperateStateParam param { deviceId, userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().GetParam(Intention::COOPERATE, CooperateParam::STATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Get cooperate state failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

void CooperateManagerImpl::OnDevCooperateListener(const std::string deviceId, CooperateMessage msg)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCooperateListener_) {
        item->OnCooperateMessage(deviceId, msg);
    }
}

void CooperateManagerImpl::OnCooperateMessageEvent(int32_t userData,
    const std::string deviceId, CooperateMessage msg)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto iter = devCooperateEvent_.find(userData);
    if (iter == devCooperateEvent_.end()) {
        return;
    }
    CooperateMsg event = iter->second.msg;
    CHKPV(event);
    event(deviceId, msg);
    devCooperateEvent_.erase(iter);
}

void CooperateManagerImpl::OnCooperateStateEvent(int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto iter = devCooperateEvent_.find(userData);
    if (iter == devCooperateEvent_.end()) {
        return;
    }
    CooperateState event = iter->second.state;
    CHKPV(event);
    event(state);
    devCooperateEvent_.erase(iter);
    FI_HILOGD("cooperate state event callback, userData:%{public}d, state:(%{public}d)", userData, state);
}

int32_t CooperateManagerImpl::GetUserData() const
{
    std::lock_guard<std::mutex> guard(mtx_);
    return userData_;
}

int32_t CooperateManagerImpl::OnCooperateListener(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    std::string deviceId;
    int32_t nType = 0;
    pkt >> userData >> deviceId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read type failed");
        return RET_ERR;
    }
    OnDevCooperateListener(deviceId, CooperateMessage(nType));
    return RET_OK;
}

int32_t CooperateManagerImpl::OnCooperateMessage(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    std::string deviceId;
    int32_t nType = 0;
    pkt >> userData >> deviceId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read cooperate msg failed");
        return RET_ERR;
    }
    OnCooperateMessageEvent(userData, deviceId, CooperateMessage(nType));
    return RET_OK;
}

int32_t CooperateManagerImpl::OnCooperateState(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData = 0;
    bool state = false;

    pkt >> userData >> state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read cooperate msg failed");
        return RET_ERR;
    }
    OnCooperateStateEvent(userData, state);
    return RET_OK;
}

const CooperateManagerImpl::CooperateMsg *CooperateManagerImpl::GetCooperateMessageEvent(
    int32_t userData) const
{
    auto iter = devCooperateEvent_.find(userData);
    return iter == devCooperateEvent_.end() ? nullptr : &iter->second.msg;
}

const CooperateManagerImpl::CooperateState *CooperateManagerImpl::GetCooperateStateEvent(
    int32_t userData) const
{
    auto iter = devCooperateEvent_.find(userData);
    return iter == devCooperateEvent_.end() ? nullptr : &iter->second.state;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS