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

#ifndef IDEVICESTATUS_H
#define IDEVICESTATUS_H

#include <iremote_broker.h>

#include "iremote_object.h"
#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class Idevicestatus : public IRemoteBroker {
public:
    enum {
        DEVICESTATUS_SUBSCRIBE = 0,
        DEVICESTATUS_UNSUBSCRIBE,
        DEVICESTATUS_GETCACHE
    };

    virtual void Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) = 0;
    virtual void UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) = 0;
    virtual DevicestatusDataUtils::DevicestatusData GetCache(const DevicestatusDataUtils::DevicestatusType& type) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.msdp.Idevicestatus");
};
} // namespace Msdp
} // namespace OHOS
#endif // IDEVICESTATUS_H
