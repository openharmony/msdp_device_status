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

#include "dsoftbus_adapter_impl.h"

#include <netinet/in.h>
#include <netinet/tcp.h>

#include "dfs_session.h"
#include "securec.h"
#include "softbus_bus_center.h"
#include "softbus_error_code.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DSoftbusAdapterImpl" };
constexpr size_t BIND_STRING_LENGTH { 10 };
constexpr size_t INTERCEPT_STRING_LENGTH { 20 };
constexpr size_t DEVICE_NAME_SIZE_MAX { 256 };
constexpr size_t PKG_NAME_SIZE_MAX { 65 };
constexpr int32_t MIN_BW { 80 * 1024 * 1024 };
constexpr int32_t LATENCY { 1600 };
constexpr int32_t SOCKET_SERVER { 0 };
constexpr int32_t SOCKET_CLIENT { 1 };
}

std::mutex DSoftbusAdapterImpl::mutex_;

DSoftbusAdapterImpl& DSoftbusAdapterImpl::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    static DSoftbusAdapterImpl instance;
    return instance;
}

DSoftbusAdapterImpl::~DSoftbusAdapterImpl()
{
    Disable();
}

int32_t DSoftbusAdapterImpl::Enable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    return SetupServer();
}

void DSoftbusAdapterImpl::Disable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    ShutdownServer();
}

void DSoftbusAdapterImpl::AddObserver(std::shared_ptr<IDSoftbusObserver> observer)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    CHKPV(observer);
    observers_.erase(Observer());
    observers_.emplace(observer);
}

void DSoftbusAdapterImpl::RemoveObserver(std::shared_ptr<IDSoftbusObserver> observer)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    if (auto iter = observers_.find(Observer(observer)); iter != observers_.end()) {
        observers_.erase(iter);
    }
    observers_.erase(Observer());
}

int32_t DSoftbusAdapterImpl::OpenSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    return OpenSessionLocked(networkId);
}

void DSoftbusAdapterImpl::CloseSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    if (auto iter = sessions_.find(networkId); iter != sessions_.end()) {
        Shutdown(iter->second.socket_);
        sessions_.erase(iter);
    }
}

int32_t DSoftbusAdapterImpl::FindConnection(const std::string &networkId)
{
    std::lock_guard guard(lock_);
    auto iter = sessions_.find(networkId);
    return (iter != sessions_.end() ? iter->second.socket_ : -1);
}

int32_t DSoftbusAdapterImpl::SendPacket(const std::string &networkId, NetPacket &packet)
{
    int32_t socket = FindConnection(networkId);
    if (socket < 0) {
        FI_HILOGE("Node \'%{public}s\' is not connected", networkId.c_str());
        return RET_ERR;
    }
    StreamBuffer buffer;
    packet.MakeData(buffer);
    if (buffer.Size() > MAX_PACKET_BUF_SIZE) {
        FI_HILOGE("Packet is too large");
        return RET_ERR;
    }
    int32_t ret = ::SendBytes(socket, buffer.Data(), buffer.Size());
    if (ret != SOFTBUS_OK) {
        FI_HILOGE("DSOFTBUS::SendBytes fail (%{public}d)", ret);
        return RET_ERR;
    }
    return RET_OK;
}

static void OnBindLink(int32_t socket, PeerSocketInfo info)
{
    DSoftbusAdapterImpl::GetInstance()->OnBind(socket, info);
}

static void OnShutdownLink(int32_t socket, ShutdownReason reason)
{
    DSoftbusAdapterImpl::GetInstance()->OnShutdown(socket, reason);
}

static void OnBytesAvailable(int32_t socket, const void *data, uint32_t dataLen)
{
    DSoftbusAdapterImpl::GetInstance()->OnBytes(socket, data, dataLen);
}

void DSoftbusAdapterImpl::OnBind(int32_t socket, PeerSocketInfo info)
{
    std::lock_guard guard(lock_);
    std::string networkId = info.networkId;

    if (auto iter = sessions_.find(networkId); iter != sessions_.cend()) {
        return;
    }
    ConfigTcpAlive(socket);
    sessions_.emplace(networkId, Session(socket));

    for (const auto &item : observers_) {
        std::shared_ptr<IDSoftbusObserver> observer = item.Lock();
        if (observer != nullptr) {
            observer->OnBind(networkId);
        }
    }
}

