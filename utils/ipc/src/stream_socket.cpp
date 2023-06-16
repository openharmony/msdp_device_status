/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "stream_socket.h"

#include <cinttypes>

namespace OHOS {
namespace Msdp {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StreamSocket" };
} // namespace

StreamSocket::StreamSocket() {}

StreamSocket::~StreamSocket()
{
    Close();
    EpollClose();
}

int32_t StreamSocket::EpollCreate(int32_t size)
{
    epollFd_ = epoll_create(size);
    if (epollFd_ < 0) {
        FI_HILOGE("epoll_create, return:%{public}d", epollFd_);
    } else {
        FI_HILOGI("epoll_create, epollFd_:%{public}d", epollFd_);
    }
    return epollFd_;
}

int32_t StreamSocket::EpollCtl(int32_t fd, int32_t op, struct epoll_event &event, int32_t epollFd)
{
    if (fd < 0) {
        FI_HILOGE("Invalid fd");
        return RET_ERR;
    }
    if (epollFd < 0) {
        epollFd = epollFd_;
    }
    if (epollFd < 0) {
        FI_HILOGE("Invalid param epollFd");
        return RET_ERR;
    }
    int32_t ret;
    if (op == EPOLL_CTL_DEL) {
        ret = epoll_ctl(epollFd, op, fd, NULL);
    } else {
        ret = epoll_ctl(epollFd, op, fd, &event);
    }
    if (ret < 0) {
        FI_HILOGE("epoll_ctl, return:%{public}d, epollFd_:%{public}d, op:%{public}d, fd:%{public}d, errno:%{public}d",
            ret, epollFd, op, fd, errno);
    }
    return ret;
}

int32_t StreamSocket::EpollWait(int32_t maxevents, int32_t timeout, struct epoll_event &events, int32_t epollFd)
{
    if (epollFd < 0) {
        epollFd = epollFd_;
    }
    if (epollFd < 0) {
        FI_HILOGE("Invalid param epollFd");
        return RET_ERR;
    }
    int32_t ret = epoll_wait(epollFd, &events, maxevents, timeout);
    if (ret < 0) {
        FI_HILOGE("epoll_wait, ret:%{public}d, errno:%{public}d", ret, errno);
    }
    return ret;
}

void StreamSocket::OnReadPackets(CircleStreamBuffer &circBuf, StreamSocket::PacketCallBackFun callbackFun)
{
    constexpr int32_t headSize = static_cast<int32_t>(sizeof(PackHead));
    for (int32_t i = 0; i < ONCE_PROCESS_NETPACKET_LIMIT; i++) {
        const int32_t unreadSize = circBuf.UnreadSize();
        if (unreadSize < headSize) {
            break;
        }
        int32_t dataSize = unreadSize - headSize;
        char *buf = const_cast<char *>(circBuf.ReadBuf());
        CHKPB(buf);
        PackHead *head = reinterpret_cast<PackHead *>(buf);
        CHKPB(head);
        if (static_cast<int32_t>(head->size) < 0 || static_cast<size_t>(head->size) > MAX_PACKET_BUF_SIZE) {
            FI_HILOGE("Packet header parsing error, and this error cannot be recovered, the buffer will be reset, "
                "head->size:%{public}d, unreadSize:%{public}d", head->size, unreadSize);
            circBuf.Reset();
            break;
        }
        if (head->size > dataSize) {
            break;
        }
        NetPacket pkt(head->idMsg);
        if ((head->size > 0) && (!pkt.Write(&buf[headSize], head->size))) {
            FI_HILOGW("Error writing data in the NetPacket, it will be retried next time, messageid:%{public}d, "
                "size:%{public}d", head->idMsg, head->size);
            break;
        }
        if (!circBuf.SeekReadPos(pkt.GetPacketLength())) {
            FI_HILOGW("Set read position error, and this error cannot be recovered, and the buffer will be reset, "
                "packetSize:%{public}d, unreadSize:%{public}d", pkt.GetPacketLength(), unreadSize);
            circBuf.Reset();
            break;
        }
        callbackFun(pkt);
        if (circBuf.IsEmpty()) {
            circBuf.Reset();
            break;
        }
    }
}

void StreamSocket::EpollClose()
{
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
}

void StreamSocket::Close()
{
    if (fd_ >= 0) {
        int32_t rf = close(fd_);
        if (rf > 0) {
            FI_HILOGE("Socket close failed rf:%{public}d", rf);
        }
    }
    fd_ = -1;
}
} // namespace Msdp
} // namespace OHOS