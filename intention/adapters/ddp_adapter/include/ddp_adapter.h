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

#ifndef DDP_ADAPTER_H
#define DDP_ADAPTER_H

#include "nocopyable.h"

#include "i_ddp_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DDPAdapter final : public IDDPAdapter {
public:
    DDPAdapter();
    ~DDPAdapter() = default;
    DISALLOW_COPY_AND_MOVE(DDPAdapter);

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
    std::shared_ptr<IDDPAdapter> ddp_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DDP_ADAPTER_H
