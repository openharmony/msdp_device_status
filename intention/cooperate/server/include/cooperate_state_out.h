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

#ifndef COOPERATE_STATE_OUT_H
#define COOPERATE_STATE_OUT_H

#include "i_cooperate_state.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CooperateStateOut final : public ICooperateState {
public:
    CooperateStateOut() = default;
    explicit CooperateStateOut(const std::string &startDeviceDhid);
    int32_t DeactivateCooperate(const std::string &remoteNetworkId, bool isUnchained,
        const std::pair<std::string, std::string> &preparedNetworkId) override;
    void OnKeyboardOnline(const std::string &dhid, const std::pair<std::string, std::string> &networkIds) override;
    void SetStartDeviceDhid(const std::string &startDeviceDhid) override;

private:
    void OnStopRemoteInput(bool isSuccess, const std::string &remoteNetworkId);
    void ProcessStop(const std::string &remoteNetworkId);
    std::string startDeviceDhid_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_STATE_OUT_H
