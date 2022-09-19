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

#ifndef DEVICESTATUS_ALGORITHM_MANAGER_INTERFACE_H
#define DEVICESTATUS_ALGORITHM_MANAGER_INTERFACE_H

#include <string>
#include <memory>
#include <map>
#include <errors.h>

#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class DevicestatusAlgorithmManagerInterface {
public:
    DevicestatusAlgorithmManagerInterface() {}
    virtual ~DevicestatusAlgorithmManagerInterface() {}
    class DevicestatusAlgorithmCallback {
    public:
        DevicestatusAlgorithmCallback() = default;
        virtual ~DevicestatusAlgorithmCallback() = default;
        virtual void OnAlogrithmResult(const DevicestatusDataUtils::DevicestatusData& data) = 0;
    };

    virtual ErrCode RegisterCallback(std::shared_ptr<DevicestatusAlgorithmCallback>& callback);
    virtual ErrCode UnregisterCallback();
    virtual ErrCode Enable(const DevicestatusDataUtils::DevicestatusType& type);
    virtual ErrCode Disable(const DevicestatusDataUtils::DevicestatusType& type);
    virtual ErrCode DisableCount(const DevicestatusDataUtils::DevicestatusType& type);
};

struct DevicestatusAlgorithmHandle {
    void* handle;
    DevicestatusAlgorithmManagerInterface* (*create)();
    void* (*destroy)(DevicestatusAlgorithmManagerInterface*);
    DevicestatusAlgorithmManagerInterface* pAlgorithm;
    DevicestatusAlgorithmHandle() : handle(nullptr), create(nullptr), destroy(nullptr), pAlgorithm(nullptr) {}
    ~DevicestatusAlgorithmHandle() {}
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
#endif // DEVICESTATUS_ALGORITHM_MANAGER_INTERFACE_H
