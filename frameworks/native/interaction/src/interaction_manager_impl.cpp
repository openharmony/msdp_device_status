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

#include "devicestatus_define.h"
#include "devicestatus_func_callback.h"
#include "drag_data.h"
#include "drag_manager_impl.h"

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
    if (client_ != nullptr && client_->CheckValidFd()) {
        return true;
    }
    client_ = std::make_shared<Client>();
    InitMsgHandler();
    if (!(client_->Start())) {
        client_.reset();
        client_ = nullptr;
        FI_HILOGE("The client fails to start");
        return false;
    }
    return true;
}

void InteractionManagerImpl::InitMsgHandler()
{
    CALL_DEBUG_ENTER;
    Client::MsgCallback funs[] = {
#ifdef OHOS_BUILD_ENABLE_COORDINATION
        {MessageId::COORDINATION_ADD_LISTENER,
            MsgCallbackBind2(&CoordinationManagerImpl::OnCoordinationListener, &coordinationManagerImpl_)},
        {MessageId::COORDINATION_MESSAGE,
            MsgCallbackBind2(&CoordinationManagerImpl::OnCoordinationMessage, &coordinationManagerImpl_)},
        {MessageId::COORDINATION_GET_STATE,
            MsgCallbackBind2(&CoordinationManagerImpl::OnCoordinationState, &coordinationManagerImpl_)},
#endif // OHOS_BUILD_ENABLE_COORDINATION
        {MessageId::DRAG_NOTIFY_RESULT,
            MsgCallbackBind2(&DragManagerImpl::OnNotifyResult, &dragManagerImpl_)},
        {MessageId::DRAG_STATE_LISTENER,
            MsgCallbackBind2(&DragManagerImpl::OnStateChangedMessage, &dragManagerImpl_)}
    };
    for (auto &it : funs) {
        if (!client_->RegisterEvent(it)) {
            FI_HILOGI("RegistER event handler msg:%{publid}d already exists", it.id);
        }
    }
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
    return coordinationManagerImpl_.RegisterCoordinationListener(listener);
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
    return coordinationManagerImpl_.UnregisterCoordinationListener(listener);
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
    return coordinationManagerImpl_.EnableCoordination(enabled, callback);
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
    return coordinationManagerImpl_.StartCoordination(sinkDeviceId, srcDeviceId, callback);
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
    return coordinationManagerImpl_.StopCoordination(callback);
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
    return coordinationManagerImpl_.GetCoordinationState(deviceId, callback);
#else
    (void)(deviceId);
    (void)(callback);
    FI_HILOGW("Coordination does not support");
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.UpdateDragStyle(style);
}

int32_t InteractionManagerImpl::StartDrag(const DragData &dragData, std::function<void(const DragNotifyMsg&)> callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.StartDrag(dragData, callback);
}

int32_t InteractionManagerImpl::StopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.StopDrag(result, hasCustomAnimation);
}

int32_t InteractionManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragTargetPid();
}

int32_t InteractionManagerImpl::AddDraglistener(DragListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.AddDraglistener(listener);
}

int32_t InteractionManagerImpl::RemoveDraglistener(DragListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.RemoveDraglistener(listener);
}

int32_t InteractionManagerImpl::SetDragWindowVisible(bool visible)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.SetDragWindowVisible(visible);
}

int32_t InteractionManagerImpl::GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetShadowOffset(offsetX, offsetY, width, height);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
