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

#include "devicestatus_service.h"

#include <ipc_skeleton.h>
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "devicestatus_permission.h"
#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace {
auto ms = DelayedSpSingleton<DevicestatusService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(ms.GetRefPtr());
}
DevicestatusService::DevicestatusService() : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{
    DEV_HILOGD(SERVICE, "Add SystemAbility");
}

DevicestatusService::~DevicestatusService() {}

void DevicestatusService::OnStart()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (ready_) {
        DEV_HILOGE(SERVICE, "OnStart is ready, nothing to do");
        return;
    }

    if (!Init()) {
        DEV_HILOGE(SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<DevicestatusService>::GetInstance())) {
        DEV_HILOGE(SERVICE, "OnStart register to system ability manager failed");
        return;
    }
    ready_ = true;
    DEV_HILOGI(SERVICE, "OnStart and add system ability success");
}

void DevicestatusService::OnStop()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (!ready_) {
        return;
    }
    ready_ = false;

    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "devicestatusManager_ is null");
        return;
    }
    devicestatusManager_->UnloadAlgorithm(false);
    DEV_HILOGI(SERVICE, "unload algorithm library exit");
}

bool DevicestatusService::Init()
{
    DEV_HILOGI(SERVICE, "Enter");

    if (!devicestatusManager_) {
        devicestatusManager_ = std::make_shared<DevicestatusManager>(ms);
    }
    if (!devicestatusManager_->Init()) {
        DEV_HILOGE(SERVICE, "OnStart init fail");
        return false;
    }

    return true;
}

bool DevicestatusService::IsServiceReady()
{
    DEV_HILOGI(SERVICE, "Enter");
    return ready_;
}

std::shared_ptr<DevicestatusManager> DevicestatusService::GetDevicestatusManager()
{
    DEV_HILOGI(SERVICE, "Enter");
    return devicestatusManager_;
}

void DevicestatusService::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    devicestatusManager_->Subscribe(type, callback);
}

void DevicestatusService::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    devicestatusManager_->UnSubscribe(type, callback);
}

DevicestatusDataUtils::DevicestatusData DevicestatusService::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    return devicestatusManager_->GetLatestDevicestatusData(type);
}
} // namespace Msdp
} // namespace OHOS
