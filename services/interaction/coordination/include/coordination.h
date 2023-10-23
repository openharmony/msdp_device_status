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

#ifndef COORDINATION_H
#define COORDINATION_H

#include "i_coordination.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Coordination : public ICoordination {
public:
    Coordination() = default;
    virtual ~Coordination() = default;

    void PrepareCoordination();
    void UnprepareCoordination();
    int32_t ActivateCoordination(SessionPtr sess, int32_t userData,
        const std::string &remoteNetworkId, int32_t startDeviceId);
    int32_t DeactivateCoordination(SessionPtr sess, int32_t userData, bool isUnchained);
    int32_t GetCoordinationState(SessionPtr sess, int32_t userData, const std::string &networkId);
    int32_t RegisterCoordinationListener(SessionPtr sess);
    int32_t UnregisterCoordinationListener(SessionPtr sess);
    void Dump(int32_t fd);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COORDINATION_H