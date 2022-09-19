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

#ifndef DEVICESTATUS_ALGORITHM_MANAGER_H
#define DEVICESTATUS_ALGORITHM_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <errors.h>
#include "devicestatus_data_utils.h"
#include "devicestatus_algorithm_manager_interface.h"
#include "devicestatus_data_parse.h"
#include "sensor_data_callback.h"
#include "device_status_absolute_still.h"
#include "device_status_horizontal.h"
#include "device_status_vertical.h"

namespace OHOS {
namespace Msdp {
class DevicestatusAlgorithmManager : public DevicestatusAlgorithmManagerInterface {
public:
    DevicestatusAlgorithmManager() {}
    virtual ~DevicestatusAlgorithmManager() {}
    bool Init();

    ErrCode RegisterCallback(std::shared_ptr<DevicestatusAlgorithmCallback>& callback) override;
    ErrCode UnregisterCallback() override;
    ErrCode Enable(const DevicestatusDataUtils::DevicestatusType& type) override;
    ErrCode Disable(const DevicestatusDataUtils::DevicestatusType& type) override;
    ErrCode DisableCount(const DevicestatusDataUtils::DevicestatusType& type) override;

    std::shared_ptr<DevicestatusAlgorithmCallback> GetCallbacksImpl()
    {
        std::unique_lock lock(mutex_);
        return callbacksImpl_;
    }
private:
    int32_t in_type[DevicestatusDataUtils::DevicestatusType::TYPE_VERTICAL_POSITION + 1] = {0};
    std::shared_ptr<DevicestatusAlgorithmCallback> callbacksImpl_;
    std::mutex mutex_;
    DevicestatusDataUtils::DevicestatusType type_;
    std::shared_ptr<SensorDataCallback> sensorEventCb_{nullptr}; 
    std::shared_ptr<AbsoluteStill> still_{nullptr};
    std::shared_ptr<DeviceStatusHorizontal> horizontalPosition_{nullptr};
    std::shared_ptr<DeviceStatusVertical> verticalPosition_{nullptr};
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_ALGORITHM_MANAGER_H