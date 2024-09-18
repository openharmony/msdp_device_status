/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "interaction_manager.h"

#include "devicestatus_define.h"
#include "drag_data.h"
#include "intention_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using CooperateMsgInfoCallback = std::function<void(const std::string&, const CoordinationMsgInfo&)>;
InteractionManager *InteractionManager::instance_ = new (std::nothrow) InteractionManager();

InteractionManager *InteractionManager::GetInstance()
{
    return instance_;
}

int32_t InteractionManager::RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener,
    bool isCompatible)
{
    return INTER_MGR_IMPL.RegisterCoordinationListener(listener, isCompatible);
}

int32_t InteractionManager::UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener,
    bool isCompatible)
{
    return INTER_MGR_IMPL.UnregisterCoordinationListener(listener, isCompatible);
}

int32_t InteractionManager::PrepareCoordination(CooperateMsgInfoCallback callback, bool isCompatible)
{
    return INTER_MGR_IMPL.PrepareCoordination(callback, isCompatible);
}

int32_t InteractionManager::UnprepareCoordination(CooperateMsgInfoCallback callback, bool isCompatible)
{
    return INTER_MGR_IMPL.UnprepareCoordination(callback, isCompatible);
}

int32_t InteractionManager::ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId,
    CooperateMsgInfoCallback callback, bool isCompatible)
{
    return INTER_MGR_IMPL.ActivateCoordination(remoteNetworkId, startDeviceId, callback, isCompatible);
}

int32_t InteractionManager::DeactivateCoordination(bool isUnchained, CooperateMsgInfoCallback callback,
    bool isCompatible)
{
    return INTER_MGR_IMPL.DeactivateCoordination(isUnchained, callback, isCompatible);
}

int32_t InteractionManager::GetCoordinationState(
    const std::string &networkId, std::function<void(bool)> callback, bool isCompatible)
{
    return INTER_MGR_IMPL.GetCoordinationState(networkId, callback, isCompatible);
}

int32_t InteractionManager::GetCoordinationState(const std::string &udId, bool &state)
{
    return INTER_MGR_IMPL.GetCoordinationState(udId, state);
}

int32_t InteractionManager::RegisterEventListener(const std::string &networkId,
    std::shared_ptr<IEventListener> listener)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.RegisterEventListener(networkId, listener);
#else
    return RET_OK;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

int32_t InteractionManager::UnregisterEventListener(const std::string &networkId,
    std::shared_ptr<IEventListener> listener)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.UnregisterEventListener(networkId, listener);
#else
    return RET_OK;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

int32_t InteractionManager::SetDamplingCoefficient(uint32_t direction, double coefficient)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.SetDamplingCoefficient(direction, coefficient);
#else
    return RET_OK;
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

int32_t InteractionManager::UpdateDragStyle(DragCursorStyle style, int32_t eventId)
{
    return INTER_MGR_IMPL.UpdateDragStyle(style, eventId);
}

int32_t InteractionManager::StartDrag(const DragData &dragData, std::shared_ptr<IStartDragListener> listener)
{
    return INTER_MGR_IMPL.StartDrag(dragData, listener);
}

int32_t InteractionManager::StopDrag(const DragDropResult &dropResult)
{
    return INTER_MGR_IMPL.StopDrag(dropResult);
}

int32_t InteractionManager::GetDragTargetPid()
{
    return INTER_MGR_IMPL.GetDragTargetPid();
}

int32_t InteractionManager::GetUdKey(std::string &udKey)
{
    return INTER_MGR_IMPL.GetUdKey(udKey);
}

int32_t InteractionManager::AddDraglistener(DragListenerPtr listener, bool isJsCaller)
{
    return INTER_MGR_IMPL.AddDraglistener(listener, isJsCaller);
}

int32_t InteractionManager::RemoveDraglistener(DragListenerPtr listener, bool isJsCaller)
{
    return INTER_MGR_IMPL.RemoveDraglistener(listener, isJsCaller);
}

int32_t InteractionManager::AddSubscriptListener(SubscriptListenerPtr listener)
{
    return INTER_MGR_IMPL.AddSubscriptListener(listener);
}

int32_t InteractionManager::RemoveSubscriptListener(SubscriptListenerPtr listener)
{
    return INTER_MGR_IMPL.RemoveSubscriptListener(listener);
}

int32_t InteractionManager::SetDragWindowVisible(bool visible, bool isForce)
{
    return INTER_MGR_IMPL.SetDragWindowVisible(visible, isForce);
}

int32_t InteractionManager::GetShadowOffset(int32_t &offsetX, int32_t &offsetY, int32_t &width, int32_t &height)
{
    ShadowOffset shadowOffset;
    int32_t ret = INTER_MGR_IMPL.GetShadowOffset(shadowOffset);
    offsetX = shadowOffset.offsetX;
    offsetY = shadowOffset.offsetY;
    width = shadowOffset.width;
    height = shadowOffset.height;
    return ret;
}

int32_t InteractionManager::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    return INTER_MGR_IMPL.UpdateShadowPic(shadowInfo);
}

int32_t InteractionManager::GetDragData(DragData &dragData)
{
    return INTER_MGR_IMPL.GetDragData(dragData);
}

int32_t InteractionManager::GetDragState(DragState &dragState)
{
    return INTER_MGR_IMPL.GetDragState(dragState);
}

int32_t InteractionManager::AddHotAreaListener(std::shared_ptr<IHotAreaListener> listener)
{
    return INTER_MGR_IMPL.AddHotAreaListener(listener);
}

int32_t InteractionManager::RemoveHotAreaListener(std::shared_ptr<IHotAreaListener> listener)
{
    return INTER_MGR_IMPL.RemoveHotAreaListener(listener);
}

int32_t InteractionManager::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    return INTER_MGR_IMPL.UpdatePreviewStyle(previewStyle);
}

int32_t InteractionManager::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle,
    const PreviewAnimation &animation)
{
    return INTER_MGR_IMPL.UpdatePreviewStyleWithAnimation(previewStyle, animation);
}

int32_t InteractionManager::RotateDragWindowSync(const std::shared_ptr<Rosen::RSTransaction>& rsTransaction)
{
    return INTER_MGR_IMPL.RotateDragWindowSync(rsTransaction);
}

int32_t InteractionManager::SetDragWindowScreenId(uint64_t displayId, uint64_t screenId)
{
    return INTER_MGR_IMPL.SetDragWindowScreenId(displayId, screenId);
}

int32_t InteractionManager::GetDragSummary(std::map<std::string, int64_t> &summarys, bool isJsCaller)
{
    return INTER_MGR_IMPL.GetDragSummary(summarys, isJsCaller);
}

int32_t InteractionManager::GetDragAction(DragAction &dragAction)
{
    return INTER_MGR_IMPL.GetDragAction(dragAction);
}

int32_t InteractionManager::EnterTextEditorArea(bool enable)
{
    return INTER_MGR_IMPL.EnterTextEditorArea(enable);
}

int32_t InteractionManager::GetExtraInfo(std::string &extraInfo)
{
    return INTER_MGR_IMPL.GetExtraInfo(extraInfo);
}

int32_t InteractionManager::AddPrivilege()
{
    return INTER_MGR_IMPL.AddPrivilege();
}

int32_t InteractionManager::EraseMouseIcon()
{
    return INTER_MGR_IMPL.EraseMouseIcon();
}

int32_t InteractionManager::SetMouseDragMonitorState(bool state)
{
    return INTER_MGR_IMPL.SetMouseDragMonitorState(state);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
