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
#include "algo_vertical.h"

#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "AlgoVertical"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr float JUDGE_FLOAT { 1e-6 };
} // namespace

bool AlgoVertical::Init(Type type)
{
    CALL_DEBUG_ENTER;
    algoCallback_ = std::bind(&AlgoVertical::StartAlgorithm, this, std::placeholders::_1, std::placeholders::_2);
    CHKPF(algoCallback_);
    SENSOR_DATA_CB.SubscribeSensorEvent(type, algoCallback_);
    return true;
}

bool AlgoVertical::StartAlgorithm(int32_t sensorTypeId, AccelData* sensorData)
{
    CALL_DEBUG_ENTER;
    if (!SetData(sensorTypeId, sensorData)) {
        FI_HILOGE("Failed to get data");
        return false;
    }
    ExecuteOperation();
    return true;
}

void AlgoVertical::ExecuteOperation()
{
    CALL_DEBUG_ENTER;
    if ((abs(algoPara_.y) <= JUDGE_FLOAT) && (abs(algoPara_.z) <= JUDGE_FLOAT)) {
        return;
    }
    algoPara_.pitch = -atan2(algoPara_.y, algoPara_.z) * (ANGLE_180_DEGREE / PI);
    algoPara_.roll = atan2(algoPara_.x, algoPara_.z) * (ANGLE_180_DEGREE / PI);
    FI_HILOGD("pitch:%{public}f, roll:%{public}f", algoPara_.pitch, algoPara_.roll);

    if (((abs(algoPara_.pitch) > ANGLE_VER_LOW_THRHD) && (abs(algoPara_.pitch) < ANGLE_VER_UP_THRHD)) ||
        ((abs(algoPara_.roll) > ANGLE_VER_LOW_THRHD) && (abs(algoPara_.roll) < ANGLE_VER_UP_THRHD))) {
        if (state_ == VERTICAL) {
            return;
        }
        counter_--;
        if (counter_ == 0) {
            counter_ = COUNTER_THRESHOLD;
            UpdateStateAndReport(VALUE_ENTER, VERTICAL, TYPE_VERTICAL_POSITION);
        }
    } else {
        counter_ = COUNTER_THRESHOLD;
        if (state_ == NON_VERTICAL) {
            return;
        }
        UpdateStateAndReport(VALUE_EXIT, NON_VERTICAL, TYPE_VERTICAL_POSITION);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE