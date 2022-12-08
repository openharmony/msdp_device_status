/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "devicestatus_service.h"

#include <vector>

#include <csignal>

#include <sys/signalfd.h>
#include <unistd.h>

#include <ipc_skeleton.h>

#include "hisysevent.h"
#include "hitrace_meter.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "cooperate_event_manager.h"
#include "input_device_cooperate_sm.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
#include "devicestatus_define.h"
#include "devicestatus_permission.h"
#include "devicestatus_common.h"
#include "devicestatus_dumper.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::HiviewDFX;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusService" };
auto ms = DelayedSpSingleton<DeviceStatusService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(ms.GetRefPtr());
constexpr int32_t DEFAULT_WAIT_TIME_MS { 1000 };
constexpr int32_t WAIT_FOR_ONCE { 1 };
constexpr int32_t MAX_N_RETRIES { 100 };
} // namespace

struct device_status_epoll_event {
    int32_t fd { 0 };
    EpollEventType event_type { EPOLL_EVENT_BEGIN };
};

DeviceStatusService::DeviceStatusService() : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{
    DEV_HILOGD(SERVICE, "Add SystemAbility");
}

DeviceStatusService::~DeviceStatusService() {}

void DeviceStatusService::OnDump()
{
    DEV_HILOGI(SERVICE, "OnDump");
}

void DeviceStatusService::OnStart()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (ready_) {
        DEV_HILOGE(SERVICE, "OnStart is ready, nothing to do");
        return;
    }

    uint64_t tid = GetThisThreadId();
    delegateTasks_.SetWorkerThreadId(tid);

    if (!Init()) {
        DEV_HILOGE(SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<DeviceStatusService>::GetInstance())) {
        DEV_HILOGE(SERVICE, "OnStart register to system ability manager failed");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    ready_ = true;
    t_ = std::thread(std::bind(&DeviceStatusService::OnThread, this));
    t_.join();
}

void DeviceStatusService::OnStop()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (!ready_) {
        return;
    }
    ready_ = false;

    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "devicestatusManager_ is null");
        return;
    }
    devicestatusManager_->UnloadAlgorithm(false);
    DEV_HILOGI(SERVICE, "unload algorithm library exit");
}

IDelegateTasks& DeviceStatusService::GetDelegateTasks()
{
    return delegateTasks_;
}

IDeviceManager& DeviceStatusService::GetDeviceManager()
{
    return devMgr_;
}

ITimerManager& DeviceStatusService::GetTimerManager()
{
    return timerMgr_;
}

int DeviceStatusService::Dump(int fd, const std::vector<std::u16string>& args)
{
    DEV_HILOGI(SERVICE, "dump DeviceStatusServiceInfo");
    if (fd < 0) {
        DEV_HILOGE(SERVICE, "fd is invalid");
        return RET_NG;
    }
    DeviceStatusDumper &deviceStatusDumper = DeviceStatusDumper::GetInstance();
    if (args.empty()) {
        DEV_HILOGE(SERVICE, "param cannot be empty");
        dprintf(fd, "param cannot be empty\n");
        deviceStatusDumper.DumpHelpInfo(fd);
        return RET_NG;
    }
    std::vector<std::string> argList = { "" };
    std::transform(args.begin(), args.end(), std::back_inserter(argList),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    DeviceStatusDataUtils::DeviceStatusType type;
    std::vector<DeviceStatusDataUtils::DeviceStatusData> datas;
    for (type = DeviceStatusDataUtils::TYPE_HIGH_STILL;
        type <= DeviceStatusDataUtils::TYPE_LID_OPEN;
        type = (DeviceStatusDataUtils::DeviceStatusType)(type+1)) {
        DeviceStatusDataUtils::DeviceStatusData data = GetCache(type);
        if (data.value != DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    deviceStatusDumper.ParseCommand(fd, argList, datas);
    return RET_OK;
}

bool DeviceStatusService::Init()
{
    CALL_INFO_TRACE;
    if (!devicestatusManager_) {
        devicestatusManager_ = std::make_shared<DeviceStatusManager>(ms);
    }
    if (!devicestatusManager_->Init()) {
        FI_HILOGE("OnStart init fail");
        return false;
    }
    if (EpollCreate(MAX_EVENT_SIZE) < 0) {
        FI_HILOGE("Create epoll failed");
        return false;
    }
    if (InitDelegateTasks() != RET_OK) {
        FI_HILOGE("Delegate tasks init failed");
        goto INIT_FAIL;
    }
    if (InitTimerMgr() != RET_OK) {
        FI_HILOGE("TimerMgr init failed");
        goto INIT_FAIL;
    }
    if (devMgr_.Init(this) != RET_OK) {
        FI_HILOGE("DevMgr init failed");
        goto INIT_FAIL;
    }

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CooperateEventMgr->SetIContext(this);
    InputDevCooSM->Init();
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return true;

INIT_FAIL:
    EpollClose();
    return false;
}

void DeviceStatusService::Subscribe(const DeviceStatusDataUtils::DeviceStatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "Unsubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DeviceStatusDumper::GetInstance().SaveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceSubscribeStart");
    devicestatusManager_->Subscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, true);
}

void DeviceStatusService::Unsubscribe(const DeviceStatusDataUtils::DeviceStatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "Unsubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DeviceStatusDumper::GetInstance().RemoveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceUnsubscribeStart");
    devicestatusManager_->Unsubscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, false);
}

