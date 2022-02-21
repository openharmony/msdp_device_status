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

#include "devicestatus_sensor_rdb.h"
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
const int NUM_HALL_SENSOR = 100000000;
std::unique_ptr<DevicestatusSensorRdb> msdpRdb_ = std::make_unique<DevicestatusSensorRdb>();
const int32_t ERR_NG = -1;
DevicestatusSensorRdb* rdb;
SensorUser user;
}
static void OnReceivedSensorEvent(SensorEvent *event)
{
    if (event == nullptr) return;

    rdb->HandleHallSensorEvent(event);
}

bool DevicestatusSensorRdb::Init()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusSensorRdbInit: enter");
    InitRdbStore();

    InitTimer();
    StartThread();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "DevicestatusSensorRdbInit: exit");
    return true;
}

void DevicestatusSensorRdb::InitRdbStore()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int errCode = ERR_OK;
    RdbStoreConfig config(DATABASE_NAME);
    HelperCallback helper;
    store_ = RdbHelper::GetRdbStore(config, 1, helper, errCode);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

ErrCode DevicestatusSensorRdb::RegisterCallback(std::shared_ptr<DevicestatusSensorHdiCallback>& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    callbacksImpl_ = callback;
    return ERR_OK;
}

ErrCode DevicestatusSensorRdb::UnregisterCallback()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    return ERR_OK;
}

ErrCode DevicestatusSensorRdb::Enable()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    Init();
    SubscribeHallSensor();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusSensorRdb::Disable()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    UnSubscribeHallSensor();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}


ErrCode DevicestatusSensorRdb::NotifyMsdpImpl(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (rdb->GetCallbacksImpl() == nullptr) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "callbacksImpl is nullptr");
        return ERR_NG;
    }
    rdb->GetCallbacksImpl()->OnSensorHdiResult(data);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusSensorRdb::SaveRdbData(
    DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

int DevicestatusSensorRdb::TrigerData(std::unique_ptr<NativeRdb::ResultSet> &resultSet)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

    return ERR_OK;
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

int DevicestatusSensorRdb::TrigerDatabaseObserver()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    std::string str;

    if (store_ == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "store_ is not exist");
        sleep(READ_RDB_WAIT_TIME);
        InitRdbStore();
        return -1;
    }

    std::unique_ptr<ResultSet> resultSet =
        store_->QuerySql("SELECT * FROM DEVICESTATUSSENSOR WHERE ID = (SELECT max(ID) from DEVICESTATUSSENSOR)");

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
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "notifyFlag_ is %{public}d", notifyFlag_);
    if (notifyFlag_) {
        NotifyMsdpImpl(data);
        notifyFlag_ = false;
    }

    return ERR_OK;
}


void DevicestatusSensorRdb::HandleHallSensorEvent(SensorEvent *event)
{
    if (event == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "HandleHallSensorEvent event is null");
        return;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE,
        "HandleHallSensorEvent sensorTypeId: %{public}d, version: %{public}d, mode: %{public}d\n",
        event[0].sensorTypeId, event[0].version, event[0].mode);

    DevicestatusDataUtils::DevicestatusData data;

    if (event[0].sensorTypeId == SENSOR_TYPE_ID_NONE) {
        AccelData *sensorData = (AccelData *)event[0].data;
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE,
            "HandleHallSensorEvent sensor_data: %{public}f\n", sensorData->axisX);

        data.type = DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN;
        data.value = DevicestatusDataUtils::DevicestatusValue(event[0].mode);
    } else if (event[0].sensorTypeId == SENSOR_TYPE_ID_HALL) {
        HallData* sensor_data = (HallData *)event[0].data;
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE,
            "HandleHallSensorEvent sensor_data: %{public}d\n", sensor_data->scalar);

        data.type = DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN;
        data.value = DevicestatusDataUtils::DevicestatusValue(sensor_data->scalar);
    }

    NotifyMsdpImpl(data);
}

void DevicestatusSensorRdb::SubscribeHallSensor()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int32_t sensorTypeId = 0;
    user.callback = OnReceivedSensorEvent;

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "SubcribeHallSensor");
    int32_t ret = SubscribeSensor(sensorTypeId, &user);
    ret = SetBatch(sensorTypeId, &user, NUM_HALL_SENSOR, NUM_HALL_SENSOR);
    ret = ActivateSensor(sensorTypeId, &user);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

void DevicestatusSensorRdb::UnSubscribeHallSensor()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int32_t sensorTypeId = 0;

    user.callback = OnReceivedSensorEvent;

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "UnsubcribeHallSensor");
    int32_t ret = DeactivateSensor(sensorTypeId, &user);
    ret = UnsubscribeSensor(sensorTypeId, &user);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "ret = %{public}d", ret);

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
}

void DevicestatusSensorRdb::InitTimer()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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
    callbacks_.insert(std::make_pair(timerFd_, &DevicestatusSensorRdb::TimerCallback));
    if (RegisterTimerCallback(timerFd_, EVENT_TIMER_FD)) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "register timer fd fail.");
    }
}

void DevicestatusSensorRdb::SetTimerInterval(int interval)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    return;
}

void DevicestatusSensorRdb::TimerCallback()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    unsigned long long timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "read timer fd fail.");
        return;
    }
    TrigerDatabaseObserver();
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

int DevicestatusSensorRdb::RegisterTimerCallback(const int fd, const EventType et)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return 0;
}

void DevicestatusSensorRdb::StartThread()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    std::make_unique<std::thread>(&DevicestatusSensorRdb::LoopingThreadEntry, this)->detach();
}

void DevicestatusSensorRdb::LoopingThreadEntry()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int nevents = 0;
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
                DevicestatusSensorRdb *func = const_cast<DevicestatusSensorRdb *>(this);
                (callbacks_.find(events[n].data.fd)->second)(func);
            }
        }
    }
}

int HelperCallback::OnCreate(RdbStore &store)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

int HelperCallback::OnUpgrade(RdbStore &store, int oldVersion, int newVersion)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Exit");
    return ERR_OK;
}

extern "C" DevicestatusSensorInterface *Create(void)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    rdb = new DevicestatusSensorRdb();
    return rdb;
}

extern "C" void Destroy(DevicestatusSensorInterface* algorithm)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    delete algorithm;
}
}
}
