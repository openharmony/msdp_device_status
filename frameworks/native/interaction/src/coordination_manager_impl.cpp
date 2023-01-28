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

#include "devicestatus_client.h"
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
        return DeviceStatusClient::GetInstance().RegisterCoordinationListener();
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
        return DeviceStatusClient::GetInstance().UnregisterCoordinationListener();
    }
    return RET_OK;
}

int32_t CoordinationManagerImpl::EnableCoordination(bool enabled, FuncCoordinationMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationEvent event;
    event.msg = callback;
    if (userData_ == INT32_MAX) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    auto ret = DeviceStatusClient::GetInstance().EnableCoordination(userData_++, enabled);
    if (ret != RET_OK) {
        FI_HILOGE("Get Coordination State failed");
    } else {
        devCoordinationEvent_[userData_] = event;
    }
    return ret;
}

int32_t CoordinationManagerImpl::StartCoordination(const std::string &sinkDeviceId,
    int32_t srcDeviceId, FuncCoordinationMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationEvent event;
    event.msg = callback;
    if (userData_ == INT32_MAX) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    auto ret = DeviceStatusClient::GetInstance().StartCoordination(
        userData_++, sinkDeviceId, srcDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Get Coordination State failed");
    } else {
        devCoordinationEvent_[userData_] = event;
    }
    return ret;
}

int32_t CoordinationManagerImpl::StopCoordination(FuncCoordinationMessage callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationEvent event;
    event.msg = callback;
    if (userData_ == INT32_MAX) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    auto ret = DeviceStatusClient::GetInstance().StopCoordination(userData_++);
    if (ret != RET_OK) {
        FI_HILOGE("Get Coordination State failed");
    } else {
        devCoordinationEvent_[userData_] = event;
    }
    return ret;
}

int32_t CoordinationManagerImpl::GetCoordinationState(
    const std::string &deviceId, FuncCoordinationState callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationEvent event;
    event.state = callback;
    if (userData_ == INT32_MAX) {
        FI_HILOGE("userData exceeds the maximum");
        return RET_ERR;
    }
    auto ret = DeviceStatusClient::GetInstance().GetCoordinationState(userData_++, deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Get Coordination State failed");
    } else {
        devCoordinationEvent_[userData_] = event;
    }
    return ret;
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
    CoordinationMsg* event;
    auto iter = devCoordinationEvent_.find(userData);
    if (iter == devCoordinationEvent_.end()) {
        event = nullptr;
    } else {
        event = &iter->second.msg;
    }
    CHKPV(event);
    devCoordinationEvent_.erase(iter);
    (*event)(deviceId, msg);
}

void CoordinationManagerImpl::OnCoordinationState(int32_t userData, bool state)
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    std::lock_guard<std::mutex> guard(mtx_);
    CoordinationState* event;
    auto iter = devCoordinationEvent_.find(userData);
    if (iter == devCoordinationEvent_.end()) {
        event = nullptr;
    } else {
        event = &iter->second.state;
    }
    CHKPV(event);
    (*event)(state);
    devCoordinationEvent_.erase(iter);
    FI_HILOGD("Coordination state event callback userData:%{public}d state:(%{public}d)", userData, state);
}

int32_t CoordinationManagerImpl::GetUserData()
{
    std::lock_guard<std::mutex> guard(mtx_);
    return userData_;
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