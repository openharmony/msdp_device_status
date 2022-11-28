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

#include "devicestatus_client.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
using namespace DeviceStatus;
InteractionManager *InteractionManager::instance_ = new (std::nothrow) InteractionManager();

InteractionManager *InteractionManager::GetInstance()
{
    return instance_;
}

int32_t InteractionManager::RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    return DevicestatusClient::GetInstance().RegisterCoordinationListener(listener);
}

int32_t InteractionManager::UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener)
{
    return DevicestatusClient::GetInstance().UnregisterCoordinationListener(listener);
}

int32_t InteractionManager::EnableInputDeviceCoordination(bool enabled,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    return DevicestatusClient::GetInstance().EnableInputDeviceCoordination(enabled, callback);
}

int32_t InteractionManager::StartInputDeviceCoordination(const std::string &sinkDeviceId, int32_t srcInputDeviceId,
    std::function<void(std::string, CoordinationMessage)> callback)
{
    return DevicestatusClient::GetInstance().StartInputDeviceCoordination(sinkDeviceId, srcInputDeviceId, callback);
}

int32_t InteractionManager::StopDeviceCoordination(std::function<void(std::string, CoordinationMessage)> callback)
{
    return DevicestatusClient::GetInstance().StopDeviceCoordination(callback);
}

int32_t InteractionManager::GetInputDeviceCoordinationState(
    const std::string &deviceId, std::function<void(bool)> callback)
{
    return DevicestatusClient::GetInstance().GetInputDeviceCoordinationState(deviceId, callback);
}
} // namespace Msdp
} // namespace OHOS
