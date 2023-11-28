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

#include "coordination_softbus_adapter.h"

#include <chrono>
#include <thread>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "softbus_bus_center.h"
#include "softbus_common.h"

#include "coordination_sm.h"
#include "device_coordination_softbus_define.h"
#include "devicestatus_define.h"
#include "dfs_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "CoordinationSoftbusAdapter" };
std::shared_ptr<CoordinationSoftbusAdapter> g_instance = nullptr;
constexpr uint32_t QOS_LEN = 3;
constexpr int32_t MIN_BW = 160 * 1024 * 1024;
constexpr int32_t LATENCY = 1600;
void ResponseStartRemoteCoordination(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* networkId = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_LOCAL_DEVICE_ID);
    cJSON* buttonIsPressed = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_POINTER_BUTTON_IS_PRESS);
    if (!cJSON_IsString(networkId) || !cJSON_IsBool(buttonIsPressed)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->StartRemoteCoordination(networkId->valuestring, cJSON_IsTrue(buttonIsPressed));
}

void ResponseStartRemoteCoordinationResult(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_RESULT);
    cJSON* dhid = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_START_DHID);
    cJSON* x = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_POINTER_X);
    cJSON* y = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_POINTER_Y);
    if (!cJSON_IsBool(result) || !cJSON_IsString(dhid) || !cJSON_IsNumber(x) || !cJSON_IsNumber(y)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->StartRemoteCoordinationResult(cJSON_IsTrue(result), dhid->valuestring, x->valueint, y->valueint);
}

void ResponseStopRemoteCoordination(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_RESULT);

    if (!cJSON_IsBool(result)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->StopRemoteCoordination(cJSON_IsTrue(result));
}

void ResponseStopRemoteCoordinationResult(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* result = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_RESULT);

    if (!cJSON_IsBool(result)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->StopRemoteCoordinationResult(cJSON_IsTrue(result));
}

void ResponseNotifyUnchainedResult(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* networkId = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_LOCAL_DEVICE_ID);
    cJSON* result = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_RESULT);
    if (!cJSON_IsString(networkId) || !cJSON_IsBool(result)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->NotifyUnchainedResult(networkId->valuestring, cJSON_IsTrue(result));
}

void ResponseStartCoordinationOtherResult(int32_t sessionId, const JsonParser &parser)
{
    CALL_DEBUG_ENTER;
    cJSON* networkId = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_OTHER_DEVICE_ID);

    if (!cJSON_IsString(networkId)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    COOR_SM->StartCoordinationOtherResult(networkId->valuestring);
}
} // namespace

static void BindLink(int32_t socket, PeerSocketInfo info)
{
    COOR_SOFTBUS_ADAPTER->OnBind(socket, info);
}

static void ShutdownLink(int32_t socket, ShutdownReason reason)
{
    COOR_SOFTBUS_ADAPTER->OnShutdown(socket, reason);
}

static void BytesReceived(int32_t socket, const void *data, uint32_t dataLen)
{
    COOR_SOFTBUS_ADAPTER->OnBytes(socket, data, dataLen);
}

int32_t CoordinationSoftbusAdapter::Init()
{
    CALL_INFO_TRACE;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    const std::string SESS_NAME = "ohos.msdp.device_status.";
    std::string localNetworkId = COORDINATION::GetLocalNetworkId();
    if (localNetworkId.empty()) {
        FI_HILOGE("Local network id is empty");
        return RET_ERR;
    }
    std::string sessionName = SESS_NAME + localNetworkId.substr(0, INTERCEPT_STRING_LENGTH);
    if (sessionName == localSessionName_) {
        FI_HILOGI("Softbus session server has already created");
        return RET_OK;
    }
    localSessionName_ = sessionName;
    char name[DEVICE_NAME_SIZE_MAX] = {};
    if (strcpy_s(name, DEVICE_NAME_SIZE_MAX, localSessionName_.c_str()) != EOK) {
        FI_HILOGE("Invalid name:%{public}s", localSessionName_.c_str());
        return RET_ERR;
    }
    char pkgName[PKG_NAME_SIZE_MAX] = FI_PKG_NAME;
    SocketInfo info = {
        .name = name,
        .pkgName = pkgName,
        .dataType = DATA_TYPE_BYTES
    };
    socketFd_ = Socket(info);
    QosTV serverQos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = MIN_BW },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = LATENCY },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = LATENCY },
    };
    ISocketListener listener = {
        .OnBind = BindLink,
        .OnShutdown = ShutdownLink,
        .OnBytes = BytesReceived
    };
    int32_t ret = Listen(socketFd_, serverQos, QOS_LEN, &listener);
    if (ret == RET_OK) {
        FI_HILOGI("server set ok");
    } else {
        FI_HILOGE("server set failed, ret:%{public}d", ret);
    }
    return ret;
}

CoordinationSoftbusAdapter::~CoordinationSoftbusAdapter()
{
    Release();
}

void CoordinationSoftbusAdapter::Release()
{
    CALL_INFO_TRACE;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    std::for_each(sessionDevs_.begin(), sessionDevs_.end(), [](auto item) {
        Shutdown(item.second);
        FI_HILOGD("Session closed successful");
    });
    Shutdown(socketFd_);
    sessionDevs_.clear();
}

bool CoordinationSoftbusAdapter::CheckDeviceSessionState(const std::string &remoteNetworkId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Check session state error");
        return false;
    }
    return true;
}

int32_t CoordinationSoftbusAdapter::OpenInputSoftbus(const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    const std::string SESSION_NAME = "ohos.msdp.device_status.";
    if (CheckDeviceSessionState(remoteNetworkId)) {
        FI_HILOGD("InputSoftbus session has already opened");
        return RET_OK;
    }
    char name[DEVICE_NAME_SIZE_MAX] = {};
    if (strcpy_s(name, DEVICE_NAME_SIZE_MAX, localSessionName_.c_str()) != EOK) {
        FI_HILOGE("Invalid name:%{public}s", localSessionName_.c_str());
        return RET_ERR;
    }
    std::string peerSessionName = SESSION_NAME + remoteNetworkId.substr(0, INTERCEPT_STRING_LENGTH);
    char peerName[DEVICE_NAME_SIZE_MAX] = {};
    if (strcpy_s(peerName, DEVICE_NAME_SIZE_MAX, peerSessionName.c_str()) != EOK) {
        FI_HILOGE("Invalid peerSessionName:%{public}s", peerSessionName.c_str());
        return RET_ERR;
    }
    char peerNetworkId[PKG_NAME_SIZE_MAX] = {};
    if (strcpy_s(peerNetworkId, PKG_NAME_SIZE_MAX, remoteNetworkId.c_str()) != EOK) {
        FI_HILOGE("Invalid peerNetworkId:%{public}s", remoteNetworkId.c_str());
        return RET_ERR;
    }
    char pkgName[PKG_NAME_SIZE_MAX] = FI_PKG_NAME;
    SocketInfo info = {
        .name = name,
        .peerName = peerName,
        .peerNetworkId = peerNetworkId,
        .pkgName = pkgName,
        .dataType = DATA_TYPE_BYTES
    };
    int32_t socket = Socket(info);
    QosTV clientQos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = MIN_BW },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = LATENCY },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = LATENCY },
    };
    ISocketListener listener = {
        .OnBind = BindLink,
        .OnShutdown = ShutdownLink,
        .OnBytes = BytesReceived
    };
    int32_t ret = Bind(socket, clientQos, QOS_LEN, &listener);
    if (ret == RET_OK) {
        FI_HILOGI("bind success");
        sessionDevs_[remoteNetworkId] = socket;
    } else {
        FI_HILOGE("bind failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t CoordinationSoftbusAdapter::WaitSessionOpend(const std::string &remoteNetworkId, int32_t sessionId)
{
    CALL_INFO_TRACE;
    std::unique_lock<std::mutex> waitLock(operationMutex_);
    sessionDevs_[remoteNetworkId] = sessionId;
    auto status = openSessionWaitCond_.wait_for(waitLock, std::chrono::seconds(SESSION_WAIT_TIMEOUT_SECOND),
        [this, remoteNetworkId] () { return false; });
    if (!status) {
        FI_HILOGE("Open session timeout");
        return RET_ERR;
    }
    return RET_OK;
}

void CoordinationSoftbusAdapter::CloseInputSoftbus(const std::string &remoteNetworkId)
{
    CALL_INFO_TRACE;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGI("SessionDevIdMap is not found");
        return;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];

    Shutdown(sessionId);
    sessionDevs_.erase(remoteNetworkId);
}

std::shared_ptr<CoordinationSoftbusAdapter> CoordinationSoftbusAdapter::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        g_instance.reset(new (std::nothrow) CoordinationSoftbusAdapter());
    });
    return g_instance;
}

