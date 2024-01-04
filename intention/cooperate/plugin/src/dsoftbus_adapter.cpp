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

#include "dsoftbus_adapter.h"

#include <netinet/in.h>
#include <netinet/tcp.h>

#include "dfs_session.h"
#include "securec.h"
#include "softbus_bus_center.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
std::weak_ptr<DSoftbusAdapter> g_dsoftbusAdapter;
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DSoftbusAdapter" };
constexpr size_t BIND_STRING_LENGTH { 10 };
constexpr size_t INTERCEPT_STRING_LENGTH { 20 };
constexpr size_t DEVICE_NAME_SIZE_MAX { 256 };
constexpr size_t PKG_NAME_SIZE_MAX { 65 };
constexpr int32_t MIN_BW { 80 * 1024 * 1024 };
constexpr int32_t LATENCY { 1600 };
constexpr int32_t SOCKET_SERVER { 0 };
constexpr int32_t SOCKET_CLIENT { 1 };
}

DSoftbusAdapter::~DSoftbusAdapter()
{
    Disable();
}

void DSoftbusAdapter::AttachSender(Channel<CooperateEvent>::Sender sender)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    sender_ = sender;
}

int32_t DSoftbusAdapter::Enable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    g_dsoftbusAdapter = shared_from_this();
    return SetupServer();
}

void DSoftbusAdapter::Disable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    ShutdownServer();
    g_dsoftbusAdapter.reset();
}

int32_t DSoftbusAdapter::OpenSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    return OpenSessionLocked(networkId);
}

void DSoftbusAdapter::CloseSession(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    if (auto iter = sessions_.find(networkId); iter != sessions_.end()) {
        Shutdown(iter->second);
        sessions_.erase(iter);
    }
}

int32_t DSoftbusAdapter::SendPacket(const std::string &networkId, const NetPacket &packet)
{
    return RET_ERR;
}

int32_t DSoftbusAdapter::StartCooperate(const std::string &networkId, const DSoftbusStartCooperate &event)
{
    return RET_ERR;
}

int32_t DSoftbusAdapter::StartCooperateResponse(const std::string &networkId,
    const DSoftbusStartCooperateResponse &event)
{
    return RET_ERR;
}

int32_t DSoftbusAdapter::StartCooperateFinish(const std::string &networkId,
    const DSoftbusStartCooperateFinished &event)
{
    return RET_ERR;
}

std::string DSoftbusAdapter::GetLocalNetworkId()
{
    CALL_DEBUG_ENTER;
    auto localNode = std::make_unique<NodeBasicInfo>();
    int32_t ret = GetLocalNodeDeviceInfo(FI_PKG_NAME, localNode.get());
    if (ret != RET_OK) {
        FI_HILOGE("GetLocalNodeDeviceInfo ret:%{public}d", ret);
        return {};
    }
    FI_HILOGD("GetLocalNodeDeviceInfo networkId:%{public}s", localNode->networkId);
    return localNode->networkId;
}

static void OnBindLink(int32_t socket, PeerSocketInfo info)
{
    std::shared_ptr<DSoftbusAdapter> dsoftbus = g_dsoftbusAdapter.lock();
    if (dsoftbus != nullptr) {
        dsoftbus->OnBind(socket, info);
    }
}

static void OnShutdownLink(int32_t socket, ShutdownReason reason)
{
    std::shared_ptr<DSoftbusAdapter> dsoftbus = g_dsoftbusAdapter.lock();
    if (dsoftbus != nullptr) {
        dsoftbus->OnShutdown(socket, reason);
    }
}

static void OnBytesAvailable(int32_t socket, const void *data, uint32_t dataLen)
{
    std::shared_ptr<DSoftbusAdapter> dsoftbus = g_dsoftbusAdapter.lock();
    if (dsoftbus != nullptr) {
        dsoftbus->OnBytes(socket, data, dataLen);
    }
}

void DSoftbusAdapter::OnBind(int32_t socket, PeerSocketInfo info)
{
    std::lock_guard guard(lock_);
}

void DSoftbusAdapter::OnShutdown(int32_t socket, ShutdownReason reason)
{
    std::lock_guard guard(lock_);
}

void DSoftbusAdapter::OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    std::lock_guard guard(lock_);
}

int32_t DSoftbusAdapter::InitSocket(SocketInfo info, int32_t socketType, int32_t &socket)
{
    socket = Socket(info);
    if (socket < 0) {
        FI_HILOGE("Socket failed");
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
        ret = Listen(socket, socketQos, sizeof(socketQos) / sizeof(socketQos[0]), &listener);
        if (ret != 0) {
            FI_HILOGE("Listen failed");
        }
    } else if (socketType == SOCKET_CLIENT) {
        ret = Bind(socket, socketQos, sizeof(socketQos) / sizeof(socketQos[0]), &listener);
        if (ret != 0) {
            FI_HILOGE("Bind failed");
        }
    }
    if (ret != 0) {
        Shutdown(socket);
        socket = -1;
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DSoftbusAdapter::SetupServer()
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

void DSoftbusAdapter::ShutdownServer()
{
    CALL_DEBUG_ENTER;
    std::for_each(sessions_.begin(), sessions_.end(), [](const auto &item) {
        Shutdown(item.second);
    });
    sessions_.clear();
    if (socketFd_ > 0) {
        Shutdown(socketFd_);
        socketFd_ = -1;
    }
}

int32_t DSoftbusAdapter::OpenSessionLocked(const std::string &networkId)
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
    sessions_.emplace(networkId, socket);
    return RET_OK;
}

void DSoftbusAdapter::ConfigTcpAlive(int32_t socket)
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
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
