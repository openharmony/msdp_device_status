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
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerImpl" };
} // namespace

InteractionManagerImpl::InteractionManagerImpl() {}
InteractionManagerImpl::~InteractionManagerImpl()
{
    CALL_DEBUG_ENTER;
}

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
        FI_HILOGE("The client failed to start");
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
        {MessageId::HOT_AREA_ADD_LISTENER,
            MsgCallbackBind2(&CoordinationManagerImpl::OnHotAreaListener, &coordinationManagerImpl_)},
#endif // OHOS_BUILD_ENABLE_COORDINATION
        {MessageId::DRAG_NOTIFY_RESULT,
            MsgCallbackBind2(&DragManagerImpl::OnNotifyResult, &dragManagerImpl_)},
        {MessageId::DRAG_STATE_LISTENER,
            MsgCallbackBind2(&DragManagerImpl::OnStateChangedMessage, &dragManagerImpl_)},
        {MessageId::DRAG_STYLE_LISTENER,
            MsgCallbackBind2(&DragManagerImpl::OnDragStyleChangedMessage, &dragManagerImpl_)}
    };
    CHKPV(client_);
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
    return coordinationManagerImpl_.UnregisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::PrepareCoordination(std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.PrepareCoordination(callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::UnprepareCoordination(std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.UnprepareCoordination(callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.ActivateCoordination(remoteNetworkId, startDeviceId, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(remoteNetworkId);
    (void)(startDeviceId);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::DeactivateCoordination(bool isUnchained,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.DeactivateCoordination(isUnchained, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::GetCoordinationState(
    const std::string &networkId, std::function<void(bool)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.GetCoordinationState(networkId, callback);
#else
    (void)(networkId);
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

int32_t InteractionManagerImpl::StopDrag(const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.StopDrag(dropResult);
}

int32_t InteractionManagerImpl::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragTargetPid();
}

int32_t InteractionManagerImpl::GetUdKey(std::string &udKey)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetUdKey(udKey);
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

int32_t InteractionManagerImpl::AddSubscriptListener(SubscriptListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return dragManagerImpl_.AddSubscriptListener(listener);
}

int32_t InteractionManagerImpl::RemoveSubscriptListener(SubscriptListenerPtr listener)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.RemoveSubscriptListener(listener);
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

int32_t InteractionManagerImpl::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.UpdateShadowPic(shadowInfo);
}

int32_t InteractionManagerImpl::GetDragData(DragData &dragData)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragData(dragData);
}

int32_t InteractionManagerImpl::GetDragState(DragState &dragState)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragState(dragState);
}

int32_t InteractionManagerImpl::AddHotAreaListener(std::shared_ptr<IHotAreaListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    if (!InitClient()) {
        FI_HILOGE("Get client is nullptr");
        return RET_ERR;
    }
    return coordinationManagerImpl_.AddHotAreaListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::RemoveHotAreaListener(std::shared_ptr<IHotAreaListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    std::lock_guard<std::mutex> guard(mutex_);
    return coordinationManagerImpl_.RemoveHotAreaListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManagerImpl::UpdateDragItemStyle(const DragItemStyle &dragItemStyle)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.UpdateDragItemStyle(dragItemStyle);
}

int32_t InteractionManagerImpl::GetDragSummary(std::map<std::string, int64_t> &summarys)
{
    CALL_DEBUG_ENTER;
    return dragManagerImpl_.GetDragSummary(summarys);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