void DSoftbusAdapterImpl::OnShutdown(int32_t socket, ShutdownReason reason)
{
    std::lock_guard guard(lock_);
    auto iter = std::find_if(sessions_.cbegin(), sessions_.cend(),
        [socket](const auto &item) {
            return (item.second.socket_ == socket);
        });
    if (iter == sessions_.cend()) {
        return;
    }
    std::string networkId = iter->first;
    sessions_.erase(iter);

    for (const auto &item : observers_) {
        std::shared_ptr<IDSoftbusObserver> observer = item.Lock();
        if (observer != nullptr) {
            observer->OnShutdown(networkId);
        }
    }
}

void DSoftbusAdapterImpl::OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    std::lock_guard guard(lock_);
    auto iter = std::find_if(sessions_.begin(), sessions_.end(),
        [socket](const auto &item) {
            return (item.second.socket_ == socket);
        });
    if (iter == sessions_.end()) {
        FI_HILOGE("Invalid socket: %{public}d", socket);
        return;
    }
    std::string networkId = iter->first;
    CircleStreamBuffer &circleBuffer = iter->second.buffer_;

    if (!circleBuffer.Write(reinterpret_cast<const char *>(data), dataLen)) {
        FI_HILOGE("Failed to write buffer");
    }
    HandleSessionData(networkId, circleBuffer);
}

int32_t DSoftbusAdapterImpl::InitSocket(SocketInfo info, int32_t socketType, int32_t &socket)
{
    socket = ::Socket(info);
    if (socket < 0) {
        FI_HILOGE("DSOFTBUS::Socket failed");
        return RET_ERR;
    }
    QosTV socketQos[] {
        { .qos = QOS_TYPE_MIN_BW, .value = MIN_BW },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = LATENCY },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = LATENCY },
    };
    ISocketListener listener {
        .OnBind = OnBindLink,
        .OnShutdown = OnShutdownLink,
        .OnBytes = OnBytesAvailable,
    };
    int32_t ret { -1 };

    if (socketType == SOCKET_SERVER) {
        ret = ::Listen(socket, socketQos, sizeof(socketQos) / sizeof(socketQos[0]), &listener);
        if (ret != 0) {
            FI_HILOGE("DSOFTBUS::Listen failed");
        }
    } else if (socketType == SOCKET_CLIENT) {
        ret = ::Bind(socket, socketQos, sizeof(socketQos) / sizeof(socketQos[0]), &listener);
        if (ret != 0) {
            FI_HILOGE("DSOFTBUS::Bind failed");
        }
    }
    if (ret != 0) {
        ::Shutdown(socket);
        socket = -1;
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DSoftbusAdapterImpl::SetupServer()
{
    CALL_DEBUG_ENTER;
    if (socketFd_ > 0) {
        return RET_OK;
    }
    const std::string SESS_NAME { "ohos.msdp.device_status." };
    std::string localNetworkId = GetLocalNetworkId();
    if (localNetworkId.empty()) {
        FI_HILOGE("Local network id is empty");
        return RET_ERR;
    }
    localSessionName_ = SESS_NAME + localNetworkId.substr(0, BIND_STRING_LENGTH);
    std::string sessionName { SESS_NAME + localNetworkId.substr(0, INTERCEPT_STRING_LENGTH) };
    char name[DEVICE_NAME_SIZE_MAX] {};
    if (strcpy_s(name, sizeof(name), sessionName.c_str()) != EOK) {
        FI_HILOGE("Invalid name:%{public}s", sessionName.c_str());
        return RET_ERR;
    }
    char pkgName[PKG_NAME_SIZE_MAX] { FI_PKG_NAME };

    SocketInfo info {
        .name = name,
        .pkgName = pkgName,
        .dataType = DATA_TYPE_BYTES
    };
    int32_t ret = InitSocket(info, SOCKET_SERVER, socketFd_);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to setup server");
        return RET_ERR;
    }
    return RET_OK;
}

void DSoftbusAdapterImpl::ShutdownServer()
{
    CALL_DEBUG_ENTER;
    std::for_each(sessions_.begin(), sessions_.end(), [](const auto &item) {
        Shutdown(item.second.socket_);
    });
    sessions_.clear();
    if (socketFd_ > 0) {
        Shutdown(socketFd_);
        socketFd_ = -1;
    }
}

