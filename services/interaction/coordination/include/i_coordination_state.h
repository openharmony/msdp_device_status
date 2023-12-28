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

#ifndef I_COORDINATION_STATE_H
#define I_COORDINATION_STATE_H

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <string>

#include "coordination_event_handler.h"
#include "coordination_event_manager.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class ICoordinationState {
public:
    ICoordinationState();
    virtual ~ICoordinationState() = default;
    virtual int32_t ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId)
    {
        return COOPERATOR_FAIL;
    }
    virtual int32_t DeactivateCoordination(const std::string &networkId, bool isUnchained,
        const std::pair<std::string, std::string> &preparedNetworkId)
    {
        return COOPERATOR_FAIL;
    }
    virtual void SetStartDeviceDhid(const std::string &startDeviceDhid) {}
    virtual void OnKeyboardOnline(const std::string &dhid, const std::pair<std::string, std::string> &networkIds) {}

protected:
    int32_t PrepareAndStart(const std::string &remoteNetworkId, int32_t startDeviceId);
    void OnPrepareDistributedInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId);
    bool NeedPrepare(const std::string &remoteNetworkId, const std::string &originNetworkId);
    int32_t StartRemoteInput(int32_t startDeviceId);
    virtual void OnStartRemoteInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId);

protected:
    std::shared_ptr<AppExecFwk::EventRunner> runner_ { nullptr };
    std::shared_ptr<CoordinationEventHandler> eventHandler_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_COORDINATION_STATE_H
