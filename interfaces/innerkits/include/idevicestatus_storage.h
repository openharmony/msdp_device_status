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

#ifndef IDEVICESTATUS_STORAGE_H
#define IDEVICESTATUS_STORAGE_H

#include <iremote_broker.h>

#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class IdevicestatusStorage : public IRemoteBroker {
public:
    struct DevicestatusRecord {
        DevicestatusDataUtils::DevicestatusType type;
        DevicestatusDataUtils::DevicestatusValue value;
        std::string date;
        std::string time;
    };

    virtual bool Save(DevicestatusRecord record) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.msdp.IdevicestatusStorage");
};
} // namespace Msdp
} // namespace OHOS
#endif // IDEVICESTATUS_STORAGE_H