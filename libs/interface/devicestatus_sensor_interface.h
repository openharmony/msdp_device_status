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

#ifndef DEVICESTATUS_SENSOR_INTERFACE_H
#define DEVICESTATUS_SENSOR_INTERFACE_H

#include <string>
#include <memory>
#include <map>
#include <errors.h>

#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class DevicestatusSensorInterface {
public:
    DevicestatusSensorInterface() {}
    virtual ~DevicestatusSensorInterface() {}
    class DevicestatusSensorHdiCallback {
    public:
        DevicestatusSensorHdiCallback() = default;
        virtual ~DevicestatusSensorHdiCallback() = default;
        virtual void OnSensorHdiResult(const DevicestatusDataUtils::DevicestatusData& data) = 0;
    };

    virtual void RegisterCallback(const std::shared_ptr<DevicestatusSensorHdiCallback>& callback) = 0;
    virtual void UnregisterCallback() = 0;
    virtual void Enable() = 0;
    virtual void Disable() = 0;
};

struct SensorHdiHandle {
    void* handle;
    DevicestatusSensorInterface* (*create)();
    void* (*destroy)(DevicestatusSensorInterface*);
    DevicestatusSensorInterface* pAlgorithm;
    SensorHdiHandle() : handle(nullptr), create(nullptr), destroy(nullptr), pAlgorithm(nullptr) {}
    ~SensorHdiHandle() {}
    void Clear()
    {
        handle = nullptr;
        create = nullptr;
        destroy = nullptr;
        pAlgorithm = nullptr;
    }
};
}
}
#endif // DEVICESTATUS_SENSOR_INTERFACE_H
