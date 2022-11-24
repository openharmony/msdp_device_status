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

#ifndef DEVICESTATUS_SERVICE_H
#define DEVICESTATUS_SERVICE_H

#include <memory>
#include <iremote_object.h>
#include <system_ability.h>

#include "devicestatus_srv_stub.h"
#include "idevicestatus_callback.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_dumper.h"
#include "devicestatus_manager.h"
#include "devicestatus_delayed_sp_singleton.h"

namespace OHOS {
namespace Msdp {
class DevicestatusService final : public SystemAbility, public DevicestatusSrvStub {
    DECLARE_SYSTEM_ABILITY(DevicestatusService)
    DECLARE_DELAYED_SP_SINGLETON(DevicestatusService);
public:
    virtual void OnDump() override;
    virtual void OnStart() override;
    virtual void OnStop() override;

    void Subscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    void UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type, \
        const sptr<IdevicestatusCallback>& callback) override;
    DevicestatusDataUtils::DevicestatusData GetCache(const DevicestatusDataUtils::DevicestatusType& type) override;
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void ReportMsdpSysEvent(const DevicestatusDataUtils::DevicestatusType& type, bool enable);
private:
    bool Init();
    bool ready_ = false;
    std::shared_ptr<DevicestatusManager> devicestatusManager_;
    std::shared_ptr<DevicestatusMsdpClientImpl> msdpImpl_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_SERVICE_H
