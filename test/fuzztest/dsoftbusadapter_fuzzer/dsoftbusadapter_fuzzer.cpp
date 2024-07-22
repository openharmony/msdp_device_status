/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dsoftbusadapter_fuzzer.h"
111
#include "singleton.h"

#include "devicestatus_define.h"
#include "dsoftbus_adapter_impl.h"
#include "socket_session_manager.h"

#include "message_parcel.h"

#undef LOG_TAG
#define LOG_TAG "DsoftbusAdapterFuzzTest"
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
#define SERVER_SESSION_NAME "ohos.msdp.device_status.intention.serversession"
constexpr size_t PKG_NAME_SIZE_MAX { 65 };
constexpr size_t DEVICE_NAME_SIZE_MAX { 256 };

namespace OHOS {

class DSoftbusObserver final : public IDSoftbusObserver {
public:
    DSoftbusObserver() = default;
    ~DSoftbusObserver() = default;

    void OnBind(const std::string &networkId) {}
    void OnShutdown(const std::string &networkId) {}
    void OnConnected(const std::string &networkId) {}
    bool OnPacket(const std::string &networkId, NetPacket &packet)
    {
        return true;
    }
    bool OnRawData(const std::string &networkId, const void *data, uint32_t dataLen)
    {
        return true;
    }
};

bool DsoftbusAdapterFuzzTest(const uint8_t* data, size_t size)
{
    std::shared_ptr<IDSoftbusObserver> observer = std::make_shared<DSoftbusObserver>();
    std::string networkId(reinterpret_cast<const char*>(data), size);
    NetPacket packet(MessageId::DSOFTBUS_START_COOPERATE);
    int32_t socket = *(reinterpret_cast<const int32_t*>(data));
    uint32_t dataLen = *(reinterpret_cast<const uint32_t*>(data));
    int32_t *g_data = new int32_t(socket);
    CircleStreamBuffer circleBuffer;
    Parcel parcel;

    char name[DEVICE_NAME_SIZE_MAX] { SERVER_SESSION_NAME };
    char pkgName[PKG_NAME_SIZE_MAX] { FI_PKG_NAME };
    SocketInfo info {
        .name = name,
        .pkgName = pkgName,
        .dataType = DATA_TYPE_BYTES
    };
    
    DSoftbusAdapterImpl::GetInstance()->Enable();
    DSoftbusAdapterImpl::GetInstance()->AddObserver(observer);
    DSoftbusAdapterImpl::GetInstance()->RemoveObserver(observer);
    DSoftbusAdapterImpl::GetInstance()->CheckDeviceOnline(networkId);
    DSoftbusAdapterImpl::GetInstance()->OpenSession(networkId);
    DSoftbusAdapterImpl::GetInstance()->FindConnection(networkId);
    DSoftbusAdapterImpl::GetInstance()->SendPacket(networkId, packet);
    DSoftbusAdapterImpl::GetInstance()->OnShutdown(socket, SHUTDOWN_REASON_UNKNOWN);
    DSoftbusAdapterImpl::GetInstance()->OnBytes(socket, g_data, dataLen);
    DSoftbusAdapterImpl::GetInstance()->InitSocket(info, socket, socket);
    DSoftbusAdapterImpl::GetInstance()->SetupServer();
    DSoftbusAdapterImpl::GetInstance()->ShutdownServer();
    DSoftbusAdapterImpl::GetInstance()->OpenSessionLocked(networkId);
    DSoftbusAdapterImpl::GetInstance()->OnConnectedLocked(networkId);
    DSoftbusAdapterImpl::GetInstance()->CloseAllSessionsLocked();
    DSoftbusAdapterImpl::GetInstance()->ConfigTcpAlive(socket);
    DSoftbusAdapterImpl::GetInstance()->HandleSessionData(networkId, circleBuffer);
    DSoftbusAdapterImpl::GetInstance()->SendParcel(networkId, parcel);
    DSoftbusAdapterImpl::GetInstance()->BroadcastPacket(packet);
    DSoftbusAdapterImpl::GetInstance()->HandlePacket(networkId, packet);
    DSoftbusAdapterImpl::GetInstance()->HandleRawData(networkId, g_data, dataLen);
    DSoftbusAdapterImpl::GetInstance()->CloseSession(networkId);
    DSoftbusAdapterImpl::GetInstance()->CloseAllSessions();
    DSoftbusAdapterImpl::GetInstance()->Disable();
    return true;
}

} // namespace OHOS
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    OHOS::DsoftbusAdapterFuzzTest(data, size);
    return 0;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS