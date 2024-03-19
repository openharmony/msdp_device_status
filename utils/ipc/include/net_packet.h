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

#ifndef NET_PACKET_H
#define NET_PACKET_H

#include "proto.h"
#include "stream_buffer.h"

#undef LOG_TAG
#define LOG_TAG "NetPacket"

#pragma pack(1)
using PACKHEAD = struct PackHead {
    MessageId idMsg { MessageId::INVALID };
    int32_t size { 0 };
};
#pragma pack()

namespace OHOS {
namespace Msdp {
class NetPacket final : public StreamBuffer {
public:
    explicit NetPacket(MessageId msgId);
    NetPacket(const NetPacket &pkt);
    NetPacket &operator = (const NetPacket &pkt);
    DISALLOW_MOVE(NetPacket);
    ~NetPacket();

    bool MakeData(StreamBuffer &buf) const;
    int32_t GetPacketLength() const
    {
        return (static_cast<int32_t>(sizeof(PackHead)) + wPos_);
    }
    const char *GetData() const
    {
        return Data();
    }
    MessageId GetMsgId() const
    {
        return msgId_;
    }

protected:
    MessageId msgId_ { MessageId::INVALID };
};
} // namespace Msdp
} // namespace OHOS
#endif // NET_PACKET_H