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

#ifndef DSOFTBUS_ADAPTER_H
#define DSOFTBUS_ADAPTER_H

#include <functional>
#include <map>

#include "nocopyable.h"
#include "socket.h"

#include "net_packet.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class DSoftbusAdapter final {
public:
    DSoftbusAdapter() = default;
    ~DSoftbusAdapter() = default;
    DISALLOW_COPY_AND_MOVE(DSoftbusAdapter);

    int32_t Enable();
    void Disable();

    int32_t OpenSession(const std::string &networkId,
        std::function<void(const std::string &, const NetPacket &)> recv);
    void CloseSession(const std::string &networkId);

    int32_t SendPacket(const std::string &networkId, const NetPacket &packet);

    void OnBind(int32_t socket, PeerSocketInfo info);
    void OnShutdown(int32_t socket, ShutdownReason reason);
    void OnBytes(int32_t socket, const void *data, uint32_t dataLen);

    static std::string GetLocalNetworkId();
    static std::shared_ptr<DSoftbusAdapter> GetInstance();
    static void DestroyInstance();

private:
    int32_t InitSocket(SocketInfo info, int32_t socketType, int32_t &socket);
    int32_t SetupServer();
    void ShutdownServer();

    std::mutex lock_;
    int32_t socketFd_ { -1 };
    std::string localSessionName_;
    std::map<std::string, int32_t> sessions_;

    static std::shared_ptr<DSoftbusAdapter> instance_;
    static std::mutex mutex_;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DSOFTBUS_ADAPTER_H
