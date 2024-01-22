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

#ifndef DSOFTBUS_HANDLER_H
#define DSOFTBUS_HANDLER_H

#include "nocopyable.h"

#include "channel.h"
#include "cooperate_events.h"
#include "i_dsoftbus_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class DSoftbusHandler final {
    class DSoftbusObserver final : public IDSoftbusObserver {
    public:
        DSoftbusObserver(DSoftbusHandler &parent) : parent_(parent) {}
        ~DSoftbusObserver() = default;

        void OnBind(const std::string &networkId) override
        {
            parent_.OnBind(networkId);
        }

        void OnShutdown(const std::string &networkId) override
        {
            parent_.OnShutdown(networkId);
        }

        void OnPacket(const std::string &networkId, NetPacket &packet) override
        {
            parent_.OnPacket(networkId, packet);
        }

    private:
        DSoftbusHandler &parent_;
    };

public:
    DSoftbusHandler();
    ~DSoftbusHandler();
    DISALLOW_COPY_AND_MOVE(DSoftbusHandler);

    void AttachSender(Channel<CooperateEvent>::Sender sender);
    int32_t Enable();
    void Disable();

    int32_t OpenSession(const std::string &networkId);
    void CloseSession(const std::string &networkId);

    int32_t StartCooperate(const std::string &networkId, const DSoftbusStartCooperate &event);
    int32_t StartCooperateResponse(const std::string &networkId, const DSoftbusStartCooperateResponse &event);
    int32_t StartCooperateFinish(const std::string &networkId, const DSoftbusStartCooperateFinished &event);

    static std::string GetLocalNetworkId();

private:
    void OnBind(const std::string &networkId);
    void OnShutdown(const std::string &networkId);
    void OnPacket(const std::string &networkId, NetPacket &packet);
    void SendEvent(const CooperateEvent &event);
    void OnStartCooperate(NetPacket &packet);

    std::mutex lock_;
    Channel<CooperateEvent>::Sender sender_;
    std::shared_ptr<DSoftbusObserver> observer_;
    std::unique_ptr<IDSoftbusAdapter> dsoftbus_;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DSOFTBUS_HANDLER_H