int32_t CoordinationSoftbusAdapter::StartRemoteCoordination(const std::string &localNetworkId,
    const std::string &remoteNetworkId, bool checkButtonDown)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    auto pointerEvent = COOR_SM->GetLastPointerEvent();
    bool isPointerButtonPressed = false;
    if (checkButtonDown && pointerEvent != nullptr) {
        for (const auto &item : pointerEvent->GetPressedButtons()) {
            if (item == MMI::PointerEvent::MOUSE_BUTTON_LEFT) {
                isPointerButtonPressed = true;
                break;
            }
        }
    }
    FI_HILOGD("isPointerButtonPressed:%{public}d", isPointerButtonPressed);
    cJSON *jsonStr = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(REMOTE_COORDINATION_START));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_LOCAL_DEVICE_ID, cJSON_CreateString(localNetworkId.c_str()));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_POINTER_BUTTON_IS_PRESS, cJSON_CreateBool(isPointerButtonPressed));
    char *sendMsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    int32_t ret = SendMsg(sessionId, sendMsg);
    cJSON_free(sendMsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    if (isPointerButtonPressed) {
        FI_HILOGD("Across with button down, waiting");
        auto status = openSessionWaitCond_.wait_for(sessionLock, std::chrono::seconds(FILTER_WAIT_TIMEOUT_SECOND));
        if (status == std::cv_status::timeout) {
            FI_HILOGE("Add filter timeout");
            return RET_ERR;
        }
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::StartRemoteCoordinationResult(const std::string &remoteNetworkId,
    bool isSuccess, const std::string &startDeviceDhid, int32_t xPercent, int32_t yPercent)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(REMOTE_COORDINATION_START_RES));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_RESULT, cJSON_CreateBool(isSuccess));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_START_DHID, cJSON_CreateString(startDeviceDhid.c_str()));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_POINTER_X, cJSON_CreateNumber(xPercent));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_POINTER_Y, cJSON_CreateNumber(yPercent));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    char *sendMsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    int32_t ret = SendMsg(sessionId, sendMsg);
    cJSON_free(sendMsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::StopRemoteCoordination(const std::string &remoteNetworkId, bool isUnchained)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(REMOTE_COORDINATION_STOP));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_RESULT, cJSON_CreateBool(isUnchained));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    char *sendMsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    int32_t ret = SendMsg(sessionId, sendMsg);
    cJSON_free(sendMsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::StopRemoteCoordinationResult(const std::string &remoteNetworkId,
    bool isSuccess)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(REMOTE_COORDINATION_STOP_RES));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_RESULT, cJSON_CreateBool(isSuccess));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    char *sendMsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    int32_t ret = SendMsg(sessionId, sendMsg);
    cJSON_free(sendMsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::NotifyUnchainedResult(const std::string &localNetworkId,
    const std::string &remoteNetworkId, bool result)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    CHKPR(jsonStr, RET_ERR);
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(NOTIFY_UNCHAINED_RES));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_LOCAL_DEVICE_ID, cJSON_CreateString(localNetworkId.c_str()));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_RESULT, cJSON_CreateBool(result));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    char *sendmsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    CHKPR(sendmsg, RET_ERR);
    int32_t ret = SendMsg(sessionId, sendmsg);
    cJSON_free(sendmsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::NotifyFilterAdded(const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(remoteNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the remote device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[remoteNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    CHKPR(jsonStr, RET_ERR);
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(NOTIFY_FILTER_ADDED));
    char *sendmsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    CHKPR(sendmsg, RET_ERR);
    int32_t ret = SendMsg(sessionId, sendmsg);
    cJSON_free(sendmsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t CoordinationSoftbusAdapter::StartCoordinationOtherResult(const std::string &originNetworkId,
    const std::string &remoteNetworkId)
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(originNetworkId) == sessionDevs_.end()) {
        FI_HILOGE("Failed to discover the origin device");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevs_[originNetworkId];
    cJSON *jsonStr = cJSON_CreateObject();
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_CMD_TYPE, cJSON_CreateNumber(REMOTE_COORDINATION_STOP_OTHER_RES));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_OTHER_DEVICE_ID, cJSON_CreateString(remoteNetworkId.c_str()));
    cJSON_AddItemToObject(jsonStr, FI_SOFTBUS_KEY_SESSION_ID, cJSON_CreateNumber(sessionId));
    char *sendMsg = cJSON_Print(jsonStr);
    cJSON_Delete(jsonStr);
    int32_t ret = SendMsg(sessionId, sendMsg);
    cJSON_free(sendMsg);
    if (ret != RET_OK) {
        FI_HILOGE("Failed to send the sendMsg, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

void CoordinationSoftbusAdapter::HandleSessionData(int32_t socket, const std::string &message)
{
    if (message.empty()) {
        FI_HILOGE("Handle session data, message is empty");
        return;
    }
    JsonParser parser;
    parser.json = cJSON_Parse(message.c_str());
    if (!cJSON_IsObject(parser.json)) {
        FI_HILOGI("Handle session data, parser json is not object");
        if (message.size() < sizeof(DataPacket)) {
            FI_HILOGE("Handle session data, data packet is incomplete");
            return;
        }
        DataPacket* dataPacket = reinterpret_cast<DataPacket *>(const_cast<char*>(message.c_str()));
        if ((message.size() - sizeof(DataPacket)) < dataPacket->dataLen) {
            FI_HILOGE("Handle session data, data is corrupt");
            return;
        }
        if (registerRecvs_.find(dataPacket->messageId) == registerRecvs_.end()) {
            FI_HILOGW("Handle session data, message:%{public}d does not register", dataPacket->messageId);
            return;
        }
        FI_HILOGI("Handle session data, message:%{public}d", dataPacket->messageId);
        if ((dataPacket->messageId == DRAGGING_DATA) ||
            (dataPacket->messageId == STOPDRAG_DATA) ||
            (dataPacket->messageId == IS_PULL_UP) ||
            (dataPacket->messageId == DRAG_CANCEL)) {
            CHKPV(registerRecvs_[dataPacket->messageId]);
            registerRecvs_[dataPacket->messageId](dataPacket->data, dataPacket->dataLen);
        }
        return;
    }
    HandleCoordinationSessionData(socket, parser);
}

void CoordinationSoftbusAdapter::OnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    FI_HILOGD("dataLen:%{public}d", dataLen);
    if ((socket < 0) || (data == nullptr) || (dataLen <= 0)) {
        FI_HILOGE("Param check failed");
        return;
    }
    std::string message = std::string(static_cast<const char *>(data), dataLen);
    HandleSessionData(socket, message);
}

int32_t CoordinationSoftbusAdapter::SendMsg(int32_t socket, const std::string &message)
{
    CALL_DEBUG_ENTER;
    if (message.size() > MSG_MAX_SIZE) {
        FI_HILOGW("Error:the message size:%{public}zu beyond the maximum limit", message.size());
        return RET_ERR;
    }
    return SendBytes(socket, message.c_str(), strlen(message.c_str()));
}

std::string CoordinationSoftbusAdapter::FindDevice(int32_t socket)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    auto find_item = std::find_if(sessionDevs_.begin(), sessionDevs_.end(),
        [socket](const std::map<std::string, int32_t>::value_type item) {
        return item.second == socket;
    });
    if (find_item == sessionDevs_.end()) {
        FI_HILOGE("Find device error");
        return {};
    }
    return find_item->first;
}

int32_t CoordinationSoftbusAdapter::OnBind(int32_t socket, PeerSocketInfo info)
{
    CALL_INFO_TRACE;
    if (socket == -1) {
        std::string networkId = FindDevice(socket);
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        if (sessionDevs_.find(networkId) != sessionDevs_.end()) {
            sessionDevs_.erase(networkId);
        }
        return RET_OK;
    }
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    sessionDevs_[info.deviceId] = socket;
    return RET_OK;
}

void CoordinationSoftbusAdapter::OnShutdown(int32_t socket, ShutdownReason reason)
{
    CALL_DEBUG_ENTER;
    (void)reason;
    std::string networkId = FindDevice(socket);
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(networkId) != sessionDevs_.end()) {
        sessionDevs_.erase(networkId);
    }
    COOR_SM->OnSoftbusSessionClosed(networkId);
    socketFd_ = -1;
}

void CoordinationSoftbusAdapter::RegisterRecvFunc(MessageId messageId, std::function<void(void*, uint32_t)> callback)
{
    CALL_DEBUG_ENTER;
    if (messageId <= MIN_ID || messageId >= MAX_ID) {
        FI_HILOGE("Message id is invalid, messageId:%{public}d", messageId);
        return;
    }
    CHKPV(callback);
    registerRecvs_[messageId] = callback;
}

int32_t CoordinationSoftbusAdapter::SendData(const std::string &networkId, MessageId messageId,
    void* data, uint32_t dataLen)
{
    CALL_DEBUG_ENTER;
    DataPacket* dataPacket = (DataPacket*)malloc(sizeof(DataPacket) + dataLen);
    CHKPR(dataPacket, RET_ERR);
    dataPacket->dataLen = dataLen;
    dataPacket->messageId = messageId;
    errno_t ret = memcpy_s(dataPacket->data, dataPacket->dataLen, data, dataPacket->dataLen);
    if (ret != EOK) {
        FI_HILOGE("Memory copy data packet failed");
        free(dataPacket);
        return RET_ERR;
    }
    int32_t result = SendBytes(sessionDevs_[networkId], dataPacket, sizeof(DataPacket) + dataLen);
    free(dataPacket);
    if (result != RET_OK) {
        FI_HILOGE("Send bytes failed");
        return RET_ERR;
    }
    return RET_OK;
}

void CoordinationSoftbusAdapter::ResponseNotifyFilterAdded()
{
    CALL_DEBUG_ENTER;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    openSessionWaitCond_.notify_all();
}

void CoordinationSoftbusAdapter::HandleCoordinationSessionData(int32_t sessionId, const JsonParser &parser)
{
    cJSON* comType = cJSON_GetObjectItemCaseSensitive(parser.json, FI_SOFTBUS_KEY_CMD_TYPE);
    if (!cJSON_IsNumber(comType)) {
        FI_HILOGE("The data type of CJSON is incorrect");
        return;
    }
    FI_HILOGD("valueint:%{public}d", comType->valueint);
    switch (comType->valueint) {
        case REMOTE_COORDINATION_START: {
            ResponseStartRemoteCoordination(sessionId, parser);
            break;
        }
        case REMOTE_COORDINATION_START_RES: {
            ResponseStartRemoteCoordinationResult(sessionId, parser);
            break;
        }
        case REMOTE_COORDINATION_STOP: {
            ResponseStopRemoteCoordination(sessionId, parser);
            break;
        }
        case REMOTE_COORDINATION_STOP_RES: {
            ResponseStopRemoteCoordinationResult(sessionId, parser);
            break;
        }
        case REMOTE_COORDINATION_STOP_OTHER_RES: {
            ResponseStartCoordinationOtherResult(sessionId, parser);
            break;
        }
        case NOTIFY_UNCHAINED_RES: {
            ResponseNotifyUnchainedResult(sessionId, parser);
            break;
        }
        case NOTIFY_FILTER_ADDED: {
            ResponseNotifyFilterAdded();
            break;
        }
        default: {
            FI_HILOGE("The cmdType is undefined");
            break;
        }
    }
}

void CoordinationSoftbusAdapter::ConfigTcpAlive()
{
    CALL_DEBUG_ENTER;
    if (socketFd_ < 0) {
        FI_HILOGW("Config tcp alive, invalid sessionId");
        return;
    }
    int32_t handle { -1 };
    int32_t result = GetSessionHandle(socketFd_, &handle);
    if (result != RET_OK) {
        FI_HILOGE("Failed to get the session handle, sessionId:%{public}d, handle:%{public}d", socketFd_, handle);
        return;
    }
    int32_t keepAliveTimeout { 10 };
    result = setsockopt(handle, IPPROTO_TCP, TCP_KEEPIDLE, &keepAliveTimeout, sizeof(keepAliveTimeout));
    if (result != RET_OK) {
        FI_HILOGE("Config tcp alive, setsockopt set idle falied");
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
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
