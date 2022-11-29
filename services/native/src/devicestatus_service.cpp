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
#include <ipc_skeleton.h>
#include <csignal>
#include <sys/signalfd.h>
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "devicestatus_permission.h"
#include "devicestatus_common.h"
#include "devicestatus_dumper.h"
#include "devicestatus_define.h"
#include "hisysevent.h"
#include "hitrace_meter.h"
#include "timer_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace OHOS::HiviewDFX;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DevicestatusService" };
auto ms = DelayedSpSingleton<DevicestatusService>::GetInstance();
const bool G_REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(ms.GetRefPtr());
} // namespace

struct device_status_epoll_event {
    int32_t fd { 0 };
    EpollEventType event_type { EPOLL_EVENT_BEGIN };
};

DevicestatusService::DevicestatusService() : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{
    DEV_HILOGD(SERVICE, "Add SystemAbility");
}

DevicestatusService::~DevicestatusService() {}

void DevicestatusService::OnDump()
{
    DEV_HILOGI(SERVICE, "OnDump");
}

void DevicestatusService::OnStart()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (ready_) {
        DEV_HILOGE(SERVICE, "OnStart is ready, nothing to do");
        return;
    }

    if (!Init()) {
        DEV_HILOGE(SERVICE, "OnStart call init fail");
        return;
    }
    if (!Publish(DelayedSpSingleton<DevicestatusService>::GetInstance())) {
        DEV_HILOGE(SERVICE, "OnStart register to system ability manager failed");
        return;
    }
    ready_ = true;
    t_ = std::thread(std::bind(&DevicestatusService::OnThread, this));
    t_.join();
}

void DevicestatusService::OnStop()
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

int DevicestatusService::Dump(int fd, const std::vector<std::u16string>& args)
{
    DEV_HILOGI(SERVICE, "dump DeviceStatusServiceInfo");
    if (fd < 0) {
        DEV_HILOGE(SERVICE, "fd is invalid");
        return RET_NG;
    }
    DevicestatusDumper &deviceStatusDumper = DevicestatusDumper::GetInstance();
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

    DevicestatusDataUtils::DevicestatusType type;
    std::vector<DevicestatusDataUtils::DevicestatusData> datas;
    for (type = DevicestatusDataUtils::TYPE_HIGH_STILL;
        type <= DevicestatusDataUtils::TYPE_LID_OPEN;
        type = (DevicestatusDataUtils::DevicestatusType)(type+1)) {
        DevicestatusDataUtils::DevicestatusData data = GetCache(type);
        if (data.value != DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID) {
            datas.emplace_back(data);
        }
    }
    deviceStatusDumper.ParseCommand(fd, argList, datas);
    return RET_OK;
}

bool DevicestatusService::Init()
{
    DEV_HILOGI(SERVICE, "Enter");

    if (!devicestatusManager_) {
        devicestatusManager_ = std::make_shared<DevicestatusManager>(ms);
    }
    if (!devicestatusManager_->Init()) {
        DEV_HILOGE(SERVICE, "OnStart init fail");
        return false;
    }
    if (EpollCreate(MAX_EVENT_SIZE) < 0) {
        FI_HILOGE("Create epoll failed");
        return EPOLL_CREATE_FAIL;
    }
    if (!InitDelegateTasks()) {
        FI_HILOGE("Delegate tasks init failed");
        return false;
    }
    if (TimerMgr->Init() != RET_OK) {
        FI_HILOGE("TimerMgr init failed");
    }
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    coordinationHandler.Init(*this);
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return true;
}

void DevicestatusService::Subscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "UnSubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DevicestatusDumper::GetInstance().SaveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceSubscribeStart");
    devicestatusManager_->Subscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, true);
}

