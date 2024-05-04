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

#include "client.h"

#include <cinttypes>
#include <condition_variable>

#include "xcollie/watchdog.h"

#include "devicestatus_client.h"
#include "fd_listener.h"
#include "fi_log.h"
#include "proto.h"
#include "time_cost_chk.h"
#include "include/util.h"

#undef LOG_TAG
#define LOG_TAG "Client"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace AppExecFwk;
namespace {
const std::string THREAD_NAME { "os_ClientEventHandler" };
const uint64_t WATCHDOG_TIMWVAL { 5000 };
} // namespace

Client::~Client()
{
    CALL_DEBUG_ENTER;
    Stop();
}

void Client::SetEventHandler(EventHandlerPtr eventHandler)
{
    CHKPV(eventHandler);
    eventHandler_ = eventHandler;
}

void Client::MarkIsEventHandlerChanged(EventHandlerPtr eventHandler)
{
    CHKPV(eventHandler);
    CHKPV(eventHandler_);
    auto currentRunner = eventHandler_->GetEventRunner();
    CHKPV(currentRunner);
    auto newEventRunner = eventHandler->GetEventRunner();
    CHKPV(newEventRunner);
    isEventHandlerChanged_ = false;
    if (currentRunner->GetRunnerThreadName() != newEventRunner->GetRunnerThreadName()) {
        isEventHandlerChanged_ = true;
        FI_HILOGD("Event handler changed");
    }
    FI_HILOGD("Current handler name:%{public}s, New handler name:%{public}s",
        currentRunner->GetRunnerThreadName().c_str(), newEventRunner->GetRunnerThreadName().c_str());
}

bool Client::SendMessage(const NetPacket &pkt) const
{
    return SendMsg(pkt);
}

bool Client::GetCurrentConnectedStatus() const
{
    return GetConnectedStatus();
}

IClientPtr Client::GetSharedPtr()
{
    return shared_from_this();
}

bool Client::Start()
{
    CALL_DEBUG_ENTER;
    auto callback = std::bind(&Client::OnMsgHandler, this,
        std::placeholders::_1, std::placeholders::_2);
    if (!StartClient(callback)) {
        FI_HILOGE("Client startup failed");
        Stop();
        return false;
    }
    if (!StartEventRunner()) {
        FI_HILOGE("Start runner failed");
        Stop();
        return false;
    }
    FI_HILOGD("Client started successfully");
    return true;
}

bool Client::StartEventRunner()
{
    CALL_DEBUG_ENTER;
    CHK_PID_AND_TID();
    auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
    CHKPF(runner);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    int ret = HiviewDFX::Watchdog::GetInstance().AddThread("os_ClientEventHandler", eventHandler_, WATCHDOG_TIMWVAL);
    if (ret != 0) {
        FI_HILOGW("add watch dog failed");
    }

    FI_HILOGI("Create event handler, thread name:%{public}s", runner->GetRunnerThreadName().c_str());

    if (hasConnected_ && fd_ >= 0) {
        if (isListening_) {
            FI_HILOGI("File fd is in listening");
            return true;
        }
        if (!AddFdListener(fd_)) {
            FI_HILOGE("Add fd listener failed");
            return false;
        }
    } else {
        if (!eventHandler_->PostTask(std::bind(&Client::OnReconnect, this), CLIENT_RECONNECT_COOLING_TIME)) {
            FI_HILOGE("Send reconnect event failed");
            return false;
        }
    }
    return true;
}

bool Client::AddFdListener(int32_t fd)
{
    CALL_DEBUG_ENTER;
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
        return false;
    }
    CHKPF(eventHandler_);
    auto fdListener = std::make_shared<FdListener>(GetSharedPtr());
    auto errCode = eventHandler_->AddFileDescriptorListener(fd, FILE_DESCRIPTOR_INPUT_EVENT, fdListener,
        "DeviceStatusTask");
    if (errCode != ERR_OK) {
        FI_HILOGE("Add fd listener failed, fd:%{public}d, code:%{public}u, str:%{public}s", fd, errCode,
            GetErrorStr(errCode).c_str());
        return false;
    }
    isRunning_ = true;
    FI_HILOGI("serverFd:%{public}d was listening, mask:%{public}u," PRIu64, fd, FILE_DESCRIPTOR_INPUT_EVENT);
    return true;
}

bool Client::DelFdListener(int32_t fd)
{
    CALL_DEBUG_ENTER;
    CHKPF(eventHandler_);
    if (fd >= 0) {
        eventHandler_->RemoveFileDescriptorListener(fd);
        FI_HILOGI("Remove file descriptor listener success");
    } else {
        FI_HILOGE("Invalid fd:%{public}d", fd);
    }
    auto runner = eventHandler_->GetEventRunner();
    CHKPF(runner);
    if (runner->GetRunnerThreadName() == THREAD_NAME) {
        eventHandler_->RemoveAllEvents();
        FI_HILOGI("Remove all events success");
    }
    isRunning_ = false;
    return true;
}

void Client::OnPacket(NetPacket &pkt)
{
    recvFun_(*this, pkt);
}

void Client::OnRecvMsg(const char *buf, size_t size)
{
    CHKPV(buf);
    if (size == 0 || size > MAX_PACKET_BUF_SIZE) {
        FI_HILOGE("Invalid input param size, size:%{public}zu", size);
        return;
    }
    if (!circBuf_.Write(buf, size)) {
        FI_HILOGW("Write data failed, size:%{public}zu", size);
    }
    OnReadPackets(circBuf_, std::bind(&Client::OnPacket, this, std::placeholders::_1));
}

