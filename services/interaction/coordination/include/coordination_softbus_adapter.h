/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef COORDINATION_SOFTBUS_ADAPTER_H
#define COORDINATION_SOFTBUS_ADAPTER_H

#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "nocopyable.h"
#include "socket.h"

#include "coordination_util.h"
#include "json_parser.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationSoftbusAdapter {
public:
    virtual ~CoordinationSoftbusAdapter();

    enum MessageId {
        MIN_ID = 0,
        DRAGGING_DATA = 1,
        STOPDRAG_DATA = 2,
        IS_PULL_UP = 3,
        DRAG_CANCEL = 4,
        MAX_ID = 50
    };
    struct DataPacket {
        MessageId messageId;
        uint32_t dataLen { 0 };
        uint8_t data[0];
    };

    int32_t StartRemoteCoordination(const std::string &localNetworkId,
        const std::string &remoteNetworkId, bool checkButtonDown);
    int32_t StartRemoteCoordinationResult(const std::string &remoteNetworkId, bool isSuccess,
        const std::string &startDeviceDhid, int32_t xPercent, int32_t yPercent);
    int32_t StopRemoteCoordination(const std::string &remoteNetworkId, bool isUnchained);
    int32_t StopRemoteCoordinationResult(const std::string &remoteNetworkId, bool isSuccess);
    int32_t StartCoordinationOtherResult(const std::string &originNetworkId, const std::string &remoteNetworkId);
    int32_t Init();
    void Release();
    int32_t OpenInputSoftbus(const std::string &remoteNetworkId);
    void CloseInputSoftbus(const std::string &remoteNetworkId);
    int32_t OnBind(int32_t socket, PeerSocketInfo info);
    void OnShutdown(int32_t socket, ShutdownReason reason);
    void OnBytes(int32_t socket, const void* data, uint32_t dataLen);
    void RegisterRecvFunc(MessageId messageId, std::function<void(void*, uint32_t)> callback);
    int32_t SendData(const std::string &networkId, MessageId messageId, void* data, uint32_t dataLen);
    static std::shared_ptr<CoordinationSoftbusAdapter> GetInstance();
    int32_t NotifyUnchainedResult(const std::string &localNetworkId,
        const std::string &remoteNetworkId, bool isSuccess);
    int32_t NotifyFilterAdded(const std::string &remoteNetworkId);
    void ConfigTcpAlive(int32_t socket);

private:
    int32_t ChkAndCpyStr(char* dest, uint32_t destLen, const std::string &src);
    CoordinationSoftbusAdapter() = default;
    DISALLOW_COPY_AND_MOVE(CoordinationSoftbusAdapter);
    std::string FindDevice(int32_t socket);
    int32_t SendMsg(int32_t socket, const std::string &message);
    bool CheckDeviceSessionState(const std::string &remoteNetworkId);
    void HandleSessionData(int32_t socket, const std::string &messageData);
    void HandleCoordinationSessionData(int32_t sessionId, const JsonParser &parser);
    int32_t WaitSessionOpend(const std::string &remoteNetworkId, int32_t sessionId);
    void ResponseNotifyFilterAdded();

    std::map<std::string, int32_t> sessionDevs_;
    std::mutex operationMutex_;
    std::string localSessionName_;
    std::condition_variable openSessionWaitCond_;
    std::map<MessageId, std::function<void(void*, uint32_t)>> registerRecvs_;
    int32_t socketFd_ { -1 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#define COOR_SOFTBUS_ADAPTER CoordinationSoftbusAdapter::GetInstance()
#endif // COORDINATION_SOFTBUS_ADAPTER_H