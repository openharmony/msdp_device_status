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

#ifndef I_COORDINATION_H
#define I_COORDINATION_H

#include <functional>
#include <memory>
#include <vector>

#include "i_context.h"
#include "key_event.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class ICoordination {
public:
    virtual void PrepareCoordination() = 0;
    virtual void UnprepareCoordination() = 0;
    virtual int32_t ActivateCoordination(SessionPtr sess, int32_t userData,
        const std::string &remoteNetworkId, int32_t startDeviceId) = 0;
    virtual int32_t DeactivateCoordination(SessionPtr sess, int32_t userData, bool isUnchained) = 0;
    virtual int32_t GetCoordinationState(SessionPtr sess, int32_t userData, const std::string &networkId) = 0;
    virtual int32_t RegisterCoordinationListener(SessionPtr sess) = 0;
    virtual int32_t UnregisterCoordinationListener(SessionPtr sess) = 0;
    virtual void Dump(int32_t fd) = 0;
};

extern "C" ICoordination* CreateICoordination(IContext *context);
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_COORDINATION_H