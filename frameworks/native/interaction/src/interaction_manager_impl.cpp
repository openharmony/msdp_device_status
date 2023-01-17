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

#include "interaction_manager_impl.h"

#include "coordination_manager_impl.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerImpl" };
} // namespace

InteractionManagerImpl::InteractionManagerImpl() {}
InteractionManagerImpl::~InteractionManagerImpl() {}

bool InteractionManagerImpl::InitClient()
{
    CALL_DEBUG_ENTER;
    if (client_ != nullptr) {
        return true;
    }
    client_ = std::make_shared<Client>();
    if (!(client_->Start())) {
        client_.reset();
        client_ = nullptr;
        FI_HILOGE("The client fails to start");
        return false;
    }
    return true;
}

int32_t InteractionManagerImpl::RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.RegisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.UnregisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::EnableCoordination(bool enabled,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.EnableCoordination(enabled, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(enabled);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::StartCoordination(const std::string &sinkDeviceId, int32_t srcDeviceId,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.StartCoordination(sinkDeviceId, srcDeviceId, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(sinkDeviceId);
    (void)(srcDeviceId);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::StopCoordination(std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.StopCoordination(callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::GetCoordinationState(
    const std::string &deviceId, std::function<void(bool)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return CoordinationMgrImpl.GetCoordinationState(deviceId, callback);
#else
    (void)(deviceId);
    (void)(callback);
    FI_HILOGW("Coordination does not support");
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.UpdateDragStyle(style);
}

int32_t InteractionManagerImpl::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.UpdateDragMessage(message);
}

int32_t InteractionManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragTargetPid();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
