/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef CLIENT_MSG_HANDLER_H
#define CLIENT_MSG_HANDLER_H

#include "nocopyable.h"

#include "msg_handler.h"
#include "stream_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
typedef std::function<int32_t(const StreamClient&, NetPacket&)> ClientMsgFun;
class ClientMsgHandler final : public MsgHandler<MessageId, ClientMsgFun> {
public:
    ClientMsgHandler() = default;
    DISALLOW_COPY_AND_MOVE(ClientMsgHandler);
    ~ClientMsgHandler() override = default;

    void Init();
    void OnMsgHandler(const StreamClient& client, NetPacket& pkt);

protected:
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t OnCoordinationListener(const StreamClient& client, NetPacket& pkt);
    int32_t OnCoordinationMessage(const StreamClient& client, NetPacket& pkt);
    int32_t OnCoordinationState(const StreamClient& client, NetPacket& pkt);
#endif // OHOS_BUILD_ENABLE_COORDINATION

private:
    std::function<void(int32_t)> dispatchCallback_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // CLIENT_MSG_HANDLER_H