DeviceStatusDataUtils::DeviceStatusData DeviceStatusService::GetCache(
    const DeviceStatusDataUtils::DeviceStatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DeviceStatusDataUtils::DeviceStatusData data = {type, DeviceStatusDataUtils::DeviceStatusValue::VALUE_EXIT};
        data.value = DeviceStatusDataUtils::DeviceStatusValue::VALUE_INVALID;
        DEV_HILOGI(SERVICE, "GetLatestDeviceStatusData func is nullptr,return default!");
        return data;
    }
    return devicestatusManager_->GetLatestDeviceStatusData(type);
}

void DeviceStatusService::ReportMsdpSysEvent(const DeviceStatusDataUtils::DeviceStatusType& type, bool enable)
{
    auto uid = this->GetCallingUid();
    auto callerToken = this->GetCallingTokenID();
    std::string packageName("");
    devicestatusManager_->GetPackageName(callerToken, packageName);
    if (enable) {
        HiSysEventWrite(HiSysEvent::Domain::MSDP, "SUBSCRIBE", HiSysEvent::EventType::STATISTIC,
            "UID", uid, "PKGNAME", packageName, "TYPE", type);
        return;
    }
    HiSysEventWrite(HiSysEvent::Domain::MSDP, "UNSUBSCRIBE", HiSysEvent::EventType::STATISTIC,
        "UID", uid, "PKGNAME", packageName, "TYPE", type);
}

int32_t DeviceStatusService::AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &toReturnClientFd, int32_t &tokenType)
{
    FI_HILOGD("Enter, programName:%{public}s,moduleType:%{public}d", programName.c_str(), moduleType);

    toReturnClientFd = -1;
    int32_t serverFd = -1;
    int32_t pid = GetCallingPid();
    int32_t uid = GetCallingUid();
    int32_t ret = delegateTasks_.PostSyncTask(std::bind(&UDSServer::AddSocketPairInfo, this,
        programName, moduleType, uid, pid, serverFd, std::ref(toReturnClientFd), tokenType));
    if (ret != RET_OK) {
        FI_HILOGE("Call AddSocketPairInfo failed,return %{public}d", ret);
        return RET_ERR;
    }
    FI_HILOGD("Leave, programName:%{public}s,moduleType:%{public}d,alloc success",
        programName.c_str(), moduleType);
    return RET_OK;
}

void DeviceStatusService::OnConnected(SessionPtr s)
{
    CHKPV(s);
    FI_HILOGI("fd:%{public}d", s->GetFd());
}
void DeviceStatusService::OnDisconnected(SessionPtr s)
{
    CHKPV(s);
    FI_HILOGW("Enter, session desc:%{public}s, fd:%{public}d", s->GetDescript().c_str(), s->GetFd());
}

int32_t DeviceStatusService::AddEpoll(EpollEventType type, int32_t fd)
{
    if (!(type >= EPOLL_EVENT_BEGIN && type < EPOLL_EVENT_END)) {
        FI_HILOGE("Invalid param type");
        return RET_ERR;
    }
    if (fd < 0) {
        FI_HILOGE("Invalid param fd_");
        return RET_ERR;
    }
    auto eventData = static_cast<device_status_epoll_event*>(malloc(sizeof(device_status_epoll_event)));
    if (!eventData) {
        FI_HILOGE("Malloc failed");
        return RET_ERR;
    }
    eventData->fd = fd;
    eventData->event_type = type;
    FI_HILOGD("userdata:[fd:%{public}d,type:%{public}d]", eventData->fd, eventData->event_type);

    struct epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.ptr = eventData;
    auto ret = EpollCtl(fd, EPOLL_CTL_ADD, ev, -1);
    if (ret < 0) {
        free(eventData);
        eventData = nullptr;
        ev.data.ptr = nullptr;
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::DelEpoll(EpollEventType type, int32_t fd)
{
    if (!(type >= EPOLL_EVENT_BEGIN && type < EPOLL_EVENT_END)) {
        FI_HILOGE("Invalid param type");
        return RET_ERR;
    }
    if (fd < 0) {
        FI_HILOGE("Invalid param fd_");
        return RET_ERR;
    }
    struct epoll_event ev = {};
    auto ret = EpollCtl(fd, EPOLL_CTL_DEL, ev, -1);
    if (ret < 0) {
        FI_HILOGE("DelEpoll failed");
        return ret;
    }
    return RET_OK;
}

bool DeviceStatusService::IsRunning() const
{
    return (state_ == ServiceRunningState::STATE_RUNNING);
}

int32_t DeviceStatusService::InitDelegateTasks()
{
    CALL_INFO_TRACE;
    if (!delegateTasks_.Init()) {
        FI_HILOGE("The delegate task init failed");
        return RET_ERR;
    }
    auto ret = AddEpoll(EPOLL_EVENT_ETASK, delegateTasks_.GetReadFd());
    if (ret != RET_OK) {
        FI_HILOGE("AddEpoll error ret:%{public}d", ret);
        return ret;
    }
    FI_HILOGI("AddEpoll, epollfd:%{public}d,fd:%{public}d", epollFd_, delegateTasks_.GetReadFd());
    return RET_OK;
}

int32_t DeviceStatusService::InitTimerMgr()
{
    CALL_INFO_TRACE;
    int32_t ret = timerMgr_.Init(this);
    if (ret != RET_OK) {
        FI_HILOGE("TimerMgr init failed");
        return ret;
    }
    ret = AddEpoll(EPOLL_EVENT_TIMER, timerMgr_.GetTimerFd());
    if (ret != RET_OK) {
        FI_HILOGE("AddEpoll for timer fail");
        return ret;
    }
    return RET_OK;
}

void DeviceStatusService::OnThread()
{
    SetThreadName(std::string("device_status_service"));
    uint64_t tid = GetThisThreadId();
    delegateTasks_.SetWorkerThreadId(tid);
    FI_HILOGD("Main worker thread start. tid:%{public}" PRId64 "", tid);

    FI_HILOGI("Failed to enable device manager, try again after delay");
    EnableDevMgr(MAX_N_RETRIES);
    FI_HILOGI("Enter loop ...");

    while (state_ == ServiceRunningState::STATE_RUNNING) {
        epoll_event ev[MAX_EVENT_SIZE] {};
        int32_t count = EpollWait(ev[0], MAX_EVENT_SIZE, -1);
        for (int32_t i = 0; i < count && state_ == ServiceRunningState::STATE_RUNNING; i++) {
            auto epollEvent = reinterpret_cast<device_status_epoll_event*>(ev[i].data.ptr);
            CHKPC(epollEvent);
            if (epollEvent->event_type == EPOLL_EVENT_SOCKET) {
                OnEpollEvent(ev[i]);
            } else if (epollEvent->event_type == EPOLL_EVENT_SIGNAL) {
                OnSignalEvent(epollEvent->fd);
            } else if (epollEvent->event_type == EPOLL_EVENT_ETASK) {
                OnDelegateTask(ev[i]);
            } else if (epollEvent->event_type == EPOLL_EVENT_TIMER) {
                OnTimeout(ev[i]);
            } else {
                FI_HILOGW("Unknown epoll event type:%{public}d", epollEvent->event_type);
            }
        }
    }
    FI_HILOGD("Main worker thread stop. tid:%{public}" PRId64 "", tid);
}

void DeviceStatusService::OnSignalEvent(int32_t signalFd)
{
    CALL_DEBUG_ENTER;
    signalfd_siginfo sigInfo;
    int32_t size = ::read(signalFd, &sigInfo, sizeof(signalfd_siginfo));
    if (size != static_cast<int32_t>(sizeof(signalfd_siginfo))) {
        FI_HILOGE("Read signal info failed, invalid size:%{public}d,errno:%{public}d", size, errno);
        return;
    }
    int32_t signo = static_cast<int32_t>(sigInfo.ssi_signo);
    FI_HILOGD("Receive signal:%{public}d", signo);
    switch (signo) {
        case SIGINT:
        case SIGQUIT:
        case SIGILL:
        case SIGABRT:
        case SIGBUS:
        case SIGFPE:
        case SIGKILL:
        case SIGSEGV:
        case SIGTERM: {
            state_ = ServiceRunningState::STATE_EXIT;
            break;
        }
        default: {
            break;
        }
    }
}

void DeviceStatusService::OnDelegateTask(epoll_event &ev)
{
    if ((ev.events & EPOLLIN) == 0) {
        FI_HILOGW("Not epollin");
        return;
    }
    DelegateTasks::TaskData data {};
    auto res = read(delegateTasks_.GetReadFd(), &data, sizeof(data));
    if (res == -1) {
        FI_HILOGW("Read failed erron:%{public}d", errno);
    }
    FI_HILOGD("RemoteRequest notify td:%{public}" PRId64 ",std:%{public}" PRId64 ""
        ",taskId:%{public}d", GetThisThreadId(), data.tid, data.taskId);
    delegateTasks_.ProcessTasks();
}

void DeviceStatusService::OnTimeout(epoll_event &ev)
{
    CALL_INFO_TRACE;
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        uint64_t expiration {};
        int ret = read(timerMgr_.GetTimerFd(), &expiration, sizeof(expiration));
        if (ret < 0) {
            FI_HILOGE("Read expiration failed: %{public}s", strerror(errno));
        }
        timerMgr_.ProcessTimers();
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup : %{public}s", strerror(errno));
    }
}

int32_t DeviceStatusService::EnableDevMgr(int32_t nRetries)
{
    CALL_INFO_TRACE;
    static int32_t timerId { -1 };
    int32_t ret = devMgr_.Enable();
    if (ret != RET_OK) {
        FI_HILOGE("Failed to enable device manager");
        if (nRetries > 0) {
            timerId = timerMgr_.AddTimer(DEFAULT_WAIT_TIME_MS, WAIT_FOR_ONCE,
                std::bind(&DeviceStatusService::EnableDevMgr, this, nRetries - 1));
            if (timerId < 0) {
                FI_HILOGE("AddTimer failed, Failed to enable device manager");
            }
        } else {
            FI_HILOGE("Maximum number of retries exceeded, Failed to enable device manager");
        }
    } else if (timerId >= 0) {
        timerMgr_.RemoveTimer(timerId);
    }
    return ret;
}

int32_t DeviceStatusService::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnRegisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("OnRegisterCoordinationListener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnUnregisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("OnUnregisterCoordinationListener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::EnableInputDeviceCoordination(int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnEnableInputDeviceCoordination, this, pid, userData, enabled));
    if (ret != RET_OK) {
        FI_HILOGE("OnEnableInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(enabled);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::StartInputDeviceCoordination(int32_t userData,
    const std::string &sinkDeviceId, int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStartInputDeviceCoordination,
        this, pid, userData, sinkDeviceId, srcInputDeviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnStartInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(sinkDeviceId);
    (void)(srcInputDeviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::StopDeviceCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStopInputDeviceCoordination, this, pid, userData));
    if (ret != RET_OK) {
        FI_HILOGE("OnStopInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnGetInputDeviceCoordinationState, this, pid, userData, deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnGetInputDeviceCoordinationState failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#else
    (void)(userData);
    (void)(deviceId);
    FI_HILOGW("Get input device cooperate state does not support");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION
int32_t DeviceStatusService::OnRegisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_ADD_LISTENER;
    CooperateEventMgr->AddCooperationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnUnregisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::LISTENER;
    event->sess = sess;
    CooperateEventMgr->RemoveCooperationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnEnableInputDeviceCoordination(int32_t pid, int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    InputDevCooSM->EnableInputDeviceCooperate(enabled);
    std::string deviceId =  "";
    CooperationMessage msg =
        enabled ? CooperationMessage::OPEN_SUCCESS : CooperationMessage::CLOSE_SUCCESS;
    NetPacket pkt(MmiMessageId::COOPERATION_MESSAGE);
    pkt << userData << deviceId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnStartInputDeviceCoordination(int32_t pid,
    int32_t userData, const std::string &sinkDeviceId, int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_MESSAGE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    int32_t ret = InputDevCooSM->StartInputDeviceCooperate(sinkDeviceId, srcInputDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("OnStartInputDeviceCoordination failed, ret:%{public}d", ret);
        CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnStopInputDeviceCoordination(int32_t pid, int32_t userData)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_MESSAGE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    int32_t ret = InputDevCooSM->StopInputDeviceCooperate();
    if (ret != RET_OK) {
        FI_HILOGE("OnStopInputDeviceCoordination failed, ret:%{public}d", ret);
        CooperateEventMgr->OnErrorMessage(event->type, CooperationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnGetInputDeviceCoordinationState(
    int32_t pid, int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CooperateEventManager::EventInfo> event = new (std::nothrow) CooperateEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CooperateEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MmiMessageId::COOPERATION_GET_STATE;
    event->userData = userData;
    CooperateEventMgr->AddCooperationEvent(event);
    InputDevCooSM->GetCooperateState(deviceId);
    return RET_OK;
}
#endif // OHOS_BUILD_ENABLE_COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
