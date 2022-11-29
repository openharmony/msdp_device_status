/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "devicestatus_define.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationManagerImpl" };
} // namespace

CoordinationManagerImpl &CoordinationManagerImpl::GetInstance()
{
    static CoordinationManagerImpl instance;
    return instance;
}

int32_t CoordinationManagerImpl::RegisterCoordinationListener(CoordinationListenerPtr listener)
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
    devCoordinationListener_.push_back(listener);
    if (!isListeningProcess_) {
        FI_HILOGI("Start monitoring");
        isListeningProcess_ = true;
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::UnregisterCoordinationListener(CoordinationListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    if (listener == nullptr) {
        devCoordinationListener_.clear();
        goto listenerLabel;
    }
    for (auto it = devCoordinationListener_.begin(); it != devCoordinationListener_.end(); ++it) {
        if (*it == listener) {
            devCoordinationListener_.erase(it);
            goto listenerLabel;
        }
    }

listenerLabel:
    if (isListeningProcess_ && devCoordinationListener_.empty()) {
        isListeningProcess_ = false;
        return RET_ERR;
    }
    return RET_OK;
}

std::optional<int32_t> CoordinationManagerImpl::AddCoordinationUserData(FuncCoordinationMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationEvent event;
    event.msg = callback;
    if (userData_ == INT32_MAX || userData_ < 0) {
        FI_HILOGE("User data Exception");
        return std::nullopt;
    }
    devCoordinationEvent_[userData_] = event;
    return userData_++;
}

void CoordinationManagerImpl::OnDevCoordinationListener(const std::string deviceId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    for (const auto &item : devCoordinationListener_) {
        item->OnCoordinationMessage(deviceId, msg);
    }
}

void CoordinationManagerImpl::OnCoordinationMessageEvent(int32_t userData,
    const std::string deviceId, CoordinationMessage msg)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto event = GetCoordinationMessageEvent(userData);
    CHKPV(event);
    (*event)(deviceId, msg);
}

void CoordinationManagerImpl::OnCoordinationState(int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    auto event = GetCoordinationStateEvent(userData);
    CHKPV(event);
    (*event)(state);
    FI_HILOGD("Coordination state event callback userData:%{public}d state:(%{public}d)", userData, state);
}

int32_t CoordinationManagerImpl::GetUserData()
{
    std::lock_guard<std::mutex> guard(mtx_);
    return userData_;
}


bool CoordinationManagerImpl::InitClient(EventHandlerPtr eventHandler)
{
    CALL_DEBUG_ENTER;
    if (client_ != nullptr) {
        if (eventHandler != nullptr) {
            client_->MarkIsEventHandlerChanged(eventHandler);
        }
        return true;
    }
    client_ = std::make_shared<MMIClient>();
    client_->SetEventHandler(eventHandler);
    client_->RegisterConnectedFunction(&OnConnected);
    if (!(client_->Start())) {
        client_.reset();
        client_ = nullptr;
        MMI_HILOGE("The client fails to start");
        return false;
    }
    return true;
}


const CoordinationManagerImpl::CoordinationMsg *CoordinationManagerImpl::GetCoordinationMessageEvent(
    int32_t userData) const
{
    auto iter = devCoordinationEvent_.find(userData);
    return iter == devCoordinationEvent_.end() ? nullptr : &iter->second.msg;
}

const CoordinationManagerImpl::CoordinationState *CoordinationManagerImpl::GetCoordinationStateEvent(
    int32_t userData) const
{
    auto iter = devCoordinationEvent_.find(userData);
    return iter == devCoordinationEvent_.end() ? nullptr : &iter->second.state;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS