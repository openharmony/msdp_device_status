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

#include "devicestatus_msdp_rdb.h"

#include <string>
#include <cerrno>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <linux/netlink.h>

#include "devicestatus_common.h"

using namespace OHOS::NativeRdb;
namespace OHOS {
namespace Msdp {
namespace {
const std::string DATABASE_NAME = "/data/MsdpStub.db";
constexpr int32_t TIMER_INTERVAL = 3;
constexpr int32_t ERR_INVALID_FD = -1;
constexpr int32_t READ_RDB_WAIT_TIME = 30;
std::unique_ptr<DevicestatusMsdpRdb> g_msdpRdb = std::make_unique<DevicestatusMsdpRdb>();
constexpr int32_t ERR_NG = -1;
DevicestatusMsdpRdb* g_rdb;
}

bool DevicestatusMsdpRdb::Init()
{
    DEV_HILOGD(SERVICE, "DevicestatusMsdpRdbInit: Enter");
    InitRdbStore();
    InitTimer();
    StartThread();
    DEV_HILOGD(SERVICE, "DevicestatusMsdpRdbInit: Exit");
    return true;
}

void DevicestatusMsdpRdb::InitRdbStore()
{}

void DevicestatusMsdpRdb::RegisterCallback(const std::shared_ptr<MsdpAlgorithmCallback>& callback)
{
    callbacksImpl_ = callback;
}

void DevicestatusMsdpRdb::UnregisterCallback()
{
    callbacksImpl_ = nullptr;
}

void DevicestatusMsdpRdb::Enable()
{
    DEV_HILOGD(SERVICE, "Enter");
    Init();
    DEV_HILOGD(SERVICE, "Exit");
}

void DevicestatusMsdpRdb::Disable()
{
    DEV_HILOGD(SERVICE, "Enter");
    CloseTimer();
    DEV_HILOGD(SERVICE, "Exit");
}

ErrCode DevicestatusMsdpRdb::NotifyMsdpImpl(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (g_rdb == nullptr) {
        DEV_HILOGE(SERVICE, "g_rdb is nullptr");
        return ERR_NG;
    }
    if (g_rdb->GetCallbacksImpl() == nullptr) {
        DEV_HILOGD(SERVICE, "callbacksImpl is nullptr");
        return ERR_NG;
    }
    g_rdb->GetCallbacksImpl()->OnResult(data);

    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusMsdpRdb::SaveRdbData(
    const DevicestatusDataUtils::DevicestatusData& data)
{
    for (auto iter = rdbDataMap_.begin(); iter != rdbDataMap_.end(); ++iter) {
        if (iter->first == data.type) {
            if (iter->second != data.value) {
                notifyFlag_ = true;
                iter->second = data.value;
            }
            DEV_HILOGD(SERVICE, "data is not changed");
            return data;
        }
    }

    rdbDataMap_.insert(std::make_pair(data.type, data.value));
    notifyFlag_ = true;

    DEV_HILOGD(SERVICE, "devicestatusType_ = %{public}d, devicestatusStatus_ = %{public}d",
        devicestatusType_, devicestatusStatus_);

    return data;
}

int32_t DevicestatusMsdpRdb::TrigerData(const std::unique_ptr<NativeRdb::ResultSet> &resultSet)
{
    int32_t columnIndex;
    int32_t intVal;
    if (resultSet == nullptr) {
        DEV_HILOGE(SERVICE, "resultSet is nullptr");
        return ERR_NG;
    }
    int32_t ret = resultSet->GetColumnIndex("ID", columnIndex);
    DEV_HILOGD(SERVICE, "TrigerDatabaseObserver GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckID: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGD(SERVICE, "ret = %{public}d, id = %{public}d", ret, intVal);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckID: GetValue failed");
        return -1;
    }

    ret = resultSet->GetColumnIndex("DEVICESTATUS_TYPE", columnIndex);
    DEV_HILOGD(SERVICE, "DEVICESTATUS_TYPE GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusType: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGD(SERVICE, "ret = %{public}d, DevicestatusType = %{public}d", ret, intVal);
    devicestatusType_ = intVal;
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusType: GetValue failed");
        return -1;
    }

    ret = resultSet->GetColumnIndex("DEVICESTATUS_STATUS", columnIndex);
    DEV_HILOGD(SERVICE, "DEVICESTATUS_STATUS GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusStatus: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGD(SERVICE, "ret = %{public}d, DevicestatusStatus = %{public}d", ret, intVal);
    devicestatusStatus_ = intVal;
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusStatus: GetValue failed");
        return -1;
    }

    return ERR_OK;
}

int32_t DevicestatusMsdpRdb::TrigerDatabaseObserver()
{
    DEV_HILOGD(SERVICE, "Enter");

    if (store_ == nullptr) {
        sleep(READ_RDB_WAIT_TIME);
        InitRdbStore();
        return -1;
    }

    std::unique_ptr<ResultSet> resultSet =
        store_->QuerySql("SELECT * FROM DEVICESTATUSSENSOR WHERE ID = (SELECT max(ID) from DEVICESTATUSSENSOR)");

    if (resultSet == nullptr) {
        DEV_HILOGE(SERVICE, "database is not exist");
        return -1;
    }

    int32_t ret = resultSet->GoToFirstRow();
    DEV_HILOGD(SERVICE, "GoToFirstRow = %{public}d", ret);
    if (ret != ERR_OK) {
        sleep(READ_RDB_WAIT_TIME);
        DEV_HILOGE(SERVICE, "database observer is null");
        return -1;
    }

    if (TrigerData(resultSet) != ERR_OK) {
        DEV_HILOGE(SERVICE, "triger data failed");
        return -1;
    }

    ret = resultSet->Close();
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "close database observer failed");
        return -1;
    }

    DevicestatusDataUtils::DevicestatusData data;
    data.type = (DevicestatusDataUtils::DevicestatusType)devicestatusType_;
    data.value = (DevicestatusDataUtils::DevicestatusValue)devicestatusStatus_;

    SaveRdbData(data);
    DEV_HILOGD(SERVICE, "notifyFlag_ is %{public}d", notifyFlag_);
    if (notifyFlag_) {
        NotifyMsdpImpl(data);
        notifyFlag_ = false;
    }

    return ERR_OK;
}

void DevicestatusMsdpRdb::InitTimer()
{
    DEV_HILOGD(SERVICE, "Enter");
    epFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epFd_ == -1) {
        DEV_HILOGD(SERVICE, "create epoll fd failed");
        return;
    }
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGD(SERVICE, "create timer fd failed");
        return;
    }
    SetTimerInterval(TIMER_INTERVAL);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbacks_.insert(std::make_pair(timerFd_, &DevicestatusMsdpRdb::TimerCallback));
    if (RegisterTimerCallback(timerFd_, EVENT_TIMER_FD)) {
        DEV_HILOGD(SERVICE, "register timer fd failed");
        return;
    }
}

void DevicestatusMsdpRdb::SetTimerInterval(int32_t interval)
{
    struct itimerspec itval;

    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGD(SERVICE, "create timer fd failed");
        return;
    }

    timerInterval_ = interval;

    if (interval < 0) {
        interval = 0;
    }

    itval.it_interval.tv_sec = interval;
    itval.it_interval.tv_nsec = 0;
    itval.it_value.tv_sec = interval;
    itval.it_value.tv_nsec = 0;

    if (timerfd_settime(timerFd_, 0, &itval, nullptr) == -1) {
        DEV_HILOGD(SERVICE, "set timer failed");
        return;
    }

    return;
}

void DevicestatusMsdpRdb::CloseTimer()
{
    DEV_HILOGD(SERVICE, "Enter");
    close(timerFd_);
    DEV_HILOGD(SERVICE, "Exit");
}

void DevicestatusMsdpRdb::TimerCallback()
{
    unsigned long long timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        DEV_HILOGD(SERVICE, "read timer fd failed");
        return;
    }
    TrigerDatabaseObserver();
}

int32_t DevicestatusMsdpRdb::RegisterTimerCallback(const int32_t fd, const EventType et)
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
        DEV_HILOGD(SERVICE, "epoll_ctl failed, error num =%{public}d", errno);
        return -1;
    }

    return 0;
}

void DevicestatusMsdpRdb::StartThread()
{
    DEV_HILOGD(SERVICE, "Enter");
    std::make_unique<std::thread>(&DevicestatusMsdpRdb::LoopingThreadEntry, this)->detach();
}

void DevicestatusMsdpRdb::LoopingThreadEntry()
{
    size_t cbct = callbacks_.size();
    struct epoll_event events[cbct];

    while (true) {
        int32_t timeout = -1;

        int32_t nevents = epoll_wait(epFd_, events, cbct, timeout);
        if (nevents == -1) {
            continue;
        }
        for (int32_t n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                DevicestatusMsdpRdb *func = const_cast<DevicestatusMsdpRdb *>(this);
                (callbacks_.find(events[n].data.fd)->second)(func);
            }
        }
    }
}

int32_t InsertOpenCallback::OnCreate(RdbStore &store)
{
    DEV_HILOGD(SERVICE, "Enter");
    return ERR_OK;
}

int32_t InsertOpenCallback::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    DEV_HILOGD(SERVICE, "Enter");
    return ERR_OK;
}

extern "C" DevicestatusMsdpInterface *Create(void)
{
    DEV_HILOGD(SERVICE, "Enter");
    g_rdb = new DevicestatusMsdpRdb();
    return g_rdb;
}

extern "C" void Destroy(DevicestatusMsdpInterface* algorithm)
{
    DEV_HILOGD(SERVICE, "Enter");
    delete algorithm;
}
}
}
