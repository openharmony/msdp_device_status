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

#include "interaction_manager.h"

#include <string>

#include "devicestatus_define.h"
#include "drag_data.h"
#include "interaction_manager_impl.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

InteractionManager *InteractionManager::instance_ = new (std::nothrow) InteractionManager();

InteractionManager *InteractionManager::GetInstance()
{
    return instance_;
}

int32_t InteractionManager::RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    return InteractionMgrImpl.RegisterCoordinationListener(listener);
}

int32_t InteractionManager::UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    return InteractionMgrImpl.UnregisterCoordinationListener(listener);
}

int32_t InteractionManager::EnableCoordination(bool enabled,
    std::function<void(const std::string&, CoordinationMessage)> callback)
{
    return InteractionMgrImpl.EnableCoordination(enabled, callback);
}

int32_t InteractionManager::StartCoordination(const std::string &sinkDeviceId, int32_t srcDeviceId,
    std::function<void(const std::string&, CoordinationMessage)> callback)
{
    return InteractionMgrImpl.StartCoordination(sinkDeviceId, srcDeviceId, callback);
}

int32_t InteractionManager::StopCoordination(std::function<void(const std::string&, CoordinationMessage)> callback)
{
    return InteractionMgrImpl.StopCoordination(callback);
}

int32_t InteractionManager::GetCoordinationState(
    const std::string &deviceId, std::function<void(bool)> callback)
{
    return InteractionMgrImpl.GetCoordinationState(deviceId, callback);
}

int32_t InteractionManager::UpdateDragStyle(int32_t style)
{
    return InteractionMgrImpl.UpdateDragStyle(style);
}

int32_t InteractionManager::UpdateDragMessage(const std::u16string &message)
{
    return InteractionMgrImpl.UpdateDragMessage(message);
}

int32_t InteractionManager::StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback)
{
    return InteractionMgrImpl.StartDrag(dragData, callback);
}

int32_t InteractionManager::StopDrag(int32_t result)
{
    return InteractionMgrImpl.StopDrag(result);
}

int32_t InteractionManager::RegisterThumbnailDraw(std::function<void(int32_t, int32_t)> startDrag,
        std::function<void(int32_t)> notice, std::function<void(void)> endDrag)
{
    return InteractionMgrImpl.RegisterThumbnailDraw(startDrag, notice, endDrag);
}

int32_t InteractionManager::UnregisterThumbnailDraw()
{
    return InteractionMgrImpl.UnregisterThumbnailDraw();
}

int32_t InteractionManager::GetDragTargetPid()
{
    return InteractionMgrImpl.GetDragTargetPid();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