void DevicestatusService::UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type,
    const sptr<IdevicestatusCallback>& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DEV_HILOGI(SERVICE, "UnSubscribe func is nullptr");
        return;
    }

    auto appInfo = std::make_shared<AppInfo>();
    appInfo->uid = GetCallingUid();
    appInfo->pid = GetCallingPid();
    appInfo->tokenId = GetCallingTokenID();
    devicestatusManager_->GetPackageName(appInfo->tokenId, appInfo->packageName);
    appInfo->type = type;
    appInfo->callback = callback;
    DevicestatusDumper::GetInstance().RemoveAppInfo(appInfo);
    StartTrace(HITRACE_TAG_MSDP, "serviceUnSubscribeStart");
    devicestatusManager_->UnSubscribe(type, callback);
    FinishTrace(HITRACE_TAG_MSDP);
    ReportMsdpSysEvent(type, false);
}

DevicestatusDataUtils::DevicestatusData DevicestatusService::GetCache(const \
    DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devicestatusManager_ == nullptr) {
        DevicestatusDataUtils::DevicestatusData data = {type, DevicestatusDataUtils::DevicestatusValue::VALUE_EXIT};
        data.value = DevicestatusDataUtils::DevicestatusValue::VALUE_INVALID;
        DEV_HILOGI(SERVICE, "GetLatestDevicestatusData func is nullptr,return default!");
        return data;
    }
    return devicestatusManager_->GetLatestDevicestatusData(type);
}

void DevicestatusService::ReportMsdpSysEvent(const DevicestatusDataUtils::DevicestatusType& type, bool enable)
{
    auto uid = this->GetCallingUid();
    auto callerToken = this->GetCallingTokenID();
    std::string packageName("");
    devicestatusManager_->GetPackageName(callerToken, packageName);
    if (enable) {
        HiSysEvent::Write(HiSysEvent::Domain::MSDP, "SUBSCRIBE", HiSysEvent::EventType::STATISTIC,
            "UID", uid, "PKGNAME", packageName, "TYPE", type);
        return;
    }
    HiSysEvent::Write(HiSysEvent::Domain::MSDP, "UNSUBSCRIBE", HiSysEvent::EventType::STATISTIC,
        "UID", uid, "PKGNAME", packageName, "TYPE", type);
}