int32_t DSoftbusAdapterImpl::OpenSessionLocked(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    if (sessions_.find(networkId) != sessions_.end()) {
        FI_HILOGD("InputSoftbus session has already opened");
        return RET_OK;
    }
    char name[DEVICE_NAME_SIZE_MAX] {};
    if (strcpy_s(name, sizeof(name), localSessionName_.c_str()) != EOK) {
        FI_HILOGE("Invalid name:%{public}s", localSessionName_.c_str());
        return RET_ERR;
    }
    const std::string SESSION_NAME { "ohos.msdp.device_status." };
    std::string peerSessionName { SESSION_NAME + networkId.substr(0, INTERCEPT_STRING_LENGTH) };
    char peerName[DEVICE_NAME_SIZE_MAX] {};
    if (strcpy_s(peerName, sizeof(peerName), peerSessionName.c_str()) != EOK) {
        FI_HILOGE("Invalid peerSessionName:%{public}s", peerSessionName.c_str());
        return RET_ERR;
    }
    char peerNetworkId[PKG_NAME_SIZE_MAX] {};
    if (strcpy_s(peerNetworkId, sizeof(peerNetworkId), networkId.c_str()) != EOK) {
        FI_HILOGE("Invalid peerNetworkId:%{public}s", networkId.c_str());
        return RET_ERR;
    }
    char pkgName[PKG_NAME_SIZE_MAX] { FI_PKG_NAME };
    SocketInfo info {
        .name = name,
        .peerName = peerName,
        .peerNetworkId = peerNetworkId,
        .pkgName = pkgName,
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket { -1 };

    int32_t ret = InitSocket(info, SOCKET_CLIENT, socket);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to bind %{public}s", networkId.c_str());
        return ret;
    }
    ConfigTcpAlive(socket);

    sessions_.emplace(networkId, Session(socket));
    return RET_OK;
}

void DSoftbusAdapterImpl::ConfigTcpAlive(int32_t socket)
{
    CALL_DEBUG_ENTER;
    if (socket < 0) {
        FI_HILOGW("Config tcp alive, invalid sessionId");
        return;
    }
    int32_t handle { -1 };
    int32_t result = GetSessionHandle(socket, &handle);
    if (result != RET_OK) {
        FI_HILOGE("Failed to get the session handle, socketId:%{public}d, handle:%{public}d", socket, handle);
        return;
    }
    int32_t keepAliveTimeout { 10 };
    result = setsockopt(handle, IPPROTO_TCP, TCP_KEEPIDLE, &keepAliveTimeout, sizeof(keepAliveTimeout));
    if (result != RET_OK) {
        FI_HILOGE("Config tcp alive, setsockopt set idle falied, result:%{public}d", result);
        return;
    }
    int32_t keepAliveCount { 5 };
    result = setsockopt(handle, IPPROTO_TCP, TCP_KEEPCNT, &keepAliveCount, sizeof(keepAliveCount));
    if (result != RET_OK) {
        FI_HILOGE("Config tcp alive, setsockopt set cnt falied");
        return;
    }
    int32_t interval { 1 };
    result = setsockopt(handle, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
    if (result != RET_OK) {
        FI_HILOGE("Config tcp alive, setsockopt set intvl falied");
        return;
    }
    int32_t enable { 1 };
    result = setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable));
    if (result != RET_OK) {
        FI_HILOGE("Config tcp alive, setsockopt enable alive falied");
        return;
    }
}

void DSoftbusAdapterImpl::HandleSessionData(const std::string &networkId, CircleStreamBuffer &circleBuffer)
{
    while (circleBuffer.ResidualSize() >= static_cast<int32_t>(sizeof(PackHead))) {
        const char *buf = circleBuffer.ReadBuf();
        const PackHead *head = reinterpret_cast<const PackHead *>(buf);

        if ((head->size < 0) || (static_cast<size_t>(head->size) > MAX_PACKET_BUF_SIZE)) {
            FI_HILOGE("Corrupted net packet");
            break;
        }
        if ((head->size + static_cast<int32_t>(sizeof(PackHead))) > circleBuffer.ResidualSize()) {
            break;
        }
        NetPacket packet(head->idMsg);

        if ((head->size > 0) && !packet.Write(&buf[sizeof(PackHead)], head->size)) {
            FI_HILOGE("Failed to fill packet, PacketSize:%{public}d", head->size);
            break;
        }
        circleBuffer.SeekReadPos(packet.GetPacketLength());
        HandlePacket(networkId, packet);
    }
}

void DSoftbusAdapterImpl::HandlePacket(const std::string &networkId, NetPacket &packet)
{
    for (const auto &item : observers_) {
        std::shared_ptr<IDSoftbusObserver> observer = item.Lock();
        if (observer != nullptr) {
            observer->OnPacket(networkId, packet);
            return;
        }
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS