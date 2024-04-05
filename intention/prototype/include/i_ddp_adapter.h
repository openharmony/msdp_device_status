/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef I_DDP_ADAPTER_H
#define I_DDP_ADAPTER_H

#include <memory>
#include <string>

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IDeviceProfileObserver {
public:
    IDeviceProfileObserver() = default;
    virtual ~IDeviceProfileObserver() = default;

    virtual void OnProfileChanged(const std::string &networkId) = 0;
};

class IDDPAdapter {
public:
    IDDPAdapter() = default;
    virtual ~IDDPAdapter() = default;

    virtual void AddObserver(std::shared_ptr<IDeviceProfileObserver> observer) = 0;
    virtual void RemoveObserver(std::shared_ptr<IDeviceProfileObserver> observer) = 0;
    virtual void AddWatch(const std::string &networkId) = 0;
    virtual void RemoveWatch(const std::string &networkId) = 0;
    virtual void OnProfileChanged(const std::string &networkId) = 0;
    virtual std::string GetNetworkIdByUdId(const std::string &udId) = 0;
    virtual std::string GetUdIdByNetworkId(const std::string &networkId) = 0;
    virtual std::string GetLocalNetworkId() = 0;
    virtual int32_t UpdateCrossingSwitchState(bool state) = 0;
    virtual int32_t GetCrossingSwitchState(const std::string &udId, bool &state) = 0;

    virtual int32_t GetProperty(const std::string &udId, const std::string &name, bool &value) = 0;
    virtual int32_t GetProperty(const std::string &udId, const std::string &name, int32_t &value) = 0;
    virtual int32_t GetProperty(const std::string &udId, const std::string &name, std::string &value) = 0;
    virtual int32_t SetProperty(const std::string &name, bool value) = 0;
    virtual int32_t SetProperty(const std::string &name, int32_t value) = 0;
    virtual int32_t SetProperty(const std::string &name, const std::string &value) = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_DDP_ADAPTER_H
