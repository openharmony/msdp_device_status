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

#ifndef ALGO_BASE_H
#define ALGO_BASE_H

#include <cmath>
#include <cstdio>
#include <iostream>

#include "devicestatus_common.h"
#include "devicestatus_data_define.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_msdp_interface.h"
#include "sensor_data_callback.h"

namespace OHOS {
namespace Msdp {
class AlgoBase {
public:
    AlgoBase() = default;
    virtual ~AlgoBase() = default;

    virtual bool Init(DevicestatusDataUtils::DevicestatusType type) = 0;
    void Unsubscribe(int32_t sensorTypeId);
    void RegisterCallback(std::shared_ptr<DevicestatusMsdpInterface::MsdpAlgorithmCallback> callback);
    void UnregisterCallback();
protected:
    virtual bool StartAlgorithm(int32_t sensorTypeId, AccelData* sensorData) = 0;
    bool GetData(int32_t sensorTypeId, AccelData* sensorData);
    virtual void ExecuteOperation() = 0;
    void UpdateStateAndReport(DevicestatusDataUtils::DevicestatusValue value,
        int32_t state, DevicestatusDataUtils::DevicestatusType type);

    enum {
        UNKNOWN = -1,
        STILL,
        NON_STILL,
        RELATIVE_STILL,
        NON_RELATIVE_STILL,
        HORIZONTAL,
        NON_HORIZONTAL,
        VERTICAL,
        NON_VERTICAL
    };
    int32_t state_ = UNKNOWN;
    int32_t counter_ = COUNTER_THRESHOLD;
    DevicestatusDataUtils::AlgoData algoPara_ {};
    DevicestatusDataUtils::DevicestatusData reportInfo_;
    SensorCallback algoCallback_ { nullptr };
    std::shared_ptr<DevicestatusMsdpInterface::MsdpAlgorithmCallback> callback_ { nullptr };
};
} // namespace Msdp
} // namespace OHOS
#endif // ALGO_BASE_H
