/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef COOPERATE_MANAGER_H
#define COOPERATE_MANAGER_H

#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CooperateManager {
public:
    CooperateManager() = default;
    ~CooperateManager() = default;

    void Enable();
    void Diable();
    int32_t ActivateCooperate(SessionPtr sess,
                              int32_t userData, 
                              const std::string &remoteNetworkId, 
                              int32_t startDeviceId);
    int32_t DeactivateCooperate(SessionPtr sess, int32_t userData, bool isUnchained);
    int32_t GetCooperateState(SessionPtr sess, int32_t userData, const std::string &networkId);
    int32_t RegisterCooperateListener(SessionPtr sess);
    int32_t UnregisterCooperateListener(SessionPtr sess);
    void Dump(int32_t fd);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_MANAGER_H