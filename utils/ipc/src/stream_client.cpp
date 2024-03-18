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

#include "stream_client.h"

#include "include/util.h"

#undef LOG_TAG
#define LOG_TAG "StreamClient"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

StreamClient::StreamClient()
{
    CALL_DEBUG_ENTER;
}

StreamClient::~StreamClient()
{
    CALL_DEBUG_ENTER;
}

int32_t StreamClient::StartConnect()
{
    CALL_DEBUG_ENTER;
    if (Socket() < 0) {
        FI_HILOGE("Socket failed");
        return RET_ERR;
    }
    OnConnected();
    return RET_OK;
}

bool StreamClient::SendMsg(const char *buf, size_t size) const
{
    CHKPF(buf);
    if ((size == 0) || (size > MAX_PACKET_BUF_SIZE)) {
        FI_HILOGE("Stream buffer size out of range");
        return false;
    }
    if (fd_ < 0) {
        FI_HILOGE("The fd_ is less than 0");
        return false;
    }

    int32_t retryCount = 0;
    int32_t idx = 0;
    const int32_t bufSize = static_cast<int32_t>(size);
    int32_t remSize = bufSize;
    while (remSize > 0 && retryCount < SEND_RETRY_LIMIT) {
        retryCount += 1;
        ssize_t number = send(fd_, &buf[idx], remSize, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (number < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                FI_HILOGW("Continue for errno EAGAIN|EINTR|EWOULDBLOCK, errno:%{public}d", errno);
                continue;
            }
            FI_HILOGE("Send return failed, error:%{public}d, fd:%{public}d", errno, fd_);
            return false;
        }
        idx += number;
        remSize -= number;
        if (remSize > 0) {
            usleep(SEND_RETRY_SLEEP_TIME);
        }
    }
    if (retryCount >= SEND_RETRY_LIMIT || remSize != 0) {
        FI_HILOGE("Too many times to send :%{public}d/%{public}d, size:%{public}d/%{public}d, fd:%{public}d",
            retryCount, SEND_RETRY_LIMIT, idx, bufSize, fd_);
        return false;
    }
    return true;
}

bool StreamClient::SendMsg(const NetPacket &pkt) const
{
    if (pkt.ChkRWError()) {
        FI_HILOGE("Read and write status is error");
        return false;
    }
    StreamBuffer buf;
    if (!pkt.MakeData(buf)) {
        FI_HILOGE("Failed to buffer packet");
        return false;
    }
    return SendMsg(buf.Data(), buf.Size());
}

bool StreamClient::StartClient(MsgClientFunCallback fun)
{
    CALL_DEBUG_ENTER;
    if (isRunning_ || hasConnected_) {
        FI_HILOGE("Client is connected or started");
        return false;
    }
    hasClient_ = true;
    recvFun_ = fun;
    if (StartConnect() < 0) {
        FI_HILOGW("Client connection failed, Try again later");
    }
    return true;
}

void StreamClient::Stop()
{
    CALL_DEBUG_ENTER;
    hasClient_ = false;
    Close();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS