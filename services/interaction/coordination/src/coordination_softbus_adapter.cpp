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
constexpr int32_t DINPUT_LINK_TYPE_MAX { 4 };
const SessionAttribute g_sessionAttr = {
    .dataType = SessionType::TYPE_BYTES,
    .linkTypeNum = DINPUT_LINK_TYPE_MAX,
    .linkType = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_WIFI_WLAN_5G
    }
};

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

static int32_t SessionOpened(int32_t sessionId, int32_t result)
{
    return COOR_SOFTBUS_ADAPTER->OnSessionOpened(sessionId, result);
}

static void SessionClosed(int32_t sessionId)
{
    COOR_SOFTBUS_ADAPTER->OnSessionClosed(sessionId);
}

static void BytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    COOR_SOFTBUS_ADAPTER->OnBytesReceived(sessionId, data, dataLen);
}

static void MessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
}

static void StreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
}

int32_t CoordinationSoftbusAdapter::Init()
{
    CALL_INFO_TRACE;
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    const std::string SESS_NAME = "ohos.msdp.device_status.";
    sessListener_ = {
        .OnSessionOpened = SessionOpened,
        .OnSessionClosed = SessionClosed,
        .OnBytesReceived = BytesReceived,
        .OnMessageReceived = MessageReceived,
        .OnStreamReceived = StreamReceived
    };
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
    int32_t ret = RET_ERR;
    if (!localSessionName_.empty()) {
        FI_HILOGD("Remove last sesison server, sessionName:%{public}s", localSessionName_.c_str());
        ret = RemoveSessionServer(FI_PKG_NAME, localSessionName_.c_str());
        if (ret != RET_OK) {
            FI_HILOGE("Remove softbus session server failed, error code:%{public}d", ret);
        }
    }

    localSessionName_ = sessionName;
    ret = CreateSessionServer(FI_PKG_NAME, localSessionName_.c_str(), &sessListener_);
    if (ret != RET_OK) {
        FI_HILOGE("Create softbus session server failed, error code:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
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
        CloseSession(item.second);
        FI_HILOGD("Session closed successful");
    });
    int32_t ret = RemoveSessionServer(FI_PKG_NAME, localSessionName_.c_str());
    FI_HILOGD("Release removeSessionServer ret:%{public}d", ret);
    sessionDevs_.clear();
    channelStatuss_.clear();
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
    const std::string GROUP_ID = "fi_softbus_group_id";
    const std::string SESSION_NAME = "ohos.msdp.device_status.";
    if (CheckDeviceSessionState(remoteNetworkId)) {
        FI_HILOGD("InputSoftbus session has already opened");
        return RET_OK;
    }

    int32_t ret = Init();
    if (ret != RET_OK) {
        FI_HILOGE("Init failed");
        return RET_ERR;
    }

    std::string peerSessionName = remoteNetworkId.substr(0, INTERCEPT_STRING_LENGTH) + SESSION_NAME;
    FI_HILOGE("PeerSessionName:%{public}s", peerSessionName.c_str());
    int32_t sessionId = OpenSession(localSessionName_.c_str(), peerSessionName.c_str(), remoteNetworkId.c_str(),
        GROUP_ID.c_str(), &g_sessionAttr);
    if (sessionId < 0) {
        FI_HILOGE("Open session failed");
        return RET_ERR;
    }
    return WaitSessionOpend(remoteNetworkId, sessionId);
}

int32_t CoordinationSoftbusAdapter::WaitSessionOpend(const std::string &remoteNetworkId, int32_t sessionId)
{
    CALL_INFO_TRACE;
    sessionDevs_[remoteNetworkId] = sessionId;
    std::unique_lock<std::mutex> waitLock(operationMutex_);
    auto status = openSessionWaitCond_.wait_for(waitLock, std::chrono::seconds(SESSION_WAIT_TIMEOUT_SECOND),
        [this, remoteNetworkId] () { return channelStatuss_[remoteNetworkId]; });
    if (!status) {
        FI_HILOGE("Open session timeout");
        return RET_ERR;
    }
    channelStatuss_[remoteNetworkId] = false;
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

    CloseSession(sessionId);
    sessionDevs_.erase(remoteNetworkId);
    channelStatuss_.erase(remoteNetworkId);
    sessionId_ = -1;
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

void CoordinationSoftbusAdapter::HandleSessionData(int32_t sessionId, const std::string &message)
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
    HandleCoordinationSessionData(sessionId, parser);
}

void CoordinationSoftbusAdapter::OnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    FI_HILOGD("dataLen:%{public}d", dataLen);
    if ((sessionId < 0) || (data == nullptr) || (dataLen <= 0)) {
        FI_HILOGE("Param check failed");
        return;
    }
    std::string message = std::string(static_cast<const char *>(data), dataLen);
    HandleSessionData(sessionId, message);
}

int32_t CoordinationSoftbusAdapter::SendMsg(int32_t sessionId, const std::string &message)
{
    CALL_DEBUG_ENTER;
    if (message.size() > MSG_MAX_SIZE) {
        FI_HILOGW("Error:the message size:%{public}zu beyond the maximum limit", message.size());
        return RET_ERR;
    }
    return SendBytes(sessionId, message.c_str(), strlen(message.c_str()));
}

std::string CoordinationSoftbusAdapter::FindDevice(int32_t sessionId)
{
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    auto find_item = std::find_if(sessionDevs_.begin(), sessionDevs_.end(),
        [sessionId](const std::map<std::string, int32_t>::value_type item) {
        return item.second == sessionId;
    });
    if (find_item == sessionDevs_.end()) {
        FI_HILOGE("Find device error");
        return {};
    }
    return find_item->first;
}

int32_t CoordinationSoftbusAdapter::OnSessionOpened(int32_t sessionId, int32_t result)
{
    CALL_INFO_TRACE;
    char peerDevId[DEVICE_ID_SIZE_MAX] = {};
    sessionId_ = sessionId;
    int32_t getPeerDeviceIdResult = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    FI_HILOGD("Get peer device id ret:%{public}d", getPeerDeviceIdResult);
    if (result != RET_OK) {
        std::string networkId = FindDevice(sessionId);
        FI_HILOGE("Failed to open session, result:%{public}d", result);
        std::unique_lock<std::mutex> sessionLock(operationMutex_);
        if (sessionDevs_.find(networkId) != sessionDevs_.end()) {
            sessionDevs_.erase(networkId);
        }
        if (getPeerDeviceIdResult == RET_OK) {
            channelStatuss_[peerDevId] = true;
        }
        openSessionWaitCond_.notify_all();
        return RET_OK;
    }

    int32_t sessSide = GetSessionSide(sessionId);
    FI_HILOGI("SoftbusSession open succeed, sessionId:%{public}d, sessionSide:%{public}d(1 is client side)",
        sessionId, sessSide);
    std::lock_guard<std::mutex> notifyLock(operationMutex_);
    if (sessSide == SESSION_SIDE_SERVER) {
        if (getPeerDeviceIdResult == RET_OK) {
            sessionDevs_[peerDevId] = sessionId;
        }
    } else {
        if (getPeerDeviceIdResult == RET_OK) {
            channelStatuss_[peerDevId] = true;
        }
        openSessionWaitCond_.notify_all();
    }
    return RET_OK;
}

void CoordinationSoftbusAdapter::OnSessionClosed(int32_t sessionId)
{
    CALL_DEBUG_ENTER;
    std::string networkId = FindDevice(sessionId);
    std::unique_lock<std::mutex> sessionLock(operationMutex_);
    if (sessionDevs_.find(networkId) != sessionDevs_.end()) {
        sessionDevs_.erase(networkId);
    }
    if (GetSessionSide(sessionId) != 0) {
        channelStatuss_.erase(networkId);
    }
    COOR_SM->OnSoftbusSessionClosed(networkId);
    sessionId_ = -1;
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
    if (sessionId_ < 0) {
        FI_HILOGW("Config tcp alive, invalid sessionId");
        return;
    }
    int32_t handle { -1 };
    int32_t result = GetSessionHandle(sessionId_, &handle);
    if (result != RET_OK) {
        FI_HILOGE("Failed to get the session handle, sessionId:%{public}d, handle:%{public}d", sessionId_, handle);
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
