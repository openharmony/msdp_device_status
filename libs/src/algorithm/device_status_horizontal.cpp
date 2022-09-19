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

#include "device_status_horizontal.h"
#include "devicestatus_common.h"
#include <cstdio>
#include <cmath>

namespace OHOS {
namespace Msdp {
DeviceStatusHorizontal::~DeviceStatusHorizontal() {}

void DeviceStatusHorizontal::Init()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    reportInfo_.type = DevicestatusDataUtils::TYPE_INVALID;
    reportInfo_.value = DevicestatusDataUtils::VALUE_INVALID; 
    reportInfo_.status = DevicestatusDataUtils::STATUS_INVALID;
    reportInfo_.action = DevicestatusDataUtils::ACTION_INVALID;
    reportInfo_.move = 0;
    dataCallback_ = std::bind(&DeviceStatusHorizontal::StartAlgorithm, this, std::placeholders::_1, std::placeholders::_2);
    sensorCallback_->SubscribeSensorEvent(dataCallback_);
}

void DeviceStatusHorizontal::HandleHorizontal()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    if (previousState_ != state_) {
        Report();
    }
}

void DeviceStatusHorizontal::HandleNonHorizontal()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
}

void DeviceStatusHorizontal::StartAlgorithm(int32_t sensorTypeId, void* sensorData)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    if (sensorTypeId == SENSOR_TYPE_ID_ACCELEROMETER) {
        AccelData* data = (AccelData*)sensorData;
        x_ = data->y;
        y_ = -(data->x);
        z_ = data->z;
    } else {
        return;
    }
    DEV_HILOGI(SERVICE,
        "acc_x_: %{public}f, acc_y_: %{public}f, acc_z_: %{public}f",
        x_, y_, z_);
    
    if ((abs(x_) < ACCELERATION_VALID_THRESHOLD) && (abs(y_) < ACCELERATION_VALID_THRESHOLD) && 
        (abs(z_) < ACCELERATION_VALID_THRESHOLD)) {

        vectorModule_ = sqrt((x_ * x_) + (y_ * y_) + (z_ * z_));
        pitch_ = -atan2(y_, z_) * (ANGLE_ONE_HUNDRED_AND_EIGHTY_DEGREE / PI);
        roll_ = atan2(x_, z_) * (ANGLE_ONE_HUNDRED_AND_EIGHTY_DEGREE / PI);

        if ((abs(pitch_) < ANGLE_TWENTY_DEGREE) && (abs(roll_) < ANGLE_TWENTY_DEGREE)) {
            counter_--;
            if (counter_ == 0) {
                counter_ = COUNTER_THRESHOLD;
                previousState_ = state_;
                state_ = HORIZONTAL;
            } else if (counter_) {
                previousState_ = state_;
                return;
            }
        } else {
            counter_ = COUNTER_THRESHOLD;
            previousState_ = state_;
            state_ = NON_HORIZONTAL;
        }

        switch (state_) {
            case HORIZONTAL:
                HandleHorizontal();
                break;
            case NON_HORIZONTAL: {
                HandleNonHorizontal();
                break;
            }
            default:
                break;
        }
    }
}

void  DeviceStatusHorizontal::RegisterCallback(std::shared_ptr<DevicestatusAlgorithmManagerInterface::DevicestatusAlgorithmCallback> &callback)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    callbackImpl_ = callback;
}

DevicestatusDataUtils::DevicestatusData DeviceStatusHorizontal::Report()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    reportInfo_.type = DevicestatusDataUtils::TYPE_HORIZONTAL_POSITION;
    reportInfo_.value = DevicestatusDataUtils::VALUE_ENTER;
    reportInfo_.action = DevicestatusDataUtils::ACTION_ENLARGE;
    reportInfo_.status = DevicestatusDataUtils::STATUS_START;
    reportInfo_.move = 0.0;
    DEV_HILOGI(SERVICE, "%{public}s: deviceStatusData.type:%{public}d, \
        deviceStatusData.status: %{public}d, deviceStatusData.action: %{public}d, deviceStatusData.move: %{public}f",\
        __func__,  static_cast<int>(reportInfo_.type), \
        static_cast<int>(reportInfo_.status), static_cast<int>(reportInfo_.action), reportInfo_.move);
    if (callbackImpl_ != nullptr) {
        callbackImpl_->OnAlogrithmResult(reportInfo_);
    } else {
        DEV_HILOGI(SERVICE, "callbackImpl_ is null"); 
    }
    return reportInfo_;
}
}
}