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

#ifndef CLIENT_H
#define CLIENT_H

#include "nocopyable.h"

#include "circle_stream_buffer.h"
#include "i_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class Client final : public StreamClient, public IClient, public std::enable_shared_from_this<IClient> {
public:
    Client() = default;
    DISALLOW_COPY_AND_MOVE(Client);
    ~Client() override;

    int32_t Socket() override;
    void SetEventHandler(EventHandlerPtr eventHandler) override;
    void MarkIsEventHandlerChanged(EventHandlerPtr eventHandler) override;
    bool Start() override;
    void RegisterConnectedFunction(ConnectCallback function) override;
    void RegisterDisconnectedFunction(ConnectCallback fun) override;
    void Stop() override;
    bool GetCurrentConnectedStatus() const override;
    bool SendMessage(const NetPacket& pkt) const override;
    void OnRecvMsg(const char *buf, size_t size) override;
    int32_t Reconnect() override;
    void OnDisconnect() override;
    IClientPtr GetSharedPtr() override;
    inline bool CheckValidFd() const override
    {
        return GetFd() > 0;
    }
    bool IsEventHandlerChanged() const override
    {
        return isEventHandlerChanged_;
    }

private:
    bool StartEventRunner();
    void OnReconnect();
    bool AddFdListener(int32_t fd);
    bool DelFdListener(int32_t fd);
    void OnPacket(NetPacket &pkt);
    const std::string &GetErrorStr(ErrCode code) const;
    void OnConnected() override;
    void OnDisconnected() override;
    void OnMsgHandler(const StreamClient &client, NetPacket &pkt);

private:
    ConnectCallback funConnected_ { nullptr };
    ConnectCallback funDisconnected_ { nullptr };
    CircleStreamBuffer circBuf_;
    std::mutex mtx_;
    EventHandlerPtr eventHandler_ { nullptr };
    bool isEventHandlerChanged_ { false };
    bool isListening_ { false };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // CLIENT_H
