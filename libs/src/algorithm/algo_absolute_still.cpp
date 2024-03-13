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
#include "algo_absolute_still.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "AlgoAbsoluteStill"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

bool AlgoAbsoluteStill::Init(Type type)
{
    CALL_DEBUG_ENTER;
    algoCallback_ = std::bind(&AlgoAbsoluteStill::StartAlgorithm, this, std::placeholders::_1, std::placeholders::_2);
    if (algoCallback_ == nullptr) {
        FI_HILOGE("algoCallback is nullptr");
        return false;
    }
    SENSOR_DATA_CB.SubscribeSensorEvent(type, algoCallback_);
    return true;
}

bool AlgoAbsoluteStill::StartAlgorithm(int32_t sensorTypeId, AccelData* sensorData)
{
    CALL_DEBUG_ENTER;
    if (!SetData(sensorTypeId, sensorData)) {
        FI_HILOGE("Failed to get data");
        return false;
    }
    ExecuteOperation();
    return true;
}

void AlgoAbsoluteStill::ExecuteOperation()
{
    CALL_DEBUG_ENTER;
    algoPara_.resultantAcc =
        sqrt((algoPara_.x * algoPara_.x) + (algoPara_.y * algoPara_.y) + (algoPara_.z * algoPara_.z));
    FI_HILOGD("resultantAcc:%{public}f", algoPara_.resultantAcc);
    if ((algoPara_.resultantAcc > RESULTANT_ACC_LOW_THRHD) && (algoPara_.resultantAcc < RESULTANT_ACC_UP_THRHD)) {
        if (state_ == STILL) {
            return;
        }
        counter_--;
        if (counter_ == 0) {
            counter_ = COUNTER_THRESHOLD;
            UpdateStateAndReport(VALUE_ENTER, STILL, TYPE_ABSOLUTE_STILL);
        }
    } else {
        counter_ = COUNTER_THRESHOLD;
        if (state_ == UNSTILL) {
            return;
        }
        UpdateStateAndReport(VALUE_EXIT, UNSTILL, TYPE_ABSOLUTE_STILL);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE
