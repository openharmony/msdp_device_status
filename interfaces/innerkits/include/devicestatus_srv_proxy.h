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

#ifndef DEVICESTATUS_SRV_PROXY_H
#define DEVICESTATUS_SRV_PROXY_H

#include <nocopyable.h>

#include "iremote_proxy.h"
#include "idevicestatus.h"

namespace OHOS {
namespace Msdp {
class DevicestatusSrvProxy : public IRemoteProxy<Idevicestatus> {
public:
    explicit DevicestatusSrvProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<Idevicestatus>(impl) {}
    ~DevicestatusSrvProxy() = default;
    DISALLOW_COPY_AND_MOVE(DevicestatusSrvProxy);

    virtual void Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    virtual void UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    virtual DevicestatusDataUtils::DevicestatusData GetCache(const \
        DevicestatusDataUtils::DevicestatusType& type) override;

private:
    static inline BrokerDelegator<DevicestatusSrvProxy> delegator_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SRV_PROXY_H
