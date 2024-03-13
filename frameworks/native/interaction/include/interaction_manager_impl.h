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

#ifndef INTERACTION_MANAGER_IMPL_H
#define INTERACTION_MANAGER_IMPL_H

#include <mutex>

#include "client.h"
#include "coordination_manager_impl.h"
#include "drag_data.h"
#include "drag_manager_impl.h"
#include "interaction_manager.h"
#include "singleton.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class InteractionManagerImpl {
    DECLARE_SINGLETON(InteractionManagerImpl);
public:
    DISALLOW_MOVE(InteractionManagerImpl);
    bool InitClient();
    int32_t RegisterCoordinationListener(
        std::shared_ptr<ICoordinationListener> listener, bool isCompatible = false);
    int32_t UnregisterCoordinationListener(
        std::shared_ptr<ICoordinationListener> listener, bool isCompatible = false);
    int32_t PrepareCoordination(
        std::function<void(std::string, CoordinationMessage)> callback, bool isCompatible = false);
    int32_t UnprepareCoordination(
        std::function<void(std::string, CoordinationMessage)> callback, bool isCompatible = false);
    int32_t ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId,
        std::function<void(std::string, CoordinationMessage)> callback, bool isCompatible = false);
    int32_t DeactivateCoordination(bool isUnchained, std::function<void(std::string, CoordinationMessage)> callback,
        bool isCompatible = false);
    int32_t GetCoordinationState(const std::string &networkId, std::function<void(bool)> callback,
        bool isCompatible = false);
    int32_t GetCoordinationState(const std::string &udId, bool &state);
    int32_t UpdateDragStyle(DragCursorStyle style);
    int32_t StartDrag(const DragData &dragData, std::shared_ptr<IStartDragListener> listener);
    int32_t StopDrag(const DragDropResult &dropResult);
    int32_t GetDragTargetPid();
    int32_t GetUdKey(std::string &udKey);
    int32_t AddDraglistener(DragListenerPtr listener);
    int32_t RemoveDraglistener(DragListenerPtr listener);
    int32_t AddSubscriptListener(SubscriptListenerPtr listener);
    int32_t RemoveSubscriptListener(SubscriptListenerPtr listener);
    int32_t SetDragWindowVisible(bool visible, bool isForce = false);
    int32_t GetShadowOffset(ShadowOffset &shadowOffset);
    int32_t UpdateShadowPic(const ShadowInfo &shadowInfo);
    int32_t GetDragData(DragData &dragData);
    int32_t GetDragState(DragState &dragState);
    int32_t AddHotAreaListener(std::shared_ptr<IHotAreaListener> listener);
    int32_t RemoveHotAreaListener(std::shared_ptr<IHotAreaListener> listener = nullptr);
    int32_t UpdatePreviewStyle(const PreviewStyle &previewStyle);
    int32_t UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle, const PreviewAnimation &animation);
    int32_t GetDragSummary(std::map<std::string, int64_t> &summarys);
    int32_t EnterTextEditorArea(bool enable);
    int32_t GetDragAction(DragAction &dragAction);
    int32_t GetExtraInfo(std::string &extraInfo);
    int32_t AddPrivilege();

private:
    void InitMsgHandler();
private:
    std::mutex mutex_;
    IClientPtr client_ { nullptr };
    DragManagerImpl dragManagerImpl_;
    CoordinationManagerImpl coordinationManagerImpl_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#define INTER_MGR_IMPL OHOS::Singleton<InteractionManagerImpl>::GetInstance()

#endif // INTERACTION_MANAGER_IMPL_H
