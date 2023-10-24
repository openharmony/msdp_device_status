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

#ifndef I_COOPERATE_STATE_H
#define I_COOPERATE_STATE_H

#include <atomic>
#include <map>
#include <mutex>
#include <set>
#include <string>

#include "cooperate_event_handler.h"
#include "cooperate_event_manager.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class ICooperateState {
public:
    ICooperateState();
    virtual ~ICooperateState() = default;
    virtual int32_t ActivateCooperate(const std::string &remoteNetworkId, int32_t startDeviceId)
    {
        return static_cast<int32_t>(CooperateMessage::COOPERATE_FAIL);
    }
    virtual int32_t DeactivateCooperate(const std::string &networkId, bool isUnchained,
        const std::pair<std::string, std::string> &preparedNetworkId)
    {
        return static_cast<int32_t>(CooperateMessage::COOPERATE_FAIL);
    }
    virtual void OnKeyboardOnline(const std::string &dhid, const std::pair<std::string, std::string> &networkIds) {}
    virtual void SetStartDeviceDhid(const std::string &startDeviceDhid) {}

protected:
    int32_t PrepareAndStart(const std::string &remoteNetworkId, int32_t startDeviceId);
    bool NeedPrepare(const std::string &remoteNetworkId, const std::string &originNetworkId);
    void OnPrepareDistributedInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId);
    int32_t StartRemoteInput(int32_t startDeviceId);
    virtual void OnStartRemoteInput(bool isSuccess, const std::string &remoteNetworkId, int32_t startDeviceId);

protected:
    std::shared_ptr<AppExecFwk::EventRunner> runner_ { nullptr };
    std::shared_ptr<CooperateEventHandler> eventHandler_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_COOPERATE_STATE_H
