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
    if (client_ != nullptr) {
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
        {MessageId::START_THUMBNAIL_DRAW,
            MsgCallbackBind2(&DragManagerImpl::OnStartThumbnailDraw, &dragManagerImpl_)},
        {MessageId::NOTICE_THUMBNAIL_DRAW,
            MsgCallbackBind2(&DragManagerImpl::OnNoticeThumbnailDraw, &dragManagerImpl_)},
        {MessageId::STOP_THUMBNAIL_DRAW,
            MsgCallbackBind2(&DragManagerImpl::OnStopThumbnailDraw, &dragManagerImpl_)},
        {MessageId::DRAG_NOTIFY_RESULT,
            MsgCallbackBind2(&DragManagerImpl::OnNotifyResult, &dragManagerImpl_)}
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

int32_t InteractionManagerImpl::StartDrag(const DragData &dragData, std::function<void(int32_t)> callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.StartDrag(dragData, callback);
}

int32_t InteractionManagerImpl::StopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.StopDrag(result);
}

int32_t InteractionManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragTargetPid();
}

int32_t InteractionManagerImpl::RegisterThumbnailDraw(std::function<void(std::shared_ptr<DragData>)> startCallback,
    std::function<void(int32_t, bool, std::u16string)> noticeCallback,
    std::function<void(int32_t, int32_t)> endCallback)
{
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.RegisterThumbnailDraw(startCallback, noticeCallback, endCallback);
}
    
int32_t InteractionManagerImpl::UnregisterThumbnailDraw(std::function<void(void)> callback)
{
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.UnregisterThumbnailDraw(callback);
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
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
