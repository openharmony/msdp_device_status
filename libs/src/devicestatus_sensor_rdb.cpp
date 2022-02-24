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
constexpr int32_t TIMER_INTERVAL = 3;
constexpr int32_t ERR_INVALID_FD = -1;
constexpr int32_t READ_RDB_WAIT_TIME = 30;
constexpr int32_t NUM_HALL_SENSOR = 100000000;
std::unique_ptr<DevicestatusSensorRdb> g_msdpRdb = std::make_unique<DevicestatusSensorRdb>();
constexpr int32_t ERR_NG = -1;
DevicestatusSensorRdb* g_rdb;
SensorUser user;
}
static void OnReceivedSensorEvent(SensorEvent *event)
{
    if (event == nullptr) {
        DEV_HILOGE(SERVICE, "event is nullptr");
    }
    g_rdb->HandleHallSensorEvent(event);
}

bool DevicestatusSensorRdb::Init()
{
    DEV_HILOGI(SERVICE, "Enter");
    InitRdbStore();
    InitTimer();
    StartThread();
    DEV_HILOGI(SERVICE, "Exit");
    return true;
}

void DevicestatusSensorRdb::InitRdbStore()
{
    DEV_HILOGI(SERVICE, "Enter");
    int32_t errCode = ERR_OK;
    RdbStoreConfig config(DATABASE_NAME);
    HelperCallback helper;
    store_ = RdbHelper::GetRdbStore(config, 1, helper, errCode);
    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusSensorRdb::RegisterCallback(const std::shared_ptr<DevicestatusSensorHdiCallback>& callback)
{
    callbacksImpl_ = callback;
}

void DevicestatusSensorRdb::UnregisterCallback()
{
    callbacksImpl_ = nullptr;
}

void DevicestatusSensorRdb::Enable()
{
    DEV_HILOGI(SERVICE, "Enter");
    Init();
    SubscribeHallSensor();
    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusSensorRdb::Disable()
{
    DEV_HILOGI(SERVICE, "Enter");
    CloseTimer();
    UnSubscribeHallSensor();
    DEV_HILOGI(SERVICE, "Exit");
}


ErrCode DevicestatusSensorRdb::NotifyMsdpImpl(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_rdb->GetCallbacksImpl() == nullptr) {
        DEV_HILOGI(SERVICE, "callbacksImpl is nullptr");
        return ERR_NG;
    }
    g_rdb->GetCallbacksImpl()->OnSensorHdiResult(data);

    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusSensorRdb::SaveRdbData(
    const DevicestatusDataUtils::DevicestatusData& data)
{
    for (auto iter = rdbDataMap_.begin(); iter != rdbDataMap_.end(); ++iter) {
        if (iter->first == data.type) {
            if (iter->second != data.value) {
                notifyFlag_ = true;
                iter->second = data.value;
            }
            DEV_HILOGI(SERVICE, "data is not changed");
            return data;
        }
    }

    rdbDataMap_.insert(std::make_pair(data.type, data.value));
    notifyFlag_ = true;

    DEV_HILOGI(SERVICE, "devicestatusType_ = %{public}d, devicestatusStatus_ = %{public}d",
        devicestatusType_, devicestatusStatus_);

    return data;
}

int32_t DevicestatusSensorRdb::TrigerData(const std::unique_ptr<NativeRdb::ResultSet> &resultSet)
{
    int32_t columnIndex;
    int32_t intVal;

    int32_t ret = resultSet->GetColumnIndex("ID", columnIndex);
    DEV_HILOGI(SERVICE, "TrigerDatabaseObserver GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckID: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGI(SERVICE, "ret = %{public}d, id = %{public}d", ret, intVal);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckID: GetValue failed");
        return -1;
    }

    ret = resultSet->GetColumnIndex("DEVICESTATUS_TYPE", columnIndex);
    DEV_HILOGI(SERVICE, "DEVICESTATUS_TYPE GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusType: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGI(SERVICE, "ret = %{public}d, DevicestatusType = %{public}d", ret, intVal);
    devicestatusType_ = intVal;
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusType: GetValue failed");
        return -1;
    }

    ret = resultSet->GetColumnIndex("DEVICESTATUS_STATUS", columnIndex);
    DEV_HILOGI(SERVICE, "DEVICESTATUS_STATUS GetColumnIndex = %{public}d", columnIndex);
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusStatus: GetColumnIndex failed");
        return -1;
    }
    ret = resultSet->GetInt(columnIndex, intVal);
    DEV_HILOGI(SERVICE, "ret = %{public}d, DevicestatusStatus = %{public}d", ret, intVal);
    devicestatusStatus_ = intVal;
    if (ret != ERR_OK) {
        DEV_HILOGE(SERVICE, "CheckDevicestatusStatus: GetValue failed");
        return -1;
    }

    return ERR_OK;
}

int32_t DevicestatusSensorRdb::TrigerDatabaseObserver()
{
    DEV_HILOGI(SERVICE, "Enter");

    if (store_ == nullptr) {
        DEV_HILOGE(SERVICE, "store_ is not exist");
        sleep(READ_RDB_WAIT_TIME);
        InitRdbStore();
        DEV_HILOGI(SERVICE, "init rdb store again succes");
        return -1;
    }

    std::unique_ptr<ResultSet> resultSet =
        store_->QuerySql("SELECT * FROM DEVICESTATUSSENSOR WHERE ID = (SELECT max(ID) from DEVICESTATUSSENSOR)");

    if (resultSet == nullptr) {
        DEV_HILOGE(SERVICE, "database is not exist");
        return -1;
    }

    int32_t ret = resultSet->GoToFirstRow();
    DEV_HILOGI(SERVICE, "GoToFirstRow = %{public}d", ret);
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
    DEV_HILOGI(SERVICE, "notifyFlag_ is %{public}d", notifyFlag_);
    if (notifyFlag_) {
        NotifyMsdpImpl(data);
        notifyFlag_ = false;
    }

    return ERR_OK;
}

void DevicestatusSensorRdb::HandleHallSensorEvent(SensorEvent *event)
{
    if (event == nullptr) {
        DEV_HILOGE(SERVICE, "HandleHallSensorEvent event is null");
        return;
    }

    DEV_HILOGI(SERVICE, "HandleHallSensorEvent sensorTypeId: %{public}d, version: %{public}d, mode: %{public}d\n",
        event[0].sensorTypeId, event[0].version, event[0].mode);

    DevicestatusDataUtils::DevicestatusData data;

    if (event[0].sensorTypeId == SENSOR_TYPE_ID_NONE) {
        AccelData *sensorData = (AccelData *)event[0].data;
        DEV_HILOGI(SERVICE, "HandleHallSensorEvent sensor_data: %{public}f\n", sensorData->axisX);

        data.type = DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN;
        data.value = DevicestatusDataUtils::DevicestatusValue(event[0].mode);
    } else if (event[0].sensorTypeId == SENSOR_TYPE_ID_HALL) {
        HallData* sensor_data = (HallData *)event[0].data;
        DEV_HILOGI(SERVICE, "HandleHallSensorEvent sensor_data: %{public}d\n", sensor_data->scalar);

        data.type = DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN;
        data.value = DevicestatusDataUtils::DevicestatusValue(sensor_data->scalar);
    }

    NotifyMsdpImpl(data);
}

