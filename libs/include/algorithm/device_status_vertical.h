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

#ifndef DEVICE_STATUS_VERTICAL_H
#define DEVICE_STATUS_VERTICAL_H
#include <iostream>
#include "devicestatus_data_utils.h"
#include "devicestatus_data_define.h"
#include "sensor_data_callback.h"
#include "devicestatus_algorithm_manager_interface.h"

namespace OHOS {
namespace Msdp {

class DeviceStatusVertical {
public:
    DeviceStatusVertical(const std::shared_ptr<SensorDataCallback> &sensorCallback) : sensorCallback_(sensorCallback) {};
    ~DeviceStatusVertical();
    void Init();
    void HandleVertical();
    void HandleNonVertical();
    void StartAlgorithm(int32_t sensorTypeId, void* sensorData);
    void RegisterCallback(std::shared_ptr<DevicestatusAlgorithmManagerInterface::DevicestatusAlgorithmCallback> &callback);
    DevicestatusDataUtils::DevicestatusData Report();
private:
    enum EventType {
        VERTICAL = 1,
        NON_VERTICAL
    };
    float x_ = 0;
    float y_ = 0;
    float z_ = 0;
    double pitch_ = 0;

    int state_ = NON_VERTICAL;
    int previousState_ = NON_VERTICAL;
    int counter_ = COUNTER_THRESHOLD;

    DevicestatusDataUtils::DevicestatusData reportInfo_;
    SensorCallback dataCallback_;
    std::shared_ptr<SensorDataCallback> sensorCallback_;
    std::shared_ptr<DevicestatusAlgorithmManagerInterface::DevicestatusAlgorithmCallback> callbackImpl_;
};
} // namespace Msdp
} // namespace OHOS
#endif	// DEVICE_STATUS_VERTICAL_H