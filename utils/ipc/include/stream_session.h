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

#ifndef STREAM_SESSION_H
#define STREAM_SESSION_H

#include <list>
#include <memory>
#include <map>

#include <sys/socket.h>
#include <sys/un.h>

#include "nocopyable.h"

#include "net_packet.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class StreamSession;
using SessionPtr = std::shared_ptr<StreamSession>;
class StreamSession : public std::enable_shared_from_this<StreamSession> {
public:
    StreamSession(const std::string &programName, int32_t moduleType, int32_t fd, int32_t uid, int32_t pid);
    DISALLOW_COPY_AND_MOVE(StreamSession);
    virtual ~StreamSession() = default;

    bool SendMsg(const char *buf, size_t size) const;
    bool SendMsg(NetPacket &pkt) const;
    void Close();

    int32_t GetPid() const
    {
        return pid_;
    }

    SessionPtr GetSharedPtr()
    {
        return shared_from_this();
    }

    int32_t GetFd() const
    {
        return fd_;
    }

    void SetTokenType(int32_t type)
    {
        tokenType_ = type;
    }

protected:
    int32_t fd_ { -1 };
    const int32_t pid_ { -1 };
    int32_t tokenType_ { TokenType::TOKEN_INVALID };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // STREAM_SESSION_H