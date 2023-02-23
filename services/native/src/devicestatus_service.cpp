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

#include "devicestatus_service.h"

#include <csignal>
#include <sys/signalfd.h>
#include <unistd.h>
#include <vector>

#include <ipc_skeleton.h>

#include "hitrace_meter.h"
#include "hisysevent.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "bytrace_adapter.h"
#include "devicestatus_common.h"
#include "devicestatus_dumper.h"
#include "devicestatus_hisysevent.h"
#include "devicestatus_permission.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_event_manager.h"
#include "coordination_sm.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::HiviewDFX;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DeviceStatusService" };
constexpr int32_t DEFAULT_WAIT_TIME_MS { 1000 };
constexpr int32_t WAIT_FOR_ONCE { 1 };
constexpr int32_t MAX_N_RETRIES { 100 };

struct device_status_epoll_event {
    int32_t fd { 0 };
    EpollEventType event_type { EPOLL_EVENT_BEGIN };
};

const bool REGISTER_RESULT =
    SystemAbility::MakeAndRegisterAbility(DelayedSpSingleton<DeviceStatusService>::GetInstance().GetRefPtr());
} // namespace
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
    DEV_HILOGD(SERVICE, "Enter");
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
    DEV_HILOGD(SERVICE, "Enter");
    if (!ready_) {
        return;
    }
    ready_ = false;

    if (devicestatusManager_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusManager_ is null");
        return;
    }
    DEV_HILOGD(SERVICE, "unload algorithm library exit");
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

int32_t DeviceStatusService::Dump(int32_t fd, const std::vector<std::u16string>& args)
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

    std::vector<Data> datas;
    for (auto type = TYPE_ABSOLUTE_STILL;type <= TYPE_LID_OPEN;
        type = (Type)(type+1)) {
        Data data = GetCache(type);
        if (data.value != OnChangedValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    deviceStatusDumper.ParseCommand(fd, argList, datas);
    return RET_OK;
}

bool DeviceStatusService::Init()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusManager_ is null");
        auto ms = DelayedSpSingleton<DeviceStatusService>::GetInstance();
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
    InitSessionDeathMonitor();

#ifdef OHOS_BUILD_ENABLE_COORDINATION
    CoordinationEventMgr->SetIContext(this);
    CooSM->Init();
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return true;

INIT_FAIL:
    EpollClose();
    return false;
}
bool DeviceStatusService::IsServiceReady() const
{
    DEV_HILOGD(SERVICE, "Enter");
    return ready_;
}

std::shared_ptr<DeviceStatusManager> DeviceStatusService::GetDeviceStatusManager() const
{
    DEV_HILOGD(SERVICE, "Enter");
    return devicestatusManager_;
}

void DeviceStatusService::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGI(SERVICE, "Enter event:%{public}d,latency:%{public}d", event, latency);
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGE(SERVICE, "devicestatusManager_ is nullptr");
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
    devicestatusManager_->Subscribe(type, event, latency, callback);
    DEV_HILOGD(SERVICE, "Exit");
    FinishTrace(HITRACE_TAG_MSDP);
    ReportSensorSysEvent(type, true);
    WriteSubscribeHiSysEvent(appInfo->uid, appInfo->packageName, type);
}

void DeviceStatusService::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    DEV_HILOGE(SERVICE, "EnterUNevent: %{public}d", event);
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGE(SERVICE, "Unsubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = IPCSkeleton::GetCallingUid();
    appInfo->pid = IPCSkeleton::GetCallingPid();
    appInfo->tokenId = IPCSkeleton::GetCallingTokenID();
    appInfo->packageName = DeviceStatusDumper::GetInstance().GetPackageName(appInfo->tokenId);
    appInfo->type = type;
    appInfo->callback = callback;
    DeviceStatusDumper::GetInstance().RemoveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceUnSubscribeStart");
    devicestatusManager_->Unsubscribe(type, event, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportSensorSysEvent(type, false);
    WriteUnSubscribeHiSysEvent(appInfo->uid, appInfo->packageName, type);
}

Data DeviceStatusService::GetCache(const Type& type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        Data data = {type, OnChangedValue::VALUE_EXIT};
        data.value = OnChangedValue::VALUE_INVALID;
        DEV_HILOGI(SERVICE, "GetLatestDeviceStatusData func is nullptr,return default!");
        return data;
    }
    return devicestatusManager_->GetLatestDeviceStatusData(type);
}

void DeviceStatusService::ReportSensorSysEvent(int32_t type, bool enable)
{
    auto callerToken = GetCallingTokenID();
    std::string packageName;
    devicestatusManager_->GetPackageName(callerToken, packageName);
    auto uid = GetCallingUid();
    std::string str = enable ? "Subscribe" : "Unsubscribe";
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MSDP,
        str,
        OHOS::HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "UID", uid,
        "PKGNAME", packageName,
        "TYPE", type);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "HiviewDFX write failed, ret:%{public}d", ret);
    }
}

