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
#include "algo_base.h"

#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "AlgoBase"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

void AlgoBase::Unsubscribe(int32_t sensorTypeId)
{
    CALL_DEBUG_ENTER;
    CHKPV(algoCallback_);
    SENSOR_DATA_CB.UnsubscribeSensorEvent(sensorTypeId, algoCallback_);
}

bool AlgoBase::SetData(int32_t sensorTypeId, AccelData* sensorData)
{
    CALL_DEBUG_ENTER;
    if (sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        FI_HILOGE("sensorTypeId:%{public}d", sensorTypeId);
        return false;
    }
    CHKPF(sensorData);
    AccelData* data = sensorData;
    if ((abs(data->x) > ACC_VALID_THRHD) ||
        (abs(data->y) > ACC_VALID_THRHD) ||
        (abs(data->z) > ACC_VALID_THRHD)) {
        FI_HILOGE("Acc data is invalid");
        return false;
    }

    algoPara_.x = data->y;
    algoPara_.y = data->x;
    algoPara_.z = -(data->z);
    FI_HILOGD("x:%{public}f, y:%{public}f, z:%{public}f", algoPara_.x, algoPara_.y, algoPara_.z);
    return true;
}

void AlgoBase::RegisterCallback(const std::shared_ptr<IMsdp::MsdpAlgoCallback> callback)
{
    CALL_DEBUG_ENTER;
    callback_ = callback;
}

void AlgoBase::UpdateStateAndReport(OnChangedValue value, int32_t state, Type type)
{
    CALL_DEBUG_ENTER;
    CHKPV(callback_);
    state_ = state;
    reportInfo_.type = type;
    reportInfo_.value = value;
    FI_HILOGI("type:%{public}d, value:%{public}d", type, value);
    callback_->OnResult(reportInfo_);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE
