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

#ifndef DEVICESTATUS_CALLBACK_PROXY_H
#define DEVICESTATUS_CALLBACK_PROXY_H

#include <iremote_proxy.h>
#include <nocopyable.h>

#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"

namespace OHOS {
namespace Msdp {
class DevicestatusCallbackProxy : public IRemoteProxy<IdevicestatusCallback> {
public:
    explicit DevicestatusCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IdevicestatusCallback>(impl) {}
    ~DevicestatusCallbackProxy() = default;
    DISALLOW_COPY_AND_MOVE(DevicestatusCallbackProxy);
    virtual void OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData& devicestatusData) override;

private:
    static inline BrokerDelegator<DevicestatusCallbackProxy> delegator_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_CALLBACK_PROXY_H
