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

#ifndef DEVICESTATUS_MANAGER_H
#define DEVICESTATUS_MANAGER_H

#include <set>
#include <map>

#include "sensor_if.h"
#include "devicestatus_data_utils.h"
#include "death_recipient.h"
#include "idevicestatus_algorithm.h"
#include "idevicestatus_callback.h"
#include "devicestatus_common.h"
#include "devicestatus_msdp_client_impl.h"
#include "accesstoken_kit.h"

namespace OHOS {
namespace Msdp {
using namespace Security::AccessToken;
class DevicestatusService;
class DevicestatusManager {
public:
    explicit DevicestatusManager(const wptr<DevicestatusService>& ms) : ms_(ms)
    {
        DEV_HILOGI(SERVICE, "DevicestatusManager instance is created.");
    }
    ~DevicestatusManager() = default;

    class DevicestatusCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DevicestatusCallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);
        virtual ~DevicestatusCallbackDeathRecipient() = default;
    };

    bool Init();
    bool EnableMock(DevicestatusDataUtils::DevicestatusType type);
    bool InitInterface(DevicestatusDataUtils::DevicestatusType type);
    bool DisableMock(DevicestatusDataUtils::DevicestatusType type);
    bool InitDataCallback();
    void NotifyDevicestatusChange(const DevicestatusDataUtils::DevicestatusData& devicestatusData);
    void Subscribe(const DevicestatusDataUtils::DevicestatusType& type, const sptr<IdevicestatusCallback>& callback);
    void UnSubscribe(const DevicestatusDataUtils::DevicestatusType& type, const sptr<IdevicestatusCallback>& callback);
    DevicestatusDataUtils::DevicestatusData GetLatestDevicestatusData(const \
        DevicestatusDataUtils::DevicestatusType& type);
    int32_t MsdpDataCallback(const DevicestatusDataUtils::DevicestatusData& data);
    int32_t LoadAlgorithm();
    int32_t UnloadAlgorithm();
    void GetPackageName(AccessTokenID tokenId, std::string &packageName);
    void ProcessDeathObserver(wptr<IRemoteObject> object);
private:
    struct classcomp {
        bool operator()(const sptr<IdevicestatusCallback> &l, const sptr<IdevicestatusCallback> &r) const
        {
            return l->AsObject() < r->AsObject();
        }
    };
    const wptr<DevicestatusService> ms_;
    std::mutex mutex_;
    sptr<IRemoteObject::DeathRecipient> devicestatusCBDeathRecipient_;
    std::unique_ptr<DevicestatusMsdpClientImpl> msdpImpl_;
    std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> msdpData_;
    sptr<IRemoteObject::DeathRecipient> clientDeathObserver_ = nullptr;
    std::map<DevicestatusDataUtils::DevicestatusType, std::set<const sptr<IdevicestatusCallback>, classcomp>> \
        listenerMap_;
};
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_MANAGER_H