int32_t Client::Reconnect()
{
    return StartConnect();
}

void Client::OnReconnect()
{
    if (Reconnect() == RET_OK) {
        FI_HILOGI("Reconnect ok");
        return;
    }
    CHKPV(eventHandler_);
    if (!eventHandler_->PostTask(std::bind(&Client::OnReconnect, this), CLIENT_RECONNECT_COOLING_TIME)) {
        FI_HILOGE("Post reconnect event failed");
    }
}

void Client::OnDisconnect()
{
    OnDisconnected();
}

void Client::RegisterConnectedFunction(ConnectCallback function)
{
    funConnected_ = function;
}

void Client::RegisterDisconnectedFunction(ConnectCallback fun)
{
    funDisconnected_ = fun;
}

void Client::OnDisconnected()
{
    CALL_DEBUG_ENTER;
    FI_HILOGI("Disconnected from server, fd:%{public}d", fd_);
    hasConnected_ = false;
    isListening_ = false;
    if (funDisconnected_ != nullptr) {
        FI_HILOGI("Execute funDisconnected");
        funDisconnected_();
    }
    if (!DelFdListener(fd_)) {
        FI_HILOGW("Delete fd listener failed");
    }
    StreamClient::Stop();
    if (hasClient_ && eventHandler_ != nullptr) {
        if (!eventHandler_->PostTask(std::bind(&Client::OnReconnect, this), CLIENT_RECONNECT_COOLING_TIME)) {
            FI_HILOGE("Send reconnect event task failed");
        }
    }
}

void Client::OnConnected()
{
    CALL_DEBUG_ENTER;
    FI_HILOGI("Connection to server succeeded, fd:%{public}d", GetFd());
    hasConnected_ = true;
    if (funConnected_ != nullptr) {
        FI_HILOGI("Execute funConnected");
        funConnected_();
    }
    if (hasClient_ && !isRunning_ && fd_ >= 0 && eventHandler_ != nullptr) {
        if (!AddFdListener(fd_)) {
            FI_HILOGE("Add fd listener failed");
            return;
        }
        isListening_ = true;
    }
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t Client::Socket()
{
    CALL_DEBUG_ENTER;
    return -1;
}
#else
int32_t Client::Socket()
{
    CALL_DEBUG_ENTER;
    int32_t ret = DeviceStatusClient::GetInstance().AllocSocketPair(CONNECT_MODULE_TYPE_FI_CLIENT);
    if (ret != RET_OK) {
        FI_HILOGE("Call AllocSocketPair return %{public}d", ret);
        return RET_ERR;
    }
    fd_ = DeviceStatusClient::GetInstance().GetClientSocketFdOfAllocedSocketPair();
    if (fd_ == -1) {
        FI_HILOGE("Call GetClientSocketFdOfAllocedSocketPair return invalid fd");
    }
    FI_HILOGD("Call GetClientSocketFdOfAllocedSocketPair return fd:%{public}d", fd_);
    return fd_;
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

void Client::Stop()
{
    CALL_DEBUG_ENTER;
    StreamClient::Stop();
    isRunning_ = false;
    if (eventHandler_ != nullptr) {
        auto runner = eventHandler_->GetEventRunner();
        CHKPV(runner);
        if (runner->GetRunnerThreadName() == THREAD_NAME) {
            runner->Stop();
            eventHandler_->RemoveAllEvents();
            eventHandler_->RemoveAllFileDescriptorListeners();
            FI_HILOGI("Remove all file descriptor listeners success");
        }
    }
}

void Client::OnMsgHandler(const StreamClient &client, NetPacket &pkt)
{
    CALL_DEBUG_ENTER;
    auto id = pkt.GetMsgId();
    TimeCostChk chk("Client::OnMsgHandler", "overtime 300(us)", MAX_OVER_TIME, id);
    auto callback = GetMsgCallback(id);
    if (callback == nullptr) {
        FI_HILOGE("Unknown msg id:%{public}d", id);
        return;
    }
    int32_t ret = (*callback)(client, pkt);
    if (ret < 0) {
        FI_HILOGE("Msg handling failed, id:%{public}d, ret:%{public}d", id, ret);
        return;
    }
}

const std::string& Client::GetErrorStr(ErrCode code) const
{
    const static std::string defErrString = "Unknown event handler error!";
    const static std::map<ErrCode, std::string> mapStrings = {
        { ERR_OK, "ERR_OK" },
        { EVENT_HANDLER_ERR_INVALID_PARAM, "Invalid parameters" },
        { EVENT_HANDLER_ERR_NO_EVENT_RUNNER, "Have not set event runner yet" },
        { EVENT_HANDLER_ERR_FD_NOT_SUPPORT, "Not support to listen file descriptors" },
        { EVENT_HANDLER_ERR_FD_ALREADY, "File descriptor is already in listening" },
        { EVENT_HANDLER_ERR_FD_FAILED, "Failed to listen file descriptor" },
        { EVENT_HANDLER_ERR_RUNNER_NO_PERMIT, "No permit to start or stop deposited event runner" },
        { EVENT_HANDLER_ERR_RUNNER_ALREADY, "Event runner is already running" }
    };
    auto it = mapStrings.find(code);
    if (it != mapStrings.end()) {
        return it->second;
    }
    return defErrString;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
