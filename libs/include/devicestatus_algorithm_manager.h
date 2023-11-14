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

#ifndef DEVICESTATUS_ALGORITHM_MANAGER_H
#define DEVICESTATUS_ALGORITHM_MANAGER_H

#ifdef DEVICE_STATUS_SENSOR_ENABLE
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "algo_absolute_still.h"
#include "algo_horizontal.h"
#include "algo_vertical.h"
#include "devicestatus_data_define.h"
#include "devicestatus_msdp_interface.h"
#include "sensor_data_callback.h"
#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class AlgoMgr final : public IMsdp {
public:
    AlgoMgr() = default;
    virtual ~AlgoMgr() = default;

    bool Init();
    ErrCode RegisterCallback(std::shared_ptr<MsdpAlgoCallback> callback) override;
    ErrCode UnregisterCallback() override;
    ErrCode Enable(Type type) override;
    ErrCode Disable(Type type) override;
    ErrCode DisableCount(Type type) override;
    ErrCode UnregisterSensor(Type type);
    std::shared_ptr<MsdpAlgoCallback> GetCallbackImpl()
    {
        std::unique_lock lock(mutex_);
        return callback_;
    }
    bool CheckSensorTypeId(int32_t sensorTypeId);
    bool StartSensor(Type type);
    int32_t GetSensorTypeId(Type type);
private:
    int32_t type_[Type::TYPE_MAX] { 0 };
    std::shared_ptr<MsdpAlgoCallback> callback_ { nullptr };
    std::mutex mutex_;
    std::shared_ptr<AlgoAbsoluteStill> still_ { nullptr };
    std::shared_ptr<AlgoHorizontal> horizontalPosition_ { nullptr };
    std::shared_ptr<AlgoVertical> verticalPosition_ { nullptr };
    std::map<Type, int32_t> callAlgoNums_ {};
    Type algoType_ { TYPE_INVALID };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE
#endif // DEVICESTATUS_ALGORITHM_MANAGER_H