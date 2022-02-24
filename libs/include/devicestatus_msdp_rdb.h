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
    void SetTimerInterval(int32_t interval);
    void CloseTimer();
    void InitTimer();
    void TimerCallback();
    int32_t RegisterTimerCallback(const int32_t fd, const EventType et);
    void StartThread();
    void LoopingThreadEntry();
    void Enable() override;
    void Disable() override;
    void RegisterCallback(const std::shared_ptr<MsdpAlgorithmCallback>& callback) override;
    void UnregisterCallback() override;
    ErrCode NotifyMsdpImpl(const DevicestatusDataUtils::DevicestatusData& data);
    int32_t TrigerData(const std::unique_ptr<NativeRdb::ResultSet> &resultSet);
    int32_t TrigerDatabaseObserver();
    DevicestatusDataUtils::DevicestatusData SaveRdbData(const DevicestatusDataUtils::DevicestatusData& data);
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
    int32_t devicestatusType_ = -1;
    int32_t devicestatusStatus_ = -1;
    bool notifyFlag_ = false;
    int32_t timerInterval_ = -1;
    int32_t timerFd_ = -1;
    int32_t epFd_ = -1;
    std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> rdbDataMap_;
    std::mutex mutex_;
};

class InsertOpenCallback : public NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(NativeRdb::RdbStore &rdbStore) override;
    int32_t OnUpgrade(NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;
};
}
}
#endif // DEVICESTATUS_MSDP_RDB_H
