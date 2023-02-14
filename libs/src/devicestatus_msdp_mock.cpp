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

#include "devicestatus_msdp_mock.h"

#include <cerrno>
#include <string>
#include <unistd.h>

#include <linux/netlink.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t TIMER_INTERVAL = 3;
constexpr int32_t ERR_INVALID_FD = -1;
constexpr int32_t ERR_NG = -1;
constexpr int32_t TIMER_MS = 1000;
DeviceStatusMsdpMock* g_msdpMock = nullptr;
} // namespace

std::vector<int32_t> DeviceStatusMsdpMock::enabledType_ =
    std::vector<int32_t> (static_cast<int32_t>(DevicestatusDataUtils::DevicestatusType::TYPE_MAX),
    static_cast<int32_t>(DevicestatusDataUtils::Value::INVALID));

bool DeviceStatusMsdpMock::Init()
{
    DEV_HILOGD(SERVICE, "DeviceStatusMsdpMock: Enter");
    if (dataParse_ == nullptr) {
        dataParse_ = std::make_unique<DeviceStatusDataParse>();
    }
    InitMockStore();
    InitTimer();
    StartThread();
    DEV_HILOGD(SERVICE, "DeviceStatusMsdpMock: Exit");
    return true;
}

void DeviceStatusMsdpMock::InitMockStore() {}

ErrCode DeviceStatusMsdpMock::RegisterCallback(std::shared_ptr<MsdpAlgorithmCallback> callback)
{
    std::lock_guard lock(mutex_);
    callback_ = callback;
    return RET_OK;
}

ErrCode DeviceStatusMsdpMock::UnregisterCallback()
{
    std::lock_guard lock(mutex_);
    callback_ = nullptr;
    return RET_OK;
}

ErrCode DeviceStatusMsdpMock::Enable(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (type < DevicestatusDataUtils::DevicestatusType::TYPE_STILL ||
        type >= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
        DEV_HILOGE(SERVICE, "Type error");
    }
    enabledType_[type] = static_cast<int32_t>(DevicestatusDataUtils::Value::VALID);
    Init();
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpMock::Disable(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGD(SERVICE, "Enter");
    scFlag_ = false;
    CloseTimer();
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpMock::DisableCount(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGD(SERVICE, "Enter");
    enabledType_[type] = static_cast<int32_t>(DevicestatusDataUtils::Value::INVALID);
    dataParse_->DisableCount(type);
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpMock::NotifyMsdpImpl(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (g_msdpMock == nullptr) {
        DEV_HILOGE(SERVICE, "g_msdpMock is nullptr");
        return ERR_NG;
    }
    if (g_msdpMock->GetCallbackImpl() == nullptr) {
        DEV_HILOGE(SERVICE, "callbacksImpl is nullptr");
        return ERR_NG;
    }
    g_msdpMock->GetCallbackImpl()->OnResult(data);

    return ERR_OK;
}

void DeviceStatusMsdpMock::InitTimer()
{
    DEV_HILOGD(SERVICE, "Enter");
    epFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epFd_ == -1) {
        DEV_HILOGE(SERVICE, "create epoll fd failed");
        return;
    }
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGE(SERVICE, "create timer fd failed");
        close(epFd_);
        epFd_ = ERR_INVALID_FD;
        return;
    }
    SetTimerInterval(TIMER_INTERVAL);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbacks_.insert(std::make_pair(timerFd_, &DeviceStatusMsdpMock::TimerCallback));
    if (RegisterTimerCallback(timerFd_, EVENT_TIMER_FD)) {
        DEV_HILOGE(SERVICE, "register timer fd failed");
        return;
    }
}

void DeviceStatusMsdpMock::SetTimerInterval(int32_t interval)
{
    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGE(SERVICE, "create timer fd failed");
        return;
    }
    if (interval != 0) {
        timerInterval_ = interval;
    }
    if (interval < 0) {
        interval = 0;
    }
    struct itimerspec itval;
    itval.it_interval.tv_sec = interval;
    itval.it_interval.tv_nsec = 0;
    itval.it_value.tv_sec = interval;
    itval.it_value.tv_nsec = 0;
    if (timerfd_settime(timerFd_, 0, &itval, nullptr) == -1) {
        DEV_HILOGE(SERVICE, "set timer failed");
        return;
    }
}

void DeviceStatusMsdpMock::CloseTimer()
{
    DEV_HILOGD(SERVICE, "Enter");
    close(timerFd_);
    DEV_HILOGD(SERVICE, "Exit");
}

void DeviceStatusMsdpMock::TimerCallback()
{
    uint64_t timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        DEV_HILOGE(SERVICE, "read timer fd failed");
        return;
    }
    GetDeviceStatusData();
}

void DeviceStatusMsdpMock::GetDeviceStatusData()
{
    DevicestatusDataUtils::DevicestatusData data;
    for (int32_t i = int(DevicestatusDataUtils::DevicestatusType::TYPE_STILL);
        i <= DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN; ++i) {
        if (enabledType_[i] == static_cast<int32_t>(DevicestatusDataUtils::Value::VALID)) {
            DevicestatusDataUtils::DevicestatusType type = DevicestatusDataUtils::DevicestatusType(i);
            DEV_HILOGE(SERVICE, "type:%{public}d", type);
            if (dataParse_ == nullptr) {
                DEV_HILOGE(SERVICE, "dataParse_ is nullptr");
                return;
            }
            dataParse_->ParseDeviceStatusData(data, type);
            NotifyMsdpImpl(data);
        }
    }
}

int32_t DeviceStatusMsdpMock::RegisterTimerCallback(const int32_t fd, const EventType et)
{
    DEV_HILOGD(SERVICE, "Enter");
    struct epoll_event ev;
    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }

    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        DEV_HILOGE(SERVICE, "epoll_ctl failed, error num:%{public}d", errno);
        return ERR_NG;
    }

    return ERR_OK;
}

void DeviceStatusMsdpMock::StartThread()
{
    DEV_HILOGD(SERVICE, "Enter");
    std::make_unique<std::thread>(&DeviceStatusMsdpMock::LoopingThreadEntry, this)->detach();
}

void DeviceStatusMsdpMock::LoopingThreadEntry()
{
    if (callbacks_.empty()) {
        DEV_HILOGD(SERVICE, "callbacks_ is empty");
        return;
    }
    size_t cbct = callbacks_.size();
    struct epoll_event events[cbct];
    while (alive_) {
        int32_t nevents = epoll_wait(epFd_, events, cbct, TIMER_INTERVAL * TIMER_MS);
        if (nevents == -1 || nevents == 0) {
            continue;
        }
        for (int32_t n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                DeviceStatusMsdpMock *func = const_cast<DeviceStatusMsdpMock *>(this);
                (callbacks_.find(events[n].data.fd)->second)(func);
            }
        }
    }
}

extern "C" DevicestatusMsdpInterface *Create(void)
{
    DEV_HILOGD(SERVICE, "Enter");
    g_msdpMock = new DeviceStatusMsdpMock();
    return g_msdpMock;
}

extern "C" void Destroy(const DevicestatusMsdpInterface* algorithm)
{
    DEV_HILOGD(SERVICE, "Enter");
    delete algorithm;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
