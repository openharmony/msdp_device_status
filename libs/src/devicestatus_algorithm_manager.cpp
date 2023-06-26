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
#include "devicestatus_algorithm_manager.h"

#include <cerrno>
#include <string>

#include <linux/netlink.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "devicestatus_common.h"
#include "devicestatus_data_define.h"

namespace OHOS {
namespace Msdp {
bool AlgoMgr::StartSensor(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::optional<int32_t> sensorIdKey = GetSensorTypeId(type);
    if (!sensorIdKey) {
        DEV_HILOGE(SERVICE, "Get sensor type id failed");
        return false;
    }
    int32_t sensorType = sensorIdKey.value();
    if (!CheckSensorTypeId(sensorType)) {
        DEV_HILOGE(SERVICE, "Sensor type mismatch");
        return false;
    }

    SensorDataCallback::GetInstance().Init();
    if (!SensorDataCallback::GetInstance().RegisterCallbackSensor(sensorType)) {
        DEV_HILOGE(SERVICE, "Failed to register callback sensor");
        return false;
    }

    return true;
}

ErrCode AlgoMgr::RegisterCallback(std::shared_ptr<DevicestatusMsdpInterface::MsdpAlgorithmCallback> callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    switch (algoType_) {
        case DevicestatusDataUtils::TYPE_STILL: {
            if (still_ != nullptr) {
                still_->RegisterCallback(callback);
            }
            break;
        }
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            if (relativeStill_ != nullptr) {
                relativeStill_->RegisterCallback(callback);
            }
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "Unsupported algorithm type");
            return RET_ERR;
        }
    }
    return RET_OK;
}

ErrCode AlgoMgr::UnregisterCallback()
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    switch (algoType_) {
        case DevicestatusDataUtils::TYPE_STILL: {
            if (still_ != nullptr) {
                still_->UnregisterCallback();
            }
            break;
        }
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            if (relativeStill_ != nullptr) {
                relativeStill_->UnregisterCallback();
            }
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "Unsupported algorithm type");
            return RET_ERR;
        }
    }
    return RET_OK;
}

bool AlgoMgr::CheckSensorTypeId(int32_t sensorTypeId)
{
    int32_t count = -1;
    SensorInfo *sensorInfo = nullptr;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "Get all sensors failed");
        return false;
    }
    SensorInfo *pt = sensorInfo + count;
    for (SensorInfo *ps = sensorInfo; ps < pt; ++ps) {
        if (ps->sensorTypeId == sensorTypeId) {
            return true;
        }
    }
    DEV_HILOGW(SERVICE, "Get sensor failed");
    return false;
}

std::optional<int32_t> AlgoMgr::GetSensorTypeId(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    switch (type) {
        case DevicestatusDataUtils::TYPE_STILL:
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            return std::make_optional(SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER);
        }
        default: {
            DEV_HILOGW(SERVICE, "GetSensorTypeId failed");
            break;
        }
    }
    return std::nullopt;
}

ErrCode AlgoMgr::Enable(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (!StartSensor(type)) {
        DEV_HILOGE(SERVICE, "sensor init failed");
        return RET_ERR;
    }
    switch (type) {
        case DevicestatusDataUtils::TYPE_STILL: {
            if (still_ == nullptr) {
                still_ = std::make_shared<AlgoAbsoluteStill>();
            }
            still_->Init(type);
            break;
        }
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            if (relativeStill_ == nullptr) {
                relativeStill_ = std::make_shared<AlgoRelativeStill>();
            }
            relativeStill_->Init(type);
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "Unsupported algorithm type");
            return RET_ERR;
        }
    }
    algoType_ = type;
    DEV_HILOGI(SERVICE, "Exit");
    return RET_OK;
}

ErrCode AlgoMgr::Disable(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    switch (type) {
        case DevicestatusDataUtils::TYPE_STILL: {
            if (still_ != nullptr) {
                DEV_HILOGE(SERVICE, "still_ is not nullptr");
                still_->Unsubscribe(type);
                still_ = nullptr;
            }
            break;
        }
        case DevicestatusDataUtils::TYPE_RELATIVE_STILL: {
            if (relativeStill_ != nullptr) {
                DEV_HILOGE(SERVICE, "relativeStill_ is not nullptr");
                relativeStill_->Unsubscribe(type);
                relativeStill_ = nullptr;
            }
            break;
        }
        default: {
            DEV_HILOGE(SERVICE, "Unsupported algorithm type");
            return RET_ERR;
        }
    }
    if (UnregisterSensor(type) != RET_OK) {
        DEV_HILOGE(SERVICE, "Failed to unregister sensor");
        return RET_ERR;
    }
    algoType_ = type;
    return RET_OK;
}

ErrCode AlgoMgr::UnregisterSensor(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::optional<int32_t> sensorIdKey = GetSensorTypeId(type);
    if (!sensorIdKey) {
        DEV_HILOGE(SERVICE, "Get sensor type id failed");
        return false;
    }
    int32_t sensorType = sensorIdKey.value();
    if (!SensorDataCallback::GetInstance().UnregisterCallbackSensor(sensorType)) {
        DEV_HILOGE(SERVICE, "Failed to unregister callback sensor");
        return RET_ERR;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return RET_OK;
}

extern "C" DevicestatusMsdpInterface *Create(void)
{
    DEV_HILOGI(SERVICE, "Create algorithm library");
    return new (std::nothrow) AlgoMgr();
}

extern "C" void Destroy(const DevicestatusMsdpInterface* algorithm)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (algorithm != nullptr) {
        DEV_HILOGI(SERVICE, "Destroy algorithm library");
        delete algorithm;
    }
}
} // namespace Msdp
} // namespace OHOS