int32_t DevicestatusService::AllocSocketFd(const std::string &programName, const int32_t moduleType,
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

void DevicestatusService::OnConnected(SessionPtr s)
{
    CHKPV(s);
    FI_HILOGI("fd:%{public}d", s->GetFd());
}
void DevicestatusService::OnDisconnected(SessionPtr s)
{
    CHKPV(s);
    FI_HILOGW("Enter, session desc:%{public}s, fd:%{public}d", s->GetDescript().c_str(), s->GetFd());
}   

int32_t DevicestatusService::AddEpoll(EpollEventType type, int32_t fd)
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

int32_t DevicestatusService::DelEpoll(EpollEventType type, int32_t fd)
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

bool DevicestatusService::IsRunning() const
{
    return (state_ == ServiceRunningState::STATE_RUNNING);
}

bool DevicestatusService::InitDelegateTasks()
{
    CALL_DEBUG_ENTER;
    if (!delegateTasks_.Init()) {
        FI_HILOGE("The delegate task init failed");
        return false;
    }
    auto ret = AddEpoll(EPOLL_EVENT_ETASK, delegateTasks_.GetReadFd());
    if (ret <  0) {
        FI_HILOGE("AddEpoll error ret:%{public}d", ret);
        EpollClose();
        return false;
    }
    FI_HILOGI("AddEpoll, epollfd:%{public}d,fd:%{public}d", epollFd_, delegateTasks_.GetReadFd());
    return true;
}

void DevicestatusService::OnThread()
{
    SetThreadName(std::string("mmi_service"));
    uint64_t tid = GetThisThreadId();
    delegateTasks_.SetWorkerThreadId(tid);
    FI_HILOGD("Main worker thread start. tid:%{public}" PRId64 "", tid);
    while (state_ == ServiceRunningState::STATE_RUNNING) {
        epoll_event ev[MAX_EVENT_SIZE] = {};
        int32_t timeout = TimerMgr->CalcNextDelay();
        FI_HILOGD("timeout:%{public}d", timeout);
        int32_t count = EpollWait(ev[0], MAX_EVENT_SIZE, timeout, -1);
        for (int32_t i = 0; i < count && state_ == ServiceRunningState::STATE_RUNNING; i++) {
            auto epollEvent = reinterpret_cast<device_status_epoll_event*>(ev[i].data.ptr);
            CHKPC(epollEvent);
            if (epollEvent->event_type == EPOLL_EVENT_SOCKET) {
                OnEpollEvent(ev[i]);
            } else if (epollEvent->event_type == EPOLL_EVENT_SIGNAL) {
                OnSignalEvent(epollEvent->fd);
            } else if (epollEvent->event_type == EPOLL_EVENT_ETASK) {
                OnDelegateTask(ev[i]);
            } else {
                FI_HILOGW("Unknown epoll event type:%{public}d", epollEvent->event_type);
            }
        }
        TimerMgr->ProcessTimers();
        if (state_ != ServiceRunningState::STATE_RUNNING) {
            break;
        }
    }
    FI_HILOGD("Main worker thread stop. tid:%{public}" PRId64 "", tid);
}
void DevicestatusService::OnSignalEvent(int32_t signalFd)
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

void DevicestatusService::OnDelegateTask(epoll_event& ev)
{
    if ((ev.events & EPOLLIN) == 0) {
        FI_HILOGW("Not epollin");
        return;
    }
    DelegateTasks::TaskData data = {};
    auto res = read(delegateTasks_.GetReadFd(), &data, sizeof(data));
    if (res == -1) {
        FI_HILOGW("Read failed erron:%{public}d", errno);
    }
    FI_HILOGD("RemoteRequest notify td:%{public}" PRId64 ",std:%{public}" PRId64 ""
        ",taskId:%{public}d", GetThisThreadId(), data.tid, data.taskId);
    delegateTasks_.ProcessTasks();
}

int32_t DevicestatusService::RegisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnRegisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("OnRegisterCoordinationListener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}

int32_t DevicestatusService::UnregisterCoordinationListener()
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnUnregisterCoordinationListener, this, pid));
    if (ret != RET_OK) {
        FI_HILOGE("OnUnregisterCoordinationListener failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}

int32_t DevicestatusService::EnableInputDeviceCoordination(int32_t userData, bool enable)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnEnableInputDeviceCoordination, this, pid, userData, enabled));
    if (ret != RET_OK) {
        FI_HILOGE("OnEnableInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(enable);
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}

int32_t DevicestatusService::StartInputDeviceCoordination(int32_t userData,
    const std::string &sinkDeviceId, int32_t srcInputDeviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnStartInputDeviceCoordination,
        this, pid, userData, sinkDeviceId, srcInputDeviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnStartInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
    (void)(sinkDeviceId);
    (void)(srcInputDeviceId);
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}

int32_t DevicestatusService::StopDeviceCoordination(int32_t userData)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnStopInputDeviceCoordination, this, pid, userData));
    if (ret != RET_OK) {
        FI_HILOGE("OnStopInputDeviceCoordination failed, ret:%{public}d", ret);
        return ret;
    }
#else
    (void)(userData);
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}

int32_t DevicestatusService::GetInputDeviceCoordinationState(int32_t userData, const std::string &deviceId)
{
    CALL_DEBUG_ENTER;
#ifdef OHOS_BUILD_ENABLE_COOPERATE
    int32_t pid = GetCallingPid();
    int32_t ret = delegateTasks_.PostSyncTask(
        std::bind(&coordinationHandler.OnGetInputDeviceCoordinationState, this, pid, userData, deviceId));
    if (ret != RET_OK) {
        FI_HILOGE("OnGetInputDeviceCoordinationState failed, ret:%{public}d", ret);
        return RET_ERR;
    }
#else
    (void)(userData);
    (void)(deviceId);
    FI_HILOGW("Get input device cooperate state does not support");
#endif // OHOS_BUILD_ENABLE_COOPERATE
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
