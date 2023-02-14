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

#ifndef DEVICESTATUS_ALGORITHM_MANAGER_H
#define DEVICESTATUS_ALGORITHM_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "algo_absolute_still.h"
#include "algo_relative_still.h"
#include "devicestatus_data_define.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_msdp_interface.h"
#include "sensor_data_callback.h"

namespace OHOS {
namespace Msdp {
class AlgoMgr final : public DevicestatusMsdpInterface {
public:
    AlgoMgr() = default;
    virtual ~AlgoMgr() = default;

    ErrCode RegisterCallback(std::shared_ptr<MsdpAlgorithmCallback> callback) override;
    ErrCode UnregisterCallback() override;
    ErrCode Enable(DevicestatusDataUtils::DevicestatusType type) override;
    ErrCode Disable(DevicestatusDataUtils::DevicestatusType type) override;

    bool Init();
    ErrCode UnregisterSensor(DevicestatusDataUtils::DevicestatusType type);
    bool CheckSensorTypeId(int32_t sensorTypeId);
    bool StartSensor(DevicestatusDataUtils::DevicestatusType type);
    std::optional<int32_t> GetSensorTypeId(DevicestatusDataUtils::DevicestatusType type);
private:
    int32_t type_[DevicestatusDataUtils::TYPE_MAX] = { 0 };
    std::mutex mutex_;
    std::shared_ptr<AlgoAbsoluteStill> still_ { nullptr };
    std::shared_ptr<AlgoRelativeStill> relativeStill_ { nullptr };
    DevicestatusDataUtils::DevicestatusType algoType_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_ALGORITHM_MANAGER_H
