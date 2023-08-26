/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef DM_BINDING_INTERNAL_H
#define DM_BINDING_INTERNAL_H

#include "dm_device_info.h"

#include "dm_binding.h"

struct CDmDeviceInfo {
    char* deviceId;
    char* deviceName;
    uint16_t deviceTypeId;
    char* networkId;
    int32_t range;
    CDmAuthForm authForm;
};

struct CRegisterDevStateCallback {
    void (*onDeviceOnline)(const CDmDeviceInfo* deviceInfo);
    void (*onDeviceChanged)(const CDmDeviceInfo* deviceInfo);
    void (*onDeviceReady)(const CDmDeviceInfo* deviceInfo);
    void (*onDeviceOffline)(const CDmDeviceInfo* deviceInfo);
};

class DeviceInitCallBack : public OHOS::DistributedHardware::DmInitCallback {
public:
    explicit DeviceInitCallBack(void (*cb)());

    void OnRemoteDied() override;
private:
    void (*callback_)();
};

class DmDeviceStateCallback : public OHOS::DistributedHardware::DeviceStateCallback {
public:
    explicit DmDeviceStateCallback(CRegisterDevStateCallback callbacks);

private:
    void OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceReady(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;
    void OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo) override;

private:
    void InitCallback(CRegisterDevStateCallback callbacks);
    CDmDeviceInfo* CreateCDeviceInfo(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo);

    void (*onlineCallback_)(const CDmDeviceInfo*);
    void (*changedCallback_)(const CDmDeviceInfo*);
    void (*readyCallback_)(const CDmDeviceInfo*);
    void (*offlineCallback_)(const CDmDeviceInfo*);
};

#endif // DM_BINDING_INTERNAL_H