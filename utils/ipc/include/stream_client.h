/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef STREAM_CLIENT_H
#define STREAM_CLIENT_H

#include <functional>
#include <future>
#include <thread>

#include "net_packet.h"
#include "nocopyable.h"
#include "stream_socket.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class StreamClient;
using MsgClientFunCallback = std::function<void(const StreamClient&, NetPacket&)>;
class StreamClient : public StreamSocket {
public:
    StreamClient();
    DISALLOW_COPY_AND_MOVE(StreamClient);
    virtual ~StreamClient();

    virtual int32_t Socket() = 0;
    virtual void Stop();
    int32_t StartConnect();
    bool SendMsg(const char *buf, size_t size) const;
    bool SendMsg(const NetPacket &pkt) const;
    bool GetConnectedStatus() const
    {
        return hasConnected_;
    }

protected:
    virtual void OnConnected() {}
    virtual void OnDisconnected() {}
    bool StartClient(MsgClientFunCallback fun);

protected:
    bool hasClient_ { false };
    bool isRunning_ { false };
    bool hasConnected_ { false };
    MsgClientFunCallback recvFun_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // STREAM_CLIENT_H