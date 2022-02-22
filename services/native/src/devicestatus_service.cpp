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
    DEVICESTATUS_HILOGD(DEVICESTATUS_MODULE_SERVICE, "Add SystemAbility");
}

DevicestatusService::~DevicestatusService() {}

void DevicestatusService::OnStart()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (ready_) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "OnStart is ready, nothing to do.");
        return;
    }

    if (!Init()) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<DevicestatusService>::GetInstance())) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "OnStart register to system ability manager failed");
        return;
    }
    ready_ = true;
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "OnStart and add system ability success");
}

void DevicestatusService::OnStop()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (!ready_) {
        return;
    }
    ready_ = false;

    if (!devicestatusManager_) {
        devicestatusManager_->UnloadAlgorithm(false);
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "unload algorithm library");
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

bool DevicestatusService::Init()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    if (!devicestatusManager_) {
        devicestatusManager_ = std::make_shared<DevicestatusManager>(ms);
    }
    if (!devicestatusManager_->Init()) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "OnStart init fail");
        return false;
    }

    return true;
}

bool DevicestatusService::IsServiceReady()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    return ready_;
}

std::shared_ptr<DevicestatusManager> DevicestatusService::GetDevicestatusManager()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    return devicestatusManager_;
}

void DevicestatusService::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    devicestatusManager_->Subscribe(type, callback);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

void DevicestatusService::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    devicestatusManager_->UnSubscribe(type, callback);
}

DevicestatusDataUtils::DevicestatusData DevicestatusService::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    return devicestatusManager_->GetLatestDevicestatusData(type);
}
} // namespace Msdp
} // namespace OHOS
