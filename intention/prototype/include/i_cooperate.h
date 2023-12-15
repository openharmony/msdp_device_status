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

#ifndef I_COOPERATE_H
#define I_COOPERATE_H

#include <string>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class ICooperate {
public:
    ICooperate() = default;
    virtual ~ICooperate() = default;

    virtual int32_t RegisterListener(int32_t pid) = 0;
    virtual int32_t UnregisterListener(int32_t pid) = 0;
    virtual int32_t RegisterHotAreaListener(int32_t pid) = 0;
    virtual int32_t UnregisterHotAreaListener(int32_t pid) = 0;

    virtual int32_t Enable(int32_t pid) = 0;
    virtual int32_t Disable(int32_t pid) = 0;
    virtual int32_t Start(int32_t pid, int32_t userData,
        const std::string &remoteNetworkId, int32_t startDeviceId) = 0;
    virtual int32_t Stop(int32_t pid, int32_t userData, bool isUnchained) = 0;

    virtual int32_t GetCooperateState(int32_t pid, int32_t userData, const std::string &networkId) = 0;

    virtual void Dump(int32_t fd) = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_COOPERATE_H
