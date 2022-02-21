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

#ifndef DEVICESTATUS_MSDP_RDB_H
#define DEVICESTATUS_MSDP_RDB_H

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <errors.h>
#include "rdb_store.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "values_bucket.h"
#include "result_set.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_msdp_interface.h"

namespace OHOS {
namespace Msdp {
class DevicestatusMsdpRdb : public DevicestatusMsdpInterface {
public:
    enum EventType {
        EVENT_UEVENT_FD,
        EVENT_TIMER_FD,
    };

    DevicestatusMsdpRdb() {}
    virtual ~DevicestatusMsdpRdb() {}
    bool Init();
    void InitRdbStore();
    void SetTimerInterval(int interval);
    void CloseTimer();
    void InitTimer();
    void TimerCallback();
    int RegisterTimerCallback(const int fd, const EventType et);
    void StartThread();
    void LoopingThreadEntry();
    ErrCode Enable() override;
    ErrCode Disable() override;
    ErrCode RegisterCallback(std::shared_ptr<MsdpAlgorithmCallback>& callback) override;
    ErrCode UnregisterCallback() override;
    ErrCode NotifyMsdpImpl(DevicestatusDataUtils::DevicestatusData& data);
    int TrigerData(std::unique_ptr<NativeRdb::ResultSet> &resultSet);
    int TrigerDatabaseObserver();
    DevicestatusDataUtils::DevicestatusData SaveRdbData(DevicestatusDataUtils::DevicestatusData& data);
    std::shared_ptr<MsdpAlgorithmCallback> GetCallbacksImpl()
    {
        std::unique_lock lock(mutex_);
        return callbacksImpl_;
    }

private:
    using Callback = std::function<void(DevicestatusMsdpRdb*)>;
    std::shared_ptr<MsdpAlgorithmCallback> callbacksImpl_;
    std::map<int32_t, Callback> callbacks_;
    std::shared_ptr<NativeRdb::RdbStore> store_;
    int devicestatusType_ = -1;
    int devicestatusStatus_ = -1;
    bool notifyFlag_ = false;
    int timerInterval_ = -1;
    int32_t timerFd_ = -1;
    int32_t epFd_ = -1;
    std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> rdbDataMap_;
    std::mutex mutex_;
};

class InsertOpenCallback : public NativeRdb::RdbOpenCallback {
public:
    int OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int OnUpgrade(NativeRdb::RdbStore &rdbStore, int oldVersion, int newVersion) override;
};
}
}
#endif // DEVICESTATUS_MSDP_RDB_H
