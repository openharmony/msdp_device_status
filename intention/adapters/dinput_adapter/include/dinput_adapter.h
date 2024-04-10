/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef DINPUT_ADAPTER_H
#define DINPUT_ADAPTER_H

#include "nocopyable.h"

#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DInputAdapter final : public IDInputAdapter {
public:
    DInputAdapter(IContext *env);
    ~DInputAdapter() = default;
    DISALLOW_COPY_AND_MOVE(DInputAdapter);

    bool IsNeedFilterOut(const std::string &networkId, BusinessEvent &&event) override;

    int32_t StartRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback) override;
    int32_t StopRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback) override;

    int32_t StopRemoteInput(const std::string &originNetworkId,
        const std::vector<std::string> &inputDeviceDhids, DInputCallback callback) override;

    int32_t PrepareRemoteInput(const std::string &remoteNetworkId,
        const std::string &originNetworkId, DInputCallback callback) override;
    int32_t UnPrepareRemoteInput(const std::string &remoteNetworkId,
        const std::string &originNetworkId, DInputCallback callback) override;

    int32_t PrepareRemoteInput(const std::string &networkId, DInputCallback callback) override;
    int32_t UnPrepareRemoteInput(const std::string &networkId, DInputCallback callback) override;
    int32_t RegisterSessionStateCb(std::function<void(uint32_t)> callback) override;
    int32_t UnregisterSessionStateCb() override;

private:
    std::shared_ptr<IDInputAdapter> dinput_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DINPUT_ADAPTER_H
