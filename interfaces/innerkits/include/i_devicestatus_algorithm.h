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

#ifndef I_DEVICESTATUS_ALGORITHM_H
#define I_DEVICESTATUS_ALGORITHM_H

#include <iremote_broker.h>

#include "i_devicestatus_algorithm_callback.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IdevicestatusAlgorithm : public IRemoteBroker {
public:
    virtual bool Enable() = 0;
    virtual bool Disable() = 0;
    virtual bool Subscribe(const sptr<IdevicestatusAlgorithmCallback>& callback) = 0;
    virtual bool Unsubscribe(const sptr<IdevicestatusAlgorithmCallback>& callback) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.msdp.IdevicestatusAlgorithm");
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_DEVICESTATUS_ALGORITHM_H
