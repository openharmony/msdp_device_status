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

#ifndef COOPERATE_H
#define COOPERATE_H

#include <atomic>
#include <iostream>
#include <string>
#include <thread>

#include "state_machine.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Cooperate {
public:
    int32_t Init();
    int32_t Enable();
    void Disable();
    int32_t StartCooperate(int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId);
    int32_t StopCooperate(int32_t userData, bool isUnchained);

private:
    void Loop();

    StateMachine sm_;
    std::thread worker_;
    std::atomic_bool running_ { false };
    Channel<CooperateEvent>::Sender sender_;
    Channel<CooperateEvent>::Receiver receiver_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_H