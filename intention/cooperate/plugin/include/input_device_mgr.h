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

#ifndef COOPERATE_INPUT_DEVICE_MANAGER_H
#define COOPERATE_INPUT_DEVICE_MANAGER_H

#include <memory>
#include <unordered_map>
#include <vector>

#include "parcel.h"

#include "device.h"
#include "nocopyable.h"

#include "cooperate_events.h"
#include "i_context.h"
#include "net_packet.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

using SoftbusParcelHandleType = void (InputDeviceMgr::*)(const std::string &networkId, Parcel &parcel);
class InputDeviceMgr {
class DSoftbusObserver final : public IDSoftbusObserver {
    public:
        DSoftbusObserver(InputEventBuilder &parent) : parent_(parent) {}
        ~DSoftbusObserver() = default;

        void OnBind(const std::string &networkId) override {}

        void OnShutdown(const std::string &networkId) override {}

        bool OnPacket(const std::string &networkId, Msdp::NetPacket &packet) override
        {
            return parent_.OnPacket(networkId, packet);
        }

        bool OnRawData(const std::string &networkId, const void *data, uint32_t dataLen) override
        {
            return parent_.OnRawData(networkId, data, dataLen);
        }

    private:
        InputDeviceMgr &parent_;
    };

public:
    InputDeviceMgr(IContext *context);
    ~InputDeviceMgr() = default;
    DISALLOW_COPY_AND_MOVE(InputDeviceMgr);

private:
    void AttachSender(Channel<CooperateEvent>::Sender sender);
    bool OnRawData(const std::string &networkId, const void *data, uint32_t dataLen);
    bool OnParcel(const std::string &networkId, SoftbusMessageId messageId, Parcel &parcel);
    void OnSessionOpened(const DSoftbusSessionOpened &notice);
    void OnSessionClosed(const DSoftbusSessionClosed &notice);
    void OnLocalHotPlugEvent(const InputHotplugEvent &notice);

    void OnRemoteInputDeviceInfo(const std::string &networkId, Parcel &parcel);
    void OnRemoteHotPlugInfo(const std::string &networkId, Parcel &parcel);

    void NotifyInputDeviceToRemote(const std::string &networkId, const std::vector<std::shared_ptr<IDevice>> &keyboards);
    void NotifyHotPlugToRemote(const DSoftbusNotifyDeviceInfo &notice);
    void DumpInputDeviceInfo(const std::string &networkId);
    void RemoveRemoteInputDevice(const std::string &networkId);
    void UpdateRemoteInputDeviceInfo();
    static bool SerializeDeviceInfo(const std::vector<IDevice> &devices, Parcel &parcel);
    static bool DeSerializeDeviceInfo(Parcel &parcel, std::vector<IDevice> &devices);

private:
    IContext *env_ { nullptr };
    bool enable_ { false };
    Channel<CooperateEvent>::Sender sender_;
    std::shared_ptr<DSoftbusObserver> observer_;
    std::map<int32_t, SoftbusParcelHandleType> rawDataHandlers_;
    std::unordered_map<std::string, std::vector<Device>> remoteDeviceInfo_;
};

} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_INPUT_DEVICE_MANAGER_H
