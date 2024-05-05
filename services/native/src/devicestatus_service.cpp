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

#include <unistd.h>
#include <vector>

#include <ipc_skeleton.h>

#include "hitrace_meter.h"
#include "hisysevent.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#include "coordination_device_manager.h"
#include "coordination_event_manager.h"
#include "coordination_hotarea.h"
#include "coordination_sm.h"
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION

#include "devicestatus_common.h"
#include "devicestatus_hisysevent.h"
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#include "ddp_adapter.h"
#include "dsoftbus_adapter.h"
#include "input_adapter.h"
#include "plugin_manager.h"
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
#include "motion_drag.h"
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG

#undef LOG_TAG
#define LOG_TAG "DeviceStatusService"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
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

DeviceStatusService::DeviceStatusService()
    : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    input_ = std::make_unique<InputAdapter>();
    pluginMgr_ = std::make_unique<PluginManager>(this);
    dsoftbus_ = std::make_unique<DSoftbusAdapter>();
    ddp_ = std::make_unique<DDPAdapter>();
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

DeviceStatusService::~DeviceStatusService()
{}

void DeviceStatusService::OnDump()
{}

void DeviceStatusService::OnStart()
{
    CALL_INFO_TRACE;
    if (ready_) {
        FI_HILOGE("On start is ready, nothing to do");
        return;
    }

    uint64_t tid = GetThisThreadId();
    delegateTasks_.SetWorkerThreadId(tid);

    if (!Init()) {
        FI_HILOGE("On start call init failed");
        return;
    }
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    EnableDSoftbus();
    intention_ = sptr<IntentionService>::MakeSptr(this);
    if (!Publish(intention_)) {
#else
    if (!Publish(this)) {
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
        FI_HILOGE("On start register to system ability manager failed");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    ready_ = true;
    worker_ = std::thread(std::bind(&DeviceStatusService::OnThread, this));
}

void DeviceStatusService::OnStop()
{
    CALL_INFO_TRACE;
    if (!ready_) {
        return;
    }
    ready_ = false;
    state_ = ServiceRunningState::STATE_EXIT;

    delegateTasks_.PostAsyncTask([]() -> int32_t {
        FI_HILOGD("No op");
        return RET_OK;
    });
    if (worker_.joinable()) {
        worker_.join();
    }
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

IDragManager& DeviceStatusService::GetDragManager()
{
    return dragMgr_;
}

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
ISocketSessionManager& DeviceStatusService::GetSocketSessionManager()
{
    return socketSessionMgr_;
}

IPluginManager& DeviceStatusService::GetPluginManager()
{
    return *pluginMgr_;
}

IInputAdapter& DeviceStatusService::GetInput()
{
    return *input_;
}

IDSoftbusAdapter& DeviceStatusService::GetDSoftbus()
{
    return *dsoftbus_;
}

IDDPAdapter& DeviceStatusService::GetDP()
{
    return *ddp_;
}

void DeviceStatusService::EnableDSoftbus()
{
    CALL_INFO_TRACE;
    int32_t ret = dsoftbus_->Enable();
    if (ret != RET_OK) {
        FI_HILOGE("Failed to enable dsoftbus, try again later");
        int32_t timerId = timerMgr_.AddTimer(DEFAULT_WAIT_TIME_MS, WAIT_FOR_ONCE,
            std::bind(&DeviceStatusService::EnableDSoftbus, this));
        if (timerId < 0) {
            FI_HILOGE("AddTimer failed, Failed to enable dsoftbus");
        }
    } else {
        FI_HILOGI("Enable dsoftbus successfully");
    }
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

int32_t DeviceStatusService::Dump(int32_t fd, const std::vector<std::u16string> &args)
{
    CALL_DEBUG_ENTER;
    if (fd < 0) {
        FI_HILOGE("fd is invalid");
        return RET_NG;
    }
    if (args.empty()) {
        FI_HILOGE("Param cannot be empty");
        dprintf(fd, "param cannot be empty\n");
        DS_DUMPER->DumpHelpInfo(fd);
        return RET_NG;
    }
    std::vector<std::string> argList;
    std::transform(args.begin(), args.end(), std::back_inserter(argList),
        [](const std::u16string &arg) {
        return Str16ToStr8(arg);
    });

    std::vector<Data> datas;
    for (auto type = TYPE_ABSOLUTE_STILL; type <= TYPE_LID_OPEN; type = static_cast<Type>(type + 1)) {
        Data data = GetCache(type);
        if (data.value != OnChangedValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    DS_DUMPER->ParseCommand(fd, argList, datas);
    return RET_OK;
}

bool DeviceStatusService::Init()
{
    CALL_DEBUG_ENTER;
    if (devicestatusManager_ == nullptr) {
        FI_HILOGW("devicestatusManager_ is nullptr");
        devicestatusManager_ = std::make_shared<DeviceStatusManager>();
    }
    if (!devicestatusManager_->Init()) {
        FI_HILOGE("OnStart init failed");
        return false;
    }
    if (EpollCreate() != RET_OK) {
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
    if (dragMgr_.Init(this) != RET_OK) {
        FI_HILOGE("Drag manager init failed");
        goto INIT_FAIL;
    }
    if (InitMotionDrag() != RET_OK) {
        FI_HILOGE("Drag adapter init failed");
        goto INIT_FAIL;
    }
    if (DS_DUMPER->Init(this) != RET_OK) {
        FI_HILOGE("Dump init failed");
        goto INIT_FAIL;
    }
#if defined(OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK)
    if (socketSessionMgr_.Init() != RET_OK) {
        FI_HILOGE("Failed to initialize socket session manager");
        goto INIT_FAIL;
    }
#elif defined(OHOS_BUILD_ENABLE_COORDINATION)
    COOR_EVENT_MGR->SetIContext(this);
    COOR_SM->Init();
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return true;

INIT_FAIL:
    EpollClose();
    return false;
}

bool DeviceStatusService::IsServiceReady() const
{
    return ready_;
}

void DeviceStatusService::Subscribe(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
    FI_HILOGI("Enter event:%{public}d, latency:%{public}d", event, latency);
    CHKPV(devicestatusManager_);
    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DS_DUMPER->SaveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceSubscribeStart");
    devicestatusManager_->Subscribe(type, event, latency, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportSensorSysEvent(type, true);
    WriteSubscribeHiSysEvent(appInfo->uid, appInfo->packageName, type);
}

void DeviceStatusService::Unsubscribe(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
    FI_HILOGI("Enter event:%{public}d", event);
    CHKPV(devicestatusManager_);
    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = IPCSkeleton::GetCallingUid();
    appInfo->pid = IPCSkeleton::GetCallingPid();
    appInfo->tokenId = IPCSkeleton::GetCallingTokenID();
    appInfo->packageName = DS_DUMPER->GetPackageName(appInfo->tokenId);
    appInfo->type = type;
    appInfo->callback = callback;
    DS_DUMPER->RemoveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceUnSubscribeStart");
    devicestatusManager_->Unsubscribe(type, event, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportSensorSysEvent(type, false);
    WriteUnSubscribeHiSysEvent(appInfo->uid, appInfo->packageName, type);
}

Data DeviceStatusService::GetCache(const Type &type)
{
    CALL_DEBUG_ENTER;
    if (devicestatusManager_ == nullptr) {
        Data data = {type, OnChangedValue::VALUE_EXIT};
        data.value = OnChangedValue::VALUE_INVALID;
        FI_HILOGI("Get latest device status data func is nullptr, return default!");
        return data;
    }
    return devicestatusManager_->GetLatestDeviceStatusData(type);
}

void DeviceStatusService::ReportSensorSysEvent(int32_t type, bool enable)
{
    auto callerToken = GetCallingTokenID();
    std::string packageName;
    CHKPV(devicestatusManager_);
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
        FI_HILOGE("HiviewDFX write failed, ret:%{public}d", ret);
    }
}

int32_t DeviceStatusService::AllocSocketFd(const std::string &programName, int32_t moduleType,
    int32_t &toReturnClientFd, int32_t &tokenType)
{
    FI_HILOGD("Enter, programName:%{public}s, moduleType:%{public}d", programName.c_str(), moduleType);

    toReturnClientFd = -1;
    int32_t serverFd = -1;
    int32_t pid = GetCallingPid();
    int32_t uid = GetCallingUid();
    int32_t ret = delegateTasks_.PostSyncTask(std::bind(&StreamServer::AddSocketPairInfo, this,
        programName, moduleType, uid, pid, serverFd, std::ref(toReturnClientFd), tokenType));
    if (ret != RET_OK) {
        FI_HILOGE("Call Add socket pair info failed, return:%{public}d", ret);
        return RET_ERR;
    }
    FI_HILOGD("Leave, programName:%{public}s, moduleType:%{public}d, alloc success",
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
    FI_HILOGW("Enter, session, fd:%{public}d", s->GetFd());
}

int32_t DeviceStatusService::AddEpoll(EpollEventType type, int32_t fd)
{
    if (!(type >= EPOLL_EVENT_BEGIN && type < EPOLL_EVENT_END)) {
        FI_HILOGE("Invalid type:%{public}d", type);
        return RET_ERR;
    }
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
        return RET_ERR;
    }
    auto eventData = static_cast<device_status_epoll_event*>(malloc(sizeof(device_status_epoll_event)));
    if (!eventData) {
        FI_HILOGE("Malloc failed");
        return RET_ERR;
    }
    eventData->fd = fd;
    eventData->event_type = type;
    FI_HILOGD("EventData:[fd:%{public}d, type:%{public}d]", eventData->fd, eventData->event_type);

    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = eventData;
    if (EpollCtl(fd, EPOLL_CTL_ADD, ev) != RET_OK) {
        free(eventData);
        eventData = nullptr;
        ev.data.ptr = nullptr;
        FI_HILOGE("EpollCtl failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceStatusService::DelEpoll(EpollEventType type, int32_t fd)
{
    if (!(type >= EPOLL_EVENT_BEGIN && type < EPOLL_EVENT_END)) {
        FI_HILOGE("Invalid type:%{public}d", type);
        return RET_ERR;
    }
    if (fd < 0) {
        FI_HILOGE("Invalid fd:%{public}d", fd);
        return RET_ERR;
    }
    struct epoll_event ev {};
    if (EpollCtl(fd, EPOLL_CTL_DEL, ev) != RET_OK) {
        FI_HILOGE("DelEpoll failed");
        return RET_ERR;
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
    int32_t ret = AddEpoll(EPOLL_EVENT_ETASK, delegateTasks_.GetReadFd());
    if (ret != RET_OK) {
        FI_HILOGE("AddEpoll error ret:%{public}d", ret);
    }
    FI_HILOGI("AddEpoll, epollfd:%{public}d, fd:%{public}d", epollFd_, delegateTasks_.GetReadFd());
    return ret;
}

int32_t DeviceStatusService::InitMotionDrag()
{
    CALL_INFO_TRACE;
#ifdef OHOS_BUILD_ENABLE_MOTION_DRAG
    if (motionDrag_ == nullptr) {
        motionDrag_ = std::make_unique<MotionDrag>();
    }
    if (motionDrag_->Init(this) != RET_OK) {
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_MOTION_DRAG
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
        FI_HILOGE("AddEpoll for timer failed");
    }
    return ret;
}

void DeviceStatusService::OnThread()
{
    SetThreadName(std::string("os_ds_service"));
    uint64_t tid = GetThisThreadId();
    delegateTasks_.SetWorkerThreadId(tid);
    FI_HILOGD("Main worker thread start, tid:%{public}" PRId64 "", tid);
    EnableDevMgr(MAX_N_RETRIES);

    while (state_ == ServiceRunningState::STATE_RUNNING) {
        struct epoll_event ev[MAX_EVENT_SIZE] {};
        int32_t count = EpollWait(MAX_EVENT_SIZE, -1, ev[0]);
        for (int32_t i = 0; i < count && state_ == ServiceRunningState::STATE_RUNNING; i++) {
            auto epollEvent = reinterpret_cast<device_status_epoll_event*>(ev[i].data.ptr);
            CHKPC(epollEvent);
            if (epollEvent->event_type == EPOLL_EVENT_SOCKET) {
                OnEpollEvent(ev[i]);
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
    FI_HILOGD("Main worker thread stop, tid:%{public}" PRId64 "", tid);
}

void DeviceStatusService::OnDelegateTask(const struct epoll_event &ev)
{
    if ((ev.events & EPOLLIN) == 0) {
        FI_HILOGW("Not epollin");
        return;
    }
    DelegateTasks::TaskData data {};
    ssize_t res = read(delegateTasks_.GetReadFd(), &data, sizeof(data));
    if (res == -1) {
        FI_HILOGW("Read failed erron:%{public}d", errno);
    }
    FI_HILOGD("RemoteRequest notify td:%{public}" PRId64 ", std:%{public}" PRId64 ""
        ", taskId:%{public}d", GetThisThreadId(), data.tid, data.taskId);
    delegateTasks_.ProcessTasks();
}

void DeviceStatusService::OnTimeout(const struct epoll_event &ev)
{
    CALL_DEBUG_ENTER;
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        uint64_t expiration {};
        ssize_t ret = read(timerMgr_.GetTimerFd(), &expiration, sizeof(expiration));
        if (ret < 0) {
            FI_HILOGE("Read expiration failed:%{public}s", strerror(errno));
        }
        timerMgr_.ProcessTimers();
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup:%{public}s", strerror(errno));
    }
}

void DeviceStatusService::OnDeviceMgr(const struct epoll_event &ev)
{
    CALL_DEBUG_ENTER;
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        devMgr_.Dispatch(ev);
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup:%{public}s", strerror(errno));
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
        return ret;
    }
    AddEpoll(EPOLL_EVENT_DEVICE_MGR, devMgr_.GetFd());
    if (timerId >= 0) {
        timerMgr_.RemoveTimer(timerId);
        timerId = -1;
    }
    return RET_OK;
}

void DeviceStatusService::DisableDevMgr()
{
    DelEpoll(EPOLL_EVENT_DEVICE_MGR, devMgr_.GetFd());
    devMgr_.Disable();
}

int32_t DeviceStatusService::RegisterCoordinationListener(bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnRegisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("On register coordination listener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::UnregisterCoordinationListener(bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnUnregisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("On unregister coordination listener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::PrepareCoordination(int32_t userData, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    AddSessionDeletedCallback(pid, std::bind(&CoordinationSM::OnSessionLost, COOR_SM, std::placeholders::_1));
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnPrepareCoordination, this, pid, userData));
    if (ret != RET_OK) {
        FI_HILOGE("On prepare coordination failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(userData);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::UnprepareCoordination(int32_t userData, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnUnprepareCoordination, this, pid, userData));
    if (ret != RET_OK) {
        FI_HILOGE("OnUnprepareCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(userData);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::ActivateCoordination(int32_t userData,
    const std::string &remoteNetworkId, int32_t startDeviceId, bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnActivateCoordination,
        this, pid, userData, remoteNetworkId, startDeviceId));
    if (ret != RET_OK) {
        FI_HILOGE("On activate coordination failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(userData);
    (void)(remoteNetworkId);
    (void)(startDeviceId);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::DeactivateCoordination(int32_t userData, bool isUnchained,
    bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnDeactivateCoordination, this, pid, userData, isUnchained));
    if (ret != RET_OK) {
        FI_HILOGE("On deactivate coordination failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(userData);
    (void)(isUnchained);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::GetCoordinationState(int32_t userData, const std::string &networkId,
    bool isCompatible)
{
    CALL_DEBUG_ENTER;
    (void)(isCompatible);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnGetCoordinationState, this, pid, userData, networkId));
    if (ret != RET_OK) {
        FI_HILOGE("OnGetCoordinationState failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(userData);
    (void)(networkId);
    FI_HILOGW("Get coordination state does not support");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::GetCoordinationState(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnGetCoordinationStateSync, this, udId, std::ref(state)));
    if (ret != RET_OK) {
        FI_HILOGE("OnGetCoordinationStateSync failed, ret:%{public}d", ret);
        return ret;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#else
    (void)(udId);
    FI_HILOGW("Get coordination state does not support");
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::AddDraglistener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t session = GetCallingPid();
#else
    int32_t pid = GetCallingPid();
    SessionPtr session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
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
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t session = GetCallingPid();
#else
    int32_t pid = GetCallingPid();
    SessionPtr session = GetSession(GetClientFd(pid));
    if (session == nullptr) {
        FI_HILOGW("Session is nullptr");
        return RET_OK;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::RemoveListener, &dragMgr_, session));
    if (ret != RET_OK) {
        FI_HILOGE("Remove listener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::AddSubscriptListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t session = GetCallingPid();
#else
    int32_t pid = GetCallingPid();
    SessionPtr session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::AddSubscriptListener, &dragMgr_, session));
    if (ret != RET_OK) {
        FI_HILOGE("AddListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::RemoveSubscriptListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t session = GetCallingPid();
#else
    int32_t pid = GetCallingPid();
    SessionPtr session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::RemoveSubscriptListener, &dragMgr_, session));
    if (ret != RET_OK) {
        FI_HILOGE("AddListener failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::StartDrag(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t session = GetCallingPid();
#else
    int32_t pid = GetCallingPid();
    AddSessionDeletedCallback(pid, std::bind(&DragManager::OnSessionLost, &dragMgr_, std::placeholders::_1));
    SessionPtr session = GetSession(GetClientFd(pid));
    CHKPR(session, RET_ERR);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::StartDrag, &dragMgr_, std::cref(dragData), session));
    if (ret != RET_OK) {
        FI_HILOGE("StartDrag failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::StopDrag(const DragDropResult &dropResult)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::StopDrag, &dragMgr_, dropResult, std::string()));
    if (ret != RET_OK) {
        FI_HILOGE("StopDrag failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::SetDragWindowVisible(bool visible, bool isForce)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::OnSetDragWindowVisible, &dragMgr_, visible, isForce));
    if (ret != RET_OK) {
        FI_HILOGE("On set drag window visible failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::EnterTextEditorArea(bool enable)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::EnterTextEditorArea, &dragMgr_, enable));
    if (ret != RET_OK) {
        FI_HILOGE("Enter Text Editor Area failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetShadowOffset(ShadowOffset &shadowOffset)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(std::bind(&DragManager::OnGetShadowOffset,
        &dragMgr_, std::ref(shadowOffset)));
    if (ret != RET_OK) {
        FI_HILOGE("Get shadow offset failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::UpdateShadowPic, &dragMgr_, std::cref(shadowInfo)));
    if (ret != RET_OK) {
        FI_HILOGE("Update shadow picture failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetDragData(DragData &dragData)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetDragData, &dragMgr_, std::ref(dragData)));
    if (ret != RET_OK) {
        FI_HILOGE("Get drag data failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetDragState(DragState &dragState)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(static_cast<int32_t(DragManager::*)(DragState&)>(&DragManager::GetDragState),
            &dragMgr_, std::ref(dragState)));
    if (ret != RET_OK) {
        FI_HILOGE("Get drag state failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    int32_t tid = static_cast<int32_t>(GetCallingTokenID());
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::UpdateDragStyle, &dragMgr_, style, pid, tid));
    if (ret != RET_OK) {
        FI_HILOGE("Update drag style failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetUdKey(std::string &udKey)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetUdKey, &dragMgr_, std::ref(udKey)));
    if (ret != RET_OK) {
        FI_HILOGE("Get udkey failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetDragTargetPid()
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetDragTargetPid, &dragMgr_));
    if (ret != RET_OK) {
        FI_HILOGE("Get drag target pid failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetDragAction(DragAction &dragAction)
{
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetDragAction, &dragMgr_, std::ref(dragAction)));
    if (ret != RET_OK) {
        FI_HILOGE("Get drag action failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetExtraInfo(std::string &extraInfo)
{
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetExtraInfo, &dragMgr_, std::ref(extraInfo)));
    if (ret != RET_OK) {
        FI_HILOGE("Get extraInfo failed, ret:%{public}d", ret);
    }
    return ret;
}

#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
int32_t DeviceStatusService::OnRegisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_ADD_LISTENER;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnUnregisterCoordinationListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    SessionPtr sess = GetSession(GetClientFd(pid));
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::LISTENER;
    event->sess = sess;
    COOR_EVENT_MGR->RemoveCoordinationEvent(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnPrepareCoordination(int32_t pid, int32_t userData)
{
    CALL_DEBUG_ENTER;
    COOR_SM->PrepareCoordination();
    std::string networkId;
    CoordinationMessage msg = CoordinationMessage::PREPARE;
    NetPacket pkt(MessageId::COORDINATION_MESSAGE);
    pkt << userData << networkId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnUnprepareCoordination(int32_t pid, int32_t userData)
{
    CALL_DEBUG_ENTER;
    COOR_SM->UnprepareCoordination();
    std::string networkId;
    CoordinationMessage msg = CoordinationMessage::UNPREPARE;
    NetPacket pkt(MessageId::COORDINATION_MESSAGE);
    pkt << userData << networkId << static_cast<int32_t>(msg);
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return RET_ERR;
    }
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return MSG_SEND_FAIL;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnActivateCoordination(int32_t pid,
    int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::START;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    if (COOR_SM->GetCurrentCoordinationState() == CoordinationState::STATE_OUT ||
        (COOR_SM->GetCurrentCoordinationState() == CoordinationState::STATE_FREE &&
        COOR_DEV_MGR->IsRemote(startDeviceId))) {
        FI_HILOGW("It is currently worn out");
        NetPacket pkt(event->msgId);
        pkt << userData << "" << static_cast<int32_t>(CoordinationMessage::ACTIVATE_SUCCESS);
        if (pkt.ChkRWError()) {
            FI_HILOGE("Failed to write packet data");
            return RET_ERR;
        }
        if (!sess->SendMsg(pkt)) {
            FI_HILOGE("Sending message failed");
            return RET_ERR;
        }
        return RET_OK;
    }
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->ActivateCoordination(remoteNetworkId, startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("On activate coordination error, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::OnDeactivateCoordination(int32_t pid, int32_t userData, bool isUnchained)
{
    CALL_DEBUG_ENTER;
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STOP;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_MESSAGE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->DeactivateCoordination(isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("On deactivate coordination failed, ret:%{public}d", ret);
        COOR_EVENT_MGR->OnErrorMessage(event->type, CoordinationMessage(ret));
    }
    return ret;
}

int32_t DeviceStatusService::OnGetCoordinationState(
    int32_t pid, int32_t userData, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    SessionPtr sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationEventManager::EventInfo> event = new (std::nothrow) CoordinationEventManager::EventInfo();
    CHKPR(event, RET_ERR);
    event->type = CoordinationEventManager::EventType::STATE;
    event->sess = sess;
    event->msgId = MessageId::COORDINATION_GET_STATE;
    event->userData = userData;
    COOR_EVENT_MGR->AddCoordinationEvent(event);
    int32_t ret = COOR_SM->GetCoordinationState(networkId);
    if (ret != RET_OK) {
        FI_HILOGE("Get coordination state failed");
    }
    return ret;
}

int32_t DeviceStatusService::OnGetCoordinationStateSync(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
    if (int32_t ret = COOR_SM->GetCoordinationState(udId, state); ret != RET_OK) {
        FI_HILOGE("GetCoordinationState failed, ret:%{public}d", ret);
        return ret;
    }
    return RET_OK;
}

int32_t DeviceStatusService::OnAddHotAreaListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationHotArea::HotAreaInfo> event = new (std::nothrow) CoordinationHotArea::HotAreaInfo();
    CHKPR(event, RET_ERR);
    event->sess = sess;
    event->msgId = MessageId::HOT_AREA_ADD_LISTENER;
    HOT_AREA->AddHotAreaListener(event);
    return RET_OK;
}

int32_t DeviceStatusService::OnRemoveHotAreaListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    auto sess = GetSession(GetClientFd(pid));
    CHKPR(sess, RET_ERR);
    sptr<CoordinationHotArea::HotAreaInfo> event = new (std::nothrow) CoordinationHotArea::HotAreaInfo();
    CHKPR(event, RET_ERR);
    event->sess = sess;
    HOT_AREA->RemoveHotAreaListener(event);
    return RET_OK;
}
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION

int32_t DeviceStatusService::AddHotAreaListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnAddHotAreaListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("Failed to add hot area listener, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::RemoveHotAreaListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#ifndef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DeviceStatusService::OnRemoveHotAreaListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("Failed to remove hot area listener, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#endif // OHOS_BUILD_ENABLE_COORDINATION
    return RET_OK;
}

int32_t DeviceStatusService::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::UpdatePreviewStyle, &dragMgr_, previewStyle));
    if (ret != RET_OK) {
        FI_HILOGE("UpdatePreviewStyle failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle,
    const PreviewAnimation &animation)
{
    CALL_DEBUG_ENTER;
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::UpdatePreviewStyleWithAnimation, &dragMgr_, previewStyle, animation));
    if (ret != RET_OK) {
        FI_HILOGE("UpdatePreviewStyleWithAnimation failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t DeviceStatusService::GetDragSummary(std::map<std::string, int64_t> &summarys)
{
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::GetDragSummary, &dragMgr_, std::ref(summarys)));
    if (ret != RET_OK) {
        FI_HILOGE("Failed to get drag summarys, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DeviceStatusService::AddPrivilege()
{
    CALL_DEBUG_ENTER;
    int32_t tokenId = static_cast<int32_t>(GetCallingTokenID());
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&DragManager::AddPrivilege, &dragMgr_, tokenId));
    if (ret != RET_OK) {
        FI_HILOGE("Failed to add privilege, ret:%{public}d", ret);
        return RET_ERR;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
