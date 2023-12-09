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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CooperateClient" };
} // namespace

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

        int32_t ret = tunnel.AddWatch(Intention::COOPERATE, CooperateAction::REGISTER_LISTENER, param, reply);
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
        return tunnel.RemoveWatch(Intention::COOPERATE, CooperateAction::UNREGISTER_LISTENER, param, reply);
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

    int32_t ret = tunnel.GetParam(Intention::COOPERATE, CooperateAction::GET_COOPERATE_STATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Get cooperate state failed");
        return ret;
    }
    devCooperateEvent_.insert_or_assign(param.userData, event);
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
    return RET_ERR;
}

int32_t CooperateClient::OnCoordinationMessage(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}

int32_t CooperateClient::OnCoordinationState(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}

int32_t CooperateClient::OnHotAreaListener(const StreamClient &client, NetPacket &pkt)
{
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
