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

#ifdef DEVICE_STATUS_SENSOR_ENABLE
#include "devicestatus_algorithm_manager.h"

#include <cerrno>
#include <string>

#include <linux/netlink.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "devicestatus_common.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "AlgoMgr"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

bool AlgoMgr::StartSensor(Type type)
{
    CALL_DEBUG_ENTER;
    int32_t sensorType = GetSensorTypeId(type);
    if (sensorType == RET_ERR) {
        FI_HILOGE("Failed to get sensorType");
        return false;
    }
    if (!CheckSensorTypeId(sensorType)) {
        FI_HILOGE("Sensor type mismatch");
        return false;
    }

    SENSOR_DATA_CB.Init();
    if (!SENSOR_DATA_CB.RegisterCallbackSensor(sensorType)) {
        FI_HILOGE("Failed to register callback sensor");
        return false;
    }

    return true;
}

ErrCode AlgoMgr::RegisterCallback(std::shared_ptr<MsdpAlgoCallback> callback)
{
    CALL_DEBUG_ENTER;
    switch (algoType_) {
        case Type::TYPE_ABSOLUTE_STILL: {
            if (still_ != nullptr) {
                still_->RegisterCallback(callback);
            }
            break;
        }
        case Type::TYPE_HORIZONTAL_POSITION: {
            if (horizontalPosition_ != nullptr) {
                horizontalPosition_->RegisterCallback(callback);
            }
            break;
        }
        case Type::TYPE_VERTICAL_POSITION: {
            if (verticalPosition_ != nullptr) {
                verticalPosition_->RegisterCallback(callback);
            }
            break;
        }
        default: {
            FI_HILOGE("Unsupported algorithm type");
            return RET_ERR;
        }
    }
    return RET_OK;
}

ErrCode AlgoMgr::UnregisterCallback()
{
    CALL_DEBUG_ENTER;
    return RET_OK;
}

bool AlgoMgr::CheckSensorTypeId(int32_t sensorTypeId)
{
    int32_t count = -1;
    SensorInfo *sensorInfo = nullptr;
    int32_t ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0) {
        FI_HILOGE("Get all sensors failed");
        return false;
    }
    SensorInfo *pt = sensorInfo + count;
    for (SensorInfo *ps = sensorInfo; ps < pt; ++ps) {
        if (ps->sensorTypeId == sensorTypeId) {
            FI_HILOGI("Get sensor sensorTypeId: %{public}d", sensorTypeId);
            return true;
        }
    }
    FI_HILOGE("Get sensor failed");
    return false;
}

int32_t AlgoMgr::GetSensorTypeId(Type type)
{
    FI_HILOGI("Get sensor type: %{public}d", type);
    switch (type) {
        case Type::TYPE_ABSOLUTE_STILL: {
            return SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
        }
        case Type::TYPE_HORIZONTAL_POSITION: {
            return SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
        }
        case Type::TYPE_VERTICAL_POSITION: {
            return SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER;
        }
        default: {
            FI_HILOGW("GetSensorTypeId failed");
            break;
        }
    }
    return RET_ERR;
}

ErrCode AlgoMgr::Enable(Type type)
{
    CALL_DEBUG_ENTER;
    if (!StartSensor(type)) {
        FI_HILOGE("sensor init failed");
        return RET_ERR;
    }
    switch (type) {
        case Type::TYPE_ABSOLUTE_STILL: {
            if (still_ == nullptr) {
                FI_HILOGE("still_ is nullptr");
                still_ = std::make_shared<AlgoAbsoluteStill>();
                still_->Init(type);
                callAlgoNums_[type] = 0;
            }
            callAlgoNums_[type]++;
            break;
        }
        case Type::TYPE_HORIZONTAL_POSITION: {
            if (horizontalPosition_ == nullptr) {
                FI_HILOGE("horizontalPosition_ is nullptr");
                horizontalPosition_ = std::make_shared<AlgoHorizontal>();
                horizontalPosition_->Init(type);
                callAlgoNums_[type] = 0;
            }
            callAlgoNums_[type]++;
            break;
        }
        case Type::TYPE_VERTICAL_POSITION: {
            if (verticalPosition_ == nullptr) {
                FI_HILOGE("verticalPosition_ is nullptr");
                verticalPosition_ = std::make_shared<AlgoVertical>();
                verticalPosition_->Init(type);
                callAlgoNums_[type] = 0;
            }
            callAlgoNums_[type]++;
            break;
        }
        default: {
            FI_HILOGE("Unsupported algorithm type");
            return RET_ERR;
        }
    }
    algoType_ = type;
    return RET_OK;
}

ErrCode AlgoMgr::Disable(Type type)
{
    CALL_DEBUG_ENTER;
    callAlgoNums_[type]--;
    FI_HILOGI("callAlgoNums_:%{public}d", callAlgoNums_[type]);
    if (callAlgoNums_[type] != 0) {
        FI_HILOGE("callAlgoNums_[type] is not zero");
        return RET_ERR;
    }
    switch (type) {
        case Type::TYPE_ABSOLUTE_STILL: {
            if (still_ != nullptr) {
                FI_HILOGD("still_ is not nullptr");
                still_->Unsubscribe(type);
                still_ = nullptr;
            }
            break;
        }
        case Type::TYPE_HORIZONTAL_POSITION: {
            if (horizontalPosition_ != nullptr) {
                FI_HILOGD("horizontalPosition_ is not nullptr");
                horizontalPosition_->Unsubscribe(type);
                horizontalPosition_ = nullptr;
            }
            break;
        }
        case Type::TYPE_VERTICAL_POSITION: {
            if (verticalPosition_ != nullptr) {
                FI_HILOGD("verticalPosition_ is not nullptr");
                verticalPosition_->Unsubscribe(type);
                verticalPosition_ = nullptr;
            }
            break;
        }
        default: {
            FI_HILOGE("Unsupported algorithm type");
            break;
        }
    }
    callAlgoNums_.erase(type);
    UnregisterSensor(type);
    return RET_OK;
}

ErrCode AlgoMgr::DisableCount(Type type)
{
    CALL_DEBUG_ENTER;
    return RET_OK;
}

ErrCode AlgoMgr::UnregisterSensor(Type type)
{
    CALL_DEBUG_ENTER;
    int32_t sensorType = GetSensorTypeId(type);
    if (sensorType == RET_ERR) {
        FI_HILOGE("Failed to get sensorType");
        return false;
    }
    if (!SENSOR_DATA_CB.UnregisterCallbackSensor(sensorType)) {
        FI_HILOGE("Failed to unregister callback sensor");
        return false;
    }
    return true;
}

extern "C" IMsdp *Create(void)
{
    CALL_DEBUG_ENTER;
    return new (std::nothrow) AlgoMgr();
}

extern "C" void Destroy(const IMsdp* algorithm)
{
    CALL_DEBUG_ENTER;
    if (algorithm != nullptr) {
        FI_HILOGD("algorithm is not nullptr");
        delete algorithm;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE
