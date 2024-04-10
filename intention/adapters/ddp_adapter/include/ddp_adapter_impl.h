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

#ifndef DDP_ADAPTER_IMPL_H
#define DDP_ADAPTER_IMPL_H

#include <map>
#include <mutex>
#include <set>
#include <unordered_map>
#include <variant>

#include "cJSON.h"
#include "dp_subscribe_info.h"
#include "nocopyable.h"

#include "i_ddp_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::DistributedDeviceProfile;
class DDPAdapterImpl final : public IDDPAdapter, public std::enable_shared_from_this<DDPAdapterImpl> {
    class Observer final {
    public:
        explicit Observer(std::shared_ptr<IDeviceProfileObserver> observer)
            : observer_(observer) {}

        Observer() = default;
        ~Observer() = default;
        DISALLOW_COPY_AND_MOVE(Observer);

        std::shared_ptr<IDeviceProfileObserver> Lock() const noexcept
        {
            return observer_.lock();
        }

        bool operator<(const Observer &other) const noexcept
        {
            return (observer_.lock() < other.observer_.lock());
        }

    private:
        std::weak_ptr<IDeviceProfileObserver> observer_;
    };

    using DPValue = std::variant<
        bool,
        int32_t,
        std::string
    >;

public:
    DDPAdapterImpl() = default;
    ~DDPAdapterImpl() = default;
    DISALLOW_COPY_AND_MOVE(DDPAdapterImpl);

    void AddObserver(std::shared_ptr<IDeviceProfileObserver> observer) override;
    void RemoveObserver(std::shared_ptr<IDeviceProfileObserver> observer) override;
    void AddWatch(const std::string &networkId) override;
    void RemoveWatch(const std::string &networkId) override;
    void OnProfileChanged(const std::string &networkId) override;
    std::string GetNetworkIdByUdId(const std::string &udId) override;
    std::string GetUdIdByNetworkId(const std::string &networkId) override;
    std::string GetLocalNetworkId() override;
    int32_t UpdateCrossingSwitchState(bool state) override;
    int32_t GetCrossingSwitchState(const std::string &udId, bool &state) override;

    int32_t GetProperty(const std::string &udId, const std::string &name, bool &value) override;
    int32_t GetProperty(const std::string &udId, const std::string &name, int32_t &value) override;
    int32_t GetProperty(const std::string &udId, const std::string &name, std::string &value) override;
    int32_t SetProperty(const std::string &name, bool value) override;
    int32_t SetProperty(const std::string &name, int32_t value) override;
    int32_t SetProperty(const std::string &name, const std::string &value) override;

private:
    int32_t RegisterProfileListener(const std::string &networkId);
    int32_t UnregisterProfileListener(const std::string &networkId);
    std::string GetCurrentPackageName();
    std::string GetLocalUdId();
    int32_t GetProperty(const std::string &udId, const std::string &name,
        std::function<int32_t(cJSON *)> parse);
    int32_t SetProperty(const std::string &name, const DPValue &value);
    int32_t GenerateProfileStr(std::string &profileStr);
    int32_t PutServiceProfile();
    int32_t PutCharacteristicProfile(const std::string &profileStr);
    int32_t PutProfile();

private:
    std::mutex mutex_;
    std::set<Observer> observers_;
    std::unordered_map<std::string, std::string> udId2NetworkId_;
    std::map<std::string, DPValue> properties_;
    bool isServiceProfileExist_ { false };
    std::unordered_map<std::string, SubscribeInfo> crossingSwitchSubscribeInfo_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DDP_ADAPTER_IMPL_H