void DevicestatusSensorRdb::SubscribeHallSensor()
{
    DEV_HILOGI(SERVICE, "Enter");
    int32_t sensorTypeId = 0;
    user.callback = OnReceivedSensorEvent;

    DEV_HILOGI(SERVICE, "SubcribeHallSensor");
    SubscribeSensor(sensorTypeId, &user);
    SetBatch(sensorTypeId, &user, NUM_HALL_SENSOR, NUM_HALL_SENSOR);
    ActivateSensor(sensorTypeId, &user);

    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusSensorRdb::UnSubscribeHallSensor()
{
    DEV_HILOGI(SERVICE, "Enter");
    int32_t sensorTypeId = 0;

    user.callback = OnReceivedSensorEvent;

    DEV_HILOGI(SERVICE, "UnsubcribeHallSensor");
    DeactivateSensor(sensorTypeId, &user);
    UnsubscribeSensor(sensorTypeId, &user);

    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusSensorRdb::InitTimer()
{
    DEV_HILOGI(SERVICE, "Enter");
    epFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epFd_ == -1) {
        DEV_HILOGI(SERVICE, "create epoll fd failed");
        return;
    }
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGI(SERVICE, "create timer fd failed");
        return;
    }
    SetTimerInterval(TIMER_INTERVAL);
    fcntl(timerFd_, F_SETFL, O_NONBLOCK);
    callbacks_.insert(std::make_pair(timerFd_, &DevicestatusSensorRdb::TimerCallback));
    if (RegisterTimerCallback(timerFd_, EVENT_TIMER_FD)) {
        DEV_HILOGI(SERVICE, "register timer fd failed");
        return;
    }
}

void DevicestatusSensorRdb::SetTimerInterval(int32_t interval)
{
    struct itimerspec itval;

    if (timerFd_ == ERR_INVALID_FD) {
        DEV_HILOGE(SERVICE, "create timer fd failed");
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
        DEV_HILOGI(SERVICE, "set timer failed");
        return;
    }

    return;
}

void DevicestatusSensorRdb::CloseTimer()
{
    DEV_HILOGI(SERVICE, "Enter");
    close(timerFd_);
    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusSensorRdb::TimerCallback()
{
    unsigned long long timers;
    if (read(timerFd_, &timers, sizeof(timers)) == -1) {
        DEV_HILOGI(SERVICE, "read timer fd failed");
        return;
    }
    TrigerDatabaseObserver();
}

int32_t DevicestatusSensorRdb::RegisterTimerCallback(const int32_t fd, const EventType et)
{
    DEV_HILOGI(SERVICE, "Enter");
    struct epoll_event ev;

    ev.events = EPOLLIN;
    if (et == EVENT_TIMER_FD) {
        ev.events |= EPOLLWAKEUP;
    }

    ev.data.ptr = reinterpret_cast<void*>(this);
    ev.data.fd = fd;
    if (epoll_ctl(epFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        DEV_HILOGI(SERVICE, "epoll_ctl failed, error num =%{public}d", errno);
        return -1;
    }

    return 0;
}

void DevicestatusSensorRdb::StartThread()
{
    DEV_HILOGI(SERVICE, "Enter");
    std::make_unique<std::thread>(&DevicestatusSensorRdb::LoopingThreadEntry, this)->detach();
}

void DevicestatusSensorRdb::LoopingThreadEntry()
{
    size_t cbct = callbacks_.size();
    struct epoll_event events[cbct];

    while (true) {
        int32_t timeout = 0;

        int32_t nevents = epoll_wait(epFd_, events, cbct, timeout);
        if (nevents == -1) {
            continue;
        }
        for (int32_t n = 0; n < nevents; ++n) {
            if (events[n].data.ptr) {
                DevicestatusSensorRdb *func = const_cast<DevicestatusSensorRdb *>(this);
                (callbacks_.find(events[n].data.fd)->second)(func);
            }
        }
    }
}

int32_t HelperCallback::OnCreate(RdbStore &store)
{
    DEV_HILOGI(SERVICE, "Enter");
    return ERR_OK;
}

int32_t HelperCallback::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    DEV_HILOGI(SERVICE, "Enter");
    return ERR_OK;
}

extern "C" DevicestatusSensorInterface *Create(void)
{
    DEV_HILOGI(SERVICE, "Enter");
    g_rdb = new DevicestatusSensorRdb();
    return g_rdb;
}

extern "C" void Destroy(DevicestatusSensorInterface* algorithm)
{
    DEV_HILOGI(SERVICE, "Enter");
    delete algorithm;
}
}
}
