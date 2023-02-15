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

#ifndef DEVICESTATUS_MSDP_CLIENT_IMPL_H
#define DEVICESTATUS_MSDP_CLIENT_IMPL_H

#include <string>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <map>
#include <errors.h>

#include "rdb_store.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "values_bucket.h"
#include "result_set.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_delayed_sp_singleton.h"
#include "devicestatus_dumper.h"
#include "devicestatus_msdp_interface.h"

namespace OHOS {
namespace Msdp {
class DevicestatusMsdpClientImpl : public DevicestatusMsdpInterface::MsdpAlgorithmCallback {
public:
    using CallbackManager = std::function<int32_t(const DevicestatusDataUtils::DevicestatusData&)>;

    ErrCode InitMsdpImpl(DevicestatusDataUtils::DevicestatusType type);
    ErrCode DisableMsdpImpl(DevicestatusDataUtils::DevicestatusType type);
    ErrCode RegisterImpl(const CallbackManager& callback);
    ErrCode UnregisterImpl();
    std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> GetObserverData() const;
    int32_t LoadAlgorithmLibrary();
    int32_t UnloadAlgorithmLibrary();
    int32_t LoadAlgoLib();
    int32_t UnloadAlgoLib();
private:
    using CreateFunc = DevicestatusMsdpInterface* (*)();
    using DestroyFunc = void *(*)(DevicestatusMsdpInterface*);

    ErrCode ImplCallback(const DevicestatusDataUtils::DevicestatusData& data);
    int32_t MsdpCallback(const DevicestatusDataUtils::DevicestatusData& data);
    ErrCode RegisterMsdp();
    ErrCode UnregisterMsdp(void);
    DevicestatusDataUtils::DevicestatusData SaveObserverData(const DevicestatusDataUtils::DevicestatusData& data);
    DevicestatusMsdpInterface* GetAlgorithmInst();
    DevicestatusMsdpInterface* GetAlgoObj();
    MsdpAlgorithmHandle mAlgorithm_;
    MsdpAlgorithmHandle algoHandler_;
    std::mutex mutex_;
    bool notifyManagerFlag_ = false;
    void OnResult(const DevicestatusDataUtils::DevicestatusData& data) override;
};
} // Msdp
} // OHOS
#endif // DEVICESTATUS_MSDP_CLIENT_IMPL_H
