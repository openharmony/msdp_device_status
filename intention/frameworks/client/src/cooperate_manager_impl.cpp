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

#include "devicestatus_define.h"
#include "intention_client.h"
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
            FI_HILOGE("The listener already exists");
            return RET_ERR;
        }
    }
    if (!isListeningProcess_) {
        FI_HILOGI("Start monitoring");
        DefaultCooperateParam param;
        DefaultCooperateReply reply;

        int32_t ret = IntentionClient::GetInstance().AddWatch(static_cast<uint32_t>(Intention::COOPERATE),
            CooperateParam::REGISTER, param, reply);
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
        return IntentionClient::GetInstance().RemoveWatch(static_cast<uint32_t>(Intention::COOPERATE),
            CooperateParam::REGISTER, param, reply);
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
        FI_HILOGD("userData exceeds the maximum");
        userData_ = 0;
    }
    DefaultCooperateParam param { userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().AddWatch(static_cast<uint32_t>(Intention::COOPERATE),
        CooperateParam::PREPARE, param, reply);
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
        userData_ = 0;
    }
    DefaultCooperateParam param { userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().RemoveWatch(static_cast<uint32_t>(Intention::COOPERATE),
        CooperateParam::PREPARE, param, reply);
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
        userData_ = 0;
    }
    StartCooperateParam param { userData_, remoteNetworkId, startDeviceId };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().Start(static_cast<uint32_t>(Intention::COOPERATE), param, reply);
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
        userData_ = 0;
    }
    StopCooperateParam param { userData_, isUnchained };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().Start(static_cast<uint32_t>(Intention::COOPERATE), param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate cooperate failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}

int32_t CooperateManagerImpl::GetCooperateState(const std::string &deviceId, FuncCooperateState callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CooperateEvent event;
    event.state = callback;
    if (userData_ == std::numeric_limits<int32_t>::max()) {
        FI_HILOGE("userData exceeds the maximum");
        userData_ = 0;
    }
    GetCooperateStateParam param { deviceId, userData_ };
    DefaultCooperateReply reply;
    int32_t ret = IntentionClient::GetInstance().GetParam(static_cast<uint32_t>(Intention::COOPERATE),
        CooperateParam::STATE, param, reply);
    if (ret != RET_OK) {
        FI_HILOGE("Get cooperate state failed");
        return ret;
    }
    devCooperateEvent_[userData_] = event;
    userData_++;
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS