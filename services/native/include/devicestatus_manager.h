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

#ifndef DEVICESTATUS_MANAGER_H
#define DEVICESTATUS_MANAGER_H

#include <set>
#include <map>

#include "sensor_if.h"
#include "devicestatus_data_utils.h"
#include "idevicestatus_algorithm.h"
#include "idevicestatus_callback.h"
#include "devicestatus_common.h"
#include "devicestatus_msdp_client_impl.h"
#include "accesstoken_kit.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace Security::AccessToken;
class DeviceStatusService;
class DeviceStatusManager {
public:
    explicit DeviceStatusManager(const wptr<DeviceStatusService>& ms) : ms_(ms)
    {
        DEV_HILOGI(SERVICE, "DeviceStatusManager instance is created.");
    }
    ~DeviceStatusManager() = default;

    class DeviceStatusCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DeviceStatusCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);
        virtual ~DeviceStatusCallbackDeathRecipient() = default;
    };

    bool Init();
    bool EnableMock(DeviceStatusDataUtils::DeviceStatusType type);
    bool InitInterface(DeviceStatusDataUtils::DeviceStatusType type);
    bool DisableMock(DeviceStatusDataUtils::DeviceStatusType type);
    bool InitDataCallback();
    void NotifyDeviceStatusChange(const DeviceStatusDataUtils::DeviceStatusData& devicestatusData);
    void Subscribe(const DeviceStatusDataUtils::DeviceStatusType& type, const sptr<IdevicestatusCallback>& callback);
    void Unsubscribe(const DeviceStatusDataUtils::DeviceStatusType& type, const sptr<IdevicestatusCallback>& callback);
    DeviceStatusDataUtils::DeviceStatusData GetLatestDeviceStatusData(const \
        DeviceStatusDataUtils::DeviceStatusType& type);
    int32_t MsdpDataCallback(const DeviceStatusDataUtils::DeviceStatusData& data);
    int32_t LoadAlgorithm(bool bCreate);
    int32_t UnloadAlgorithm(bool bCreate);
    void GetPackageName(AccessTokenID tokenId, std::string &packageName);

private:
    struct classcomp {
        bool operator()(const sptr<IdevicestatusCallback> &l, const sptr<IdevicestatusCallback> &r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    const wptr<DeviceStatusService> ms_;
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> devicestatusCBDeathRecipient_;
    std::unique_ptr<DeviceStatusMsdpClientImpl> msdpImpl_;
    std::map<DeviceStatusDataUtils::DeviceStatusType, DeviceStatusDataUtils::DeviceStatusValue> msdpData_;
    std::map<DeviceStatusDataUtils::DeviceStatusType, std::set<const sptr<IdevicestatusCallback>, classcomp>> \
        listenerMap_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_MANAGER_H
