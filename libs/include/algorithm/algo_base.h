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

#ifndef ALGO_BASE_H
#define ALGO_BASE_H

#include <cmath>
#include <cstdio>
#include <iostream>

#include "devicestatus_common.h"
#include "devicestatus_data_define.h"
#include "stationary_data.h"
#include "devicestatus_msdp_interface.h"
#include "sensor_data_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class AlgoBase {
public:
    AlgoBase() {};
    virtual ~AlgoBase() = default;

    virtual bool Init(Type type) = 0;
    void Unsubscribe(int32_t sensorTypeId);
    void RegisterCallback(const std::shared_ptr<IMsdp::MsdpAlgoCallback> callback);

protected:
    virtual bool StartAlgorithm(int32_t sensorTypeId, AccelData* sensorData) = 0;
    bool GetData(int32_t sensorTypeId, AccelData* sensorData);
    virtual void ExecuteOperation() = 0;
    void UpdateStateAndReport(OnChangedValue value, int32_t state, Type type);

    enum {
        UNKNOWN = -1,
        STILL,
        UNSTILL,
        HORIZONTAL,
        NON_HORIZONTAL,
        VERTICAL,
        NON_VERTICAL
    };

    struct {
        float x;
        float y;
        float z;
        double resultantAcc;
        double pitch;
        double roll;
    } algoPara_ {};

    int32_t state_ = UNKNOWN;
    int32_t counter_ = COUNTER_THRESHOLD;

    Data reportInfo_ {TYPE_INVALID,
                      VALUE_INVALID,
                      STATUS_INVALID,
                      ACTION_INVALID,
                      0.0};

    SensorCallback algoCallback_;
    std::shared_ptr<IMsdp::MsdpAlgoCallback> callback_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // ALGO_BASE_H
