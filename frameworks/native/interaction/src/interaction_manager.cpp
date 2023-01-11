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
#include "drag_manager_impl.h"
#include "drag_data.h"
#include "devicestatus_define.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManager" };
} // namespace
using namespace DeviceStatus;
InteractionManager *InteractionManager::instance_ = new (std::nothrow) InteractionManager();

InteractionManager *InteractionManager::GetInstance()
{
    return instance_;
}

int32_t InteractionManager::RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.RegisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManager::UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.UnregisterCoordinationListener(listener);
#else
    FI_HILOGW("Coordination does not support");
    (void)(listener);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManager::EnableCoordination(bool enabled,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.EnableCoordination(enabled, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(enabled);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManager::StartCoordination(const std::string &sinkDeviceId, int32_t srcDeviceId,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.StartCoordination(sinkDeviceId, srcDeviceId, callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(sinkDeviceId);
    (void)(srcDeviceId);
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManager::StopCoordination(std::function<void(std::string, CoordinationMessage)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.StopCoordination(callback);
#else
    FI_HILOGW("Coordination does not support");
    (void)(callback);
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

int32_t InteractionManager::GetCoordinationState(
    const std::string &deviceId, std::function<void(bool)> callback)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    return CoordinationMgrImpl.GetCoordinationState(deviceId, callback);
#else
    (void)(deviceId);
    (void)(callback);
    FI_HILOGW("Coordination does not support");
    return ERROR_UNSUPPORT;
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * buffer 里边是啥在整个框架中是不需要关注的，我们只需要将buffer往下传传到服务端
 * StartDrag 接口传入的参数较多，之后考虑将某几个整合成一个结构体
*/
int32_t InteractionManager::StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback)
{
    CALL_DEBUG_ENTER;
    return DragMgrImpl.StartDrag(dragData, callback);
}

int32_t InteractionManager::StopDrag(int32_t &dragResult)
{
    return DragMgrImpl.StopDrag(dragResult);
}

} // namespace Msdp
} // namespace OHOS
