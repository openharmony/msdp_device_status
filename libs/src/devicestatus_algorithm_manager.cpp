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

#include "devicestatus_algorithm_manager.h"
#include <cerrno>
#include <linux/netlink.h>
#include <string>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace {
DevicestatusAlgorithmManager* g_rdb;
}

bool DevicestatusAlgorithmManager::Init()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    if (!sensorEventCb_) {
        sensorEventCb_ = SensorDataCallback::GetInstance();
        sensorEventCb_->Initiate();
    }
    DEV_HILOGI(SERVICE, "%{public}s exit", __func__);
    return true;
}

ErrCode DevicestatusAlgorithmManager::RegisterCallback(std::shared_ptr<DevicestatusAlgorithmCallback>& callback)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    if(type_ == DevicestatusDataUtils::TYPE_STILL) {
        if (!still_) {
            still_ = std::make_shared<AbsoluteStill>(sensorEventCb_);
            still_->RegisterCallback(callback);
            still_->Init();
        }
    } else if(type_ == DevicestatusDataUtils::TYPE_HORIZONTAL_POSITION) {
        if (!horizontalPosition_) {
            horizontalPosition_ = std::make_shared<DeviceStatusHorizontal>(sensorEventCb_);
            horizontalPosition_->RegisterCallback(callback);
            horizontalPosition_->Init(); 
        }
    } else if(type_ == DevicestatusDataUtils::TYPE_VERTICAL_POSITION) {
        if (!verticalPosition_) {
            verticalPosition_ = std::make_shared<DeviceStatusVertical>(sensorEventCb_);
            verticalPosition_->RegisterCallback(callback);
            verticalPosition_->Init();
        }
    }
    return ERR_OK;
}

ErrCode DevicestatusAlgorithmManager::UnregisterCallback()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    callbacksImpl_ = nullptr;
    return ERR_OK;
}

ErrCode DevicestatusAlgorithmManager::DisableCount(const DevicestatusDataUtils::DevicestatusType& type) 
{
    return ERR_OK;
}

ErrCode DevicestatusAlgorithmManager::Enable(const DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    type_ = type;
    int deviceStatusItem = int(type);
    in_type[deviceStatusItem] = int(DevicestatusDataUtils::DevicestatusTypeValue::VALID);
    Init();
    DEV_HILOGI(SERVICE, "%{public}s exit", __func__);
    return ERR_OK;
}

ErrCode DevicestatusAlgorithmManager::Disable(const DevicestatusDataUtils::DevicestatusType& type)
{
    return ERR_OK;
}

extern "C" DevicestatusAlgorithmManagerInterface *Create(void)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    g_rdb = new DevicestatusAlgorithmManager();
    return g_rdb;
}

extern "C" void Destroy(const DevicestatusAlgorithmManagerInterface* algorithm)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    delete algorithm;
}
} // namespace Msdp
} // namespace OHOS
