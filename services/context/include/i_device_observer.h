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

#ifndef OHOS_MSDP_DEVICE_STATUS_I_DEVICE_OBSERVER_H
#define OHOS_MSDP_DEVICE_STATUS_I_DEVICE_OBSERVER_H

#include <memory>

#include "i_device.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDeviceObserver {
public:
    IDeviceObserver() = default;
    virtual ~IDeviceObserver() = default;

    virtual void OnDeviceAdded(std::shared_ptr<IDevice>) = 0;
    virtual void OnDeviceRemoved(std::shared_ptr<IDevice>) = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // OHOS_MSDP_DEVICE_STATUS_I_DEVICE_OBSERVER_H