int32_t DeviceStatusService::AllocSocketFd(const std::string &programName, const int32_t moduleType,
        int32_t &toReturnClientFd, int32_t &tokenType)
{
    FI_HILOGD("Enter, programName:%{public}s,moduleType:%{public}d", programName.c_str(), moduleType);

    toReturnClientFd = -1;
    int32_t serverFd = -1;
    int32_t pid = GetCallingPid();
    int32_t uid = GetCallingUid();
    int32_t ret = delegateTasks_.PostSyncTask(std::bind(&StreamServer::AddSocketPairInfo, this,
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

void DeviceStatusService::InitSessionDeathMonitor()
{
    CALL_INFO_TRACE;
    std::vector<std::function<void(SessionPtr)>> sessionLostList = {
        std::bind(&DragManager::OnSessionLost, &dragMgr_, std::placeholders::_1),
#ifdef OHOS_BUILD_ENABLE_COORDINATION
        std::bind(&CoordinationSM::OnSessionLost, CooSM, std::placeholders::_1)
#endif
    };
    for (const auto &it : sessionLostList) {
        AddSessionDeletedCallback(it);
    }
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
    EnableDevMgr(MAX_N_RETRIES);

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
            } else if (epollEvent->event_type == EPOLL_EVENT_DEVICE_MGR) {
                OnDeviceMgr(ev[i]);
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

void DeviceStatusService::OnDelegateTask(const epoll_event &ev)
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

void DeviceStatusService::OnTimeout(const epoll_event &ev)
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
        FI_HILOGE("Epoll hangup: %{public}s", strerror(errno));
    }
}

void DeviceStatusService::OnDeviceMgr(const epoll_event &ev)
{
    CALL_INFO_TRACE;
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        devMgr_.Dispatch(ev);
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup: %{public}s", strerror(errno));
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
    } else {
        AddEpoll(EPOLL_EVENT_DEVICE_MGR, devMgr_.GetFd());
        if (timerId >= 0) {
            timerMgr_.RemoveTimer(timerId);
            timerId = -1;
        }
    }
    return ret;
}

void DeviceStatusService::DisableDevMgr()
{
    DelEpoll(EPOLL_EVENT_DEVICE_MGR, devMgr_.GetFd());
    devMgr_.Disable();
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

int32_t DeviceStatusService::EnableCoordination(int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnEnableCoordination, this, pid, userData, enabled));
    if (ret != RET_OK) {
        FI_HILOGE("OnEnableCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(enabled);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::StartCoordination(int32_t userData,
    const std::string &sinkDeviceId, int32_t srcDeviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStartCoordination,
        this, pid, userData, sinkDeviceId, srcDeviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnStartCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(sinkDeviceId);
    (void)(srcDeviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::StopCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStopCoordination, this, pid, userData));
    if (ret != RET_OK) {
        FI_HILOGE("OnStopCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::GetCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnGetCoordinationState, this, pid, userData, deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnGetCoordinationState failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#else
    (void)(userData);
    (void)(deviceId);
    FI_HILOGW("Get coordination state does not support");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::AddDraglistener()
{
    CALL_DEBUG_ENTER;
    int32_t pid = GetCallingPid();
    auto session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::AddListener, &dragMgr_, session));
    if (ret != RET_OK) {
        FI_HILOGE("AddListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::RemoveDraglistener()
{
    CALL_DEBUG_ENTER;
    int32_t pid = GetCallingPid();
    auto session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::RemoveListener, &dragMgr_, session));
    if (ret != RET_OK) {
        FI_HILOGE("RemoveListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStartDrag, this, std::cref(dragData), pid));
    if (ret != RET_OK) {
        FI_HILOGE("OnStartDrag failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::StopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnStopDrag, this, result));
    if (ret != RET_OK) {
        FI_HILOGE("OnStopDrag failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DeviceStatusService::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DeviceStatusService::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    return RET_OK;
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION
int32_t DeviceStatusService::OnRegisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    CoordinationEventMgr->AddCoordinationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnUnregisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    CoordinationEventMgr->RemoveCoordinationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnEnableCoordination(int32_t pid, int32_t userData, bool enabled)
{
    CALL_DEBUG_ENTER;
    CooSM->EnableCoordination(enabled);
    std::string deviceId =  "";
    CoordinationMessage msg =
        enabled ? CoordinationMessage::OPEN_SUCCESS : CoordinationMessage::CLOSE_SUCCESS;
    NetPacket pkt(MessageId::COORDINATION_MESSAGE);
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

int32_t DeviceStatusService::OnStartCoordination(int32_t pid,
    int32_t userData, const std::string &sinkDeviceId, int32_t srcDeviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    if (CooSM->GetCurrentCoordinationState() == CoordinationState::STATE_OUT) {
        FI_HILOGW("It is currently worn out");
        NetPacket pkt(event->msgId);
        pkt << userData << "" << static_cast<int32_t>(CoordinationMessage::INFO_SUCCESS);
        if (pkt.ChkRWError()) {
            FI_HILOGE("Packet write data failed");
            return RET_ERR;
        }
        if (!sess->SendMsg(pkt)) {
            FI_HILOGE("Sending failed");
            return RET_ERR;
        }
        return RET_OK;
    }
    CoordinationEventMgr->AddCoordinationEvent(event);
    int32_t ret = CooSM->StartCoordination(sinkDeviceId, srcDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("OnStartCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnStopCoordination(int32_t pid, int32_t userData)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    int32_t ret = CooSM->StopCoordination();
    if (ret != RET_OK) {
        FI_HILOGE("OnStopCoordination failed, ret:%{public}d", ret);
        CoordinationEventMgr->OnErrorMessage(event->type, CoordinationMessage(ret));
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnGetCoordinationState(
    int32_t pid, int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    CoordinationEventMgr->AddCoordinationEvent(event);
    CooSM->GetCoordinationState(deviceId);
    return RET_OK;
}
#endif // OHOS_BUILD_ENABLE_COORDINATION

int32_t DeviceStatusService::OnStartDrag(const DragData &dragData, int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    int32_t ret = dragMgr_.StartDrag(dragData, sess);
    if (ret != RET_OK) {
        FI_HILOGE("StartDrag failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnStopDrag(int32_t result)
{
    CALL_DEBUG_ENTER;
    int32_t ret = dragMgr_.StopDrag(result);
    if (ret != RET_OK) {
        FI_HILOGE("StopDrag failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
