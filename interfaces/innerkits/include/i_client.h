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

#ifndef I_CLIENT_H
#define I_CLIENT_H

#include <functional>

#include "event_handler.h"
#include "msg_handler.h"
#include "net_packet.h"
#include "stream_client.h"
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IClient;
using IClientPtr = std::shared_ptr<IClient>;
using EventHandlerPtr = std::shared_ptr<AppExecFwk::EventHandler>;
typedef std::function<void()> ConnectCallback;
typedef std::function<int32_t(const StreamClient&, NetPacket&)> ClientMsgFun;
class IClient : public MsgHandler<MessageId, ClientMsgFun> {
public:
    virtual IClientPtr GetSharedPtr() = 0;
    virtual bool GetCurrentConnectedStatus() const = 0;
    virtual bool Start() = 0;
    virtual bool SendMessage(const NetPacket &pkt) const = 0;
    virtual void RegisterConnectedFunction(ConnectCallback function) = 0;
    virtual void RegisterDisconnectedFunction(ConnectCallback fun) = 0;
    virtual void OnRecvMsg(const char *buf, size_t size) = 0;
    virtual int32_t Reconnect() = 0;
    virtual void OnDisconnect() = 0;
    virtual void SetEventHandler(EventHandlerPtr eventHandler) = 0;
    virtual void MarkIsEventHandlerChanged(EventHandlerPtr eventHandler) = 0;
    virtual bool IsEventHandlerChanged() const = 0;
    virtual bool CheckValidFd() const = 0;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // I_CLIENT_H