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
#include "dummy_values_bucket.h"
#include "devicestatus_common.h"

using namespace OHOS::NativeRdb;
namespace OHOS {
namespace Msdp {
namespace {
const std::string DATABASE_NAME = "/data/MsdpStub.db";
const int TIMER_INTERVAL = 3;
const int ERR_INVALID_FD = -1;
const int READ_RDB_WAIT_TIME = 30;
std::unique_ptr<DevicestatusMsdpRdb> msdpRdb_ = std::make_unique<DevicestatusMsdpRdb>();
const int32_t ERR_NG = -1;
DevicestatusMsdpRdb* rdb;
}

bool DevicestatusMsdpRdb::Init()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusMsdpRdbInit: Enter");
    InitRdbStore();

    InitTimer();
    StartThread();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusMsdpRdbInit: Exit");
    return true;
}

void DevicestatusMsdpRdb::InitRdbStore()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    int errCode = ERR_OK;
    RdbStoreConfig config(DATABASE_NAME);
    InsertOpenCallback helper;
    store_ = RdbHelper::GetRdbStore(config, 1, helper, errCode);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

ErrCode DevicestatusMsdpRdb::RegisterCallback(std::shared_ptr<MsdpAlgorithmCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    callbacksImpl_ = callback;
    return ERR_OK;
}

ErrCode DevicestatusMsdpRdb::UnregisterCallback()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    return ERR_OK;
}

ErrCode DevicestatusMsdpRdb::Enable()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    Init();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpRdb::Disable()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    CloseTimer();
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}


ErrCode DevicestatusMsdpRdb::NotifyMsdpImpl(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    if (rdb->GetCallbacksImpl() == nullptr) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "callbacksImpl is nullptr");
        return ERR_NG;
    }
    rdb->GetCallbacksImpl()->OnResult(data);

    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusMsdpRdb::SaveRdbData(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    for (auto iter = rdbDataMap_.begin(); iter != rdbDataMap_.end(); ++iter) {
        if (iter->first == data.type) {
            if (iter->second != data.value) {
                notifyFlag_ = true;
                iter->second = data.value;
            }
            return data;
        }
    }

    rdbDataMap_.insert(std::make_pair(data.type, data.value));
    notifyFlag_ = true;

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "devicestatusType_ = %{public}d, devicestatusStatus_ = %{public}d",
        devicestatusType_, devicestatusStatus_);

    return data;
}

int DevicestatusMsdpRdb::TrigerData(std::unique_ptr<NativeRdb::ResultSet> &resultSet)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    int columnIndex;
    int intVal;

    int ret = resultSet->GetColumnIndex("ID", columnIndex);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "TrigerDatabaseObserver GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "CheckID: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "ret = %{public}d", ret);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "id = %{public}d", intVal);
    if (ret != ERR_OK) {
        return -1;
    }
    ret = resultSet->GetColumnIndex("DEVICESTATUS_TYPE", columnIndex);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DEVICESTATUS_TYPE GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "CheckDevicestatusType: GetColumnIndex failed");
        return -1;
    }

    ret = resultSet->GetInt(columnIndex, intVal);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "ret = %{public}d", ret);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusType = %{public}d", intVal);
    devicestatusType_ = intVal;
    if (ret != ERR_OK) {
        return -1;
    }

    ret = resultSet->GetColumnIndex("DEVICESTATUS_STATUS", columnIndex);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DEVICESTATUS_STATUS GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "CheckDevicestatusStatus: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "ret = %{public}d", ret);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusStatus = %{public}d", intVal);
    devicestatusStatus_ = intVal;
    if (ret != ERR_OK) {
        return -1;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

int DevicestatusMsdpRdb::TrigerDatabaseObserver()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    if (store_ == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "store_ is not exist");
        sleep(READ_RDB_WAIT_TIME);
        InitRdbStore();
        return -1;
    }

    std::unique_ptr<ResultSet> resultSet =
        store_->QuerySql("SELECT * FROM DEVICESTATUS WHERE ID = (SELECT max(ID) from DEVICESTATUS)");

    if (resultSet == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "database is not exist");
        return -1;
    }

    int ret = resultSet->GoToFirstRow();
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "GoToFirstRow = %{public}d", ret);
    if (ret != ERR_OK) {
        sleep(READ_RDB_WAIT_TIME);
        return -1;
    }

    if (TrigerData(resultSet) != ERR_OK) {
        return -1;
    }

    ret = resultSet->Close();
    if (ret != ERR_OK) {
        return -1;
    }

    DevicestatusDataUtils::DevicestatusData data;
    data.type = (DevicestatusDataUtils::DevicestatusType)devicestatusType_;
    data.value = (DevicestatusDataUtils::DevicestatusValue)devicestatusStatus_;

    SaveRdbData(data);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "msdp notifyFlag_ is %{public}d", notifyFlag_);
    if (notifyFlag_) {
        NotifyMsdpImpl(data);
        notifyFlag_ = false;
    }

    return ERR_OK;
}

void DevicestatusMsdpRdb::InitTimer()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    epFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epFd_ == -1) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "create epoll fd fail.");
    }
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "create timer fd fail.");
    }
    SetTimerInterval(TIMER_INTERVAL);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbacks_.insert(std::make_pair(timerFd_, &DevicestatusMsdpRdb::TimerCallback));
    if (RegisterTimerCallback(timerFd_, EVENT_TIMER_FD)) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "register timer fd fail.");
    }
}

void DevicestatusMsdpRdb::SetTimerInterval(int interval)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    struct itimerspec itval;

    if (timerFd_ == ERR_INVALID_FD) {
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
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "set timer failed");
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    return;
}

void DevicestatusMsdpRdb::CloseTimer()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    close(timerFd_);
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

void DevicestatusMsdpRdb::TimerCallback()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    unsigned long long timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "read timer fd fail.");
        return;
    }
    TrigerDatabaseObserver();
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

int DevicestatusMsdpRdb::RegisterTimerCallback(const int fd, const EventType et)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }

    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "epoll_ctl failed, error num =%{public}d", errno);
        return -1;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return 0;
}

void DevicestatusMsdpRdb::StartThread()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    std::make_unique<std::thread>(&DevicestatusMsdpRdb::LoopingThreadEntry, this)->detach();
}

void DevicestatusMsdpRdb::LoopingThreadEntry()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    int32_t nevents = 0;
    size_t cbct = callbacks_.size();
    struct epoll_event events[cbct];

    while (true) {
        int timeout = 0;

        nevents = epoll_wait(epFd_, events, cbct, timeout);
        if (nevents == -1) {
            continue;
        }
        for (int n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                DevicestatusMsdpRdb *func = const_cast<DevicestatusMsdpRdb *>(this);
                (callbacks_.find(events[n].data.fd)->second)(func);
            }
        }
    }
}

int InsertOpenCallback::OnCreate(RdbStore &store)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

int InsertOpenCallback::OnUpgrade(RdbStore &store, int oldVersion, int newVersion)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

extern "C" DevicestatusMsdpInterface *Create(void)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    rdb = new DevicestatusMsdpRdb();
    return rdb;
}

extern "C" void Destroy(DevicestatusMsdpInterface* algorithm)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Enter");
    delete algorithm;
}
}
}
