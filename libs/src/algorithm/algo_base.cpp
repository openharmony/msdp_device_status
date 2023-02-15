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

#include "algo_base.h"

namespace OHOS {
namespace Msdp {
void AlgoBase::Unsubscribe(int32_t sensorTypeId)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (algoCallback_ == nullptr) {
        DEV_HILOGE(SERVICE, "algoCallback is nullptr");
        return;
    }
    SensorDataCallback::GetInstance().UnsubscribeSensorEvent(sensorTypeId, algoCallback_);
}

bool AlgoBase::GetData(int32_t sensorTypeId, AccelData* sensorData)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        DEV_HILOGE(SERVICE, "sensorTypeId:%{public}d is invalid", sensorTypeId);
        return false;
    }
    if (sensorData == nullptr) {
        DEV_HILOGE(SERVICE, "sensorData is nullptr");
        return false;
    }

    AccelData* data = sensorData;
    if ((abs(data->x) > ACC_VALID_THRHD) ||
        (abs(data->y) > ACC_VALID_THRHD) ||
        (abs(data->z) > ACC_VALID_THRHD)) {
        DEV_HILOGE(SERVICE, "Acc data is invalid");
        return false;
    }

    algoPara_.x = data->y;
    algoPara_.y = data->x;
    algoPara_.z = -(data->z);
    DEV_HILOGD(SERVICE, "x:%{public}f, y:%{public}f, z:%{public}f", algoPara_.x, algoPara_.y, algoPara_.z);
    return true;
}

void AlgoBase::RegisterCallback(std::shared_ptr<DevicestatusMsdpInterface::MsdpAlgorithmCallback> callback)
{
    DEV_HILOGD(SERVICE, "Enter");
    state_ = UNKNOWN;
    callback_ = callback;
}

void AlgoBase::UnregisterCallback()
{
    DEV_HILOGD(SERVICE, "Enter");
    callback_ = nullptr;
}

void AlgoBase::UpdateStateAndReport(DevicestatusDataUtils::DevicestatusValue value,
    int32_t state, DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGD(SERVICE, "Enter type:%{public}d,value:%{public}d", type, value);
    if (callback_ == nullptr) {
        DEV_HILOGE(SERVICE, "callback_ is nullptr");
        return;
    }
    state_ = state;
    reportInfo_.type = type;
    reportInfo_.value = value;
    callback_->OnResult(reportInfo_);
}
} // namespace Msdp
} // namespace OHOS
