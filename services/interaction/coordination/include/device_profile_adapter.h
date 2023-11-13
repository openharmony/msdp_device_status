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

#ifndef DEVICE_PROFILE_ADAPTER_H
#define DEVICE_PROFILE_ADAPTER_H

#include <functional>
#include <memory>
#include <vector>

#include "iprofile_event_callback.h"
#include "nocopyable.h"
#include "singleton.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceProfileAdapter final {
    DECLARE_DELAYED_SINGLETON(DeviceProfileAdapter);
    class ProfileEventCallbackImpl final : public DeviceProfile::IProfileEventCallback {
    public:
        void OnProfileChanged(const DeviceProfile::ProfileChangeNotification &changeNotification) override;
        void OnSyncCompleted(const DeviceProfile::SyncResult &syncResults) override;
    };
public:
    using DPCallback = std::function<void(const std::string &, bool)>;
    using ProfileEventCallback = std::shared_ptr<DeviceProfile::IProfileEventCallback>;
    DISALLOW_COPY_AND_MOVE(DeviceProfileAdapter);

    int32_t UpdateCrossingSwitchState(bool state, const std::vector<std::string> &deviceIds);
    int32_t UpdateCrossingSwitchState(bool state);
    bool GetCrossingSwitchState(const std::string &networkId);
    int32_t UnregisterCrossingStateListener(const std::string &networkId);
    int32_t RegisterCrossingStateListener(const std::string &networkId, DPCallback callback);

private:
    void OnProfileChanged(const std::string &networkId);
    int32_t RegisterProfileListener(const std::string &networkId);
    std::mutex adapterLock_;
    std::map<std::string, DeviceProfileAdapter::DPCallback> callbacks_;
    std::map<std::string, DeviceProfileAdapter::ProfileEventCallback> profileEventCallbacks_;
    const std::string characteristicsName_ { "currentStatus" };
};

#define DP_ADAPTER OHOS::DelayedSingleton<DeviceProfileAdapter>::GetInstance()
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#endif // DEVICE_PROFILE_ADAPTER_H