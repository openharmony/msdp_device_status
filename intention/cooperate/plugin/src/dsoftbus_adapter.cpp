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

#include "securec.h"
#include "softbus_bus_center.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DSoftbusAdapter" };
constexpr size_t BIND_STRING_LENGTH { 10 };
constexpr size_t INTERCEPT_STRING_LENGTH { 20 };
constexpr size_t DEVICE_NAME_SIZE_MAX { 256 };
constexpr size_t PKG_NAME_SIZE_MAX { 65 };
constexpr int32_t MIN_BW { 80 * 1024 * 1024 };
constexpr int32_t LATENCY { 1600 };
constexpr int32_t SOCKET_SERVER { 0 };
constexpr int32_t SOCKET_CLIENT { 1 };
}

std::shared_ptr<DSoftbusAdapter> DSoftbusAdapter::instance_ = nullptr;
std::mutex DSoftbusAdapter::mutex_;

std::shared_ptr<DSoftbusAdapter> DSoftbusAdapter::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<DSoftbusAdapter>();
        }
    }
    return instance_;
}

void DSoftbusAdapter::DestroyInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_ != nullptr) {
        instance_.reset();
    }
}

int32_t DSoftbusAdapter::Enable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    return SetupServer();
}

void DSoftbusAdapter::Disable()
{
    CALL_DEBUG_ENTER;
    std::lock_guard guard(lock_);
    ShutdownServer();
}

int32_t DSoftbusAdapter::OpenSession(const std::string &networkId,
    std::function<void(const std::string &, const NetPacket &)> recv)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

void DSoftbusAdapter::CloseSession(const std::string &networkId)
{

}

int32_t DSoftbusAdapter::SendPacket(const std::string &networkId, const NetPacket &packet)
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
    DSoftbusAdapter::GetInstance()->OnBind(socket, info);
}

static void OnShutdownLink(int32_t socket, ShutdownReason reason)
{
    DSoftbusAdapter::GetInstance()->OnShutdown(socket, reason);
}

static void OnBytesAvailable(int32_t socket, const void *data, uint32_t dataLen)
{
    DSoftbusAdapter::GetInstance()->OnBytes(socket, data, dataLen);
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
    int32_t ret = -1;

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
    if (socketFd_ > 0) {
        Shutdown(socketFd_);
        socketFd_ = -1;
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
