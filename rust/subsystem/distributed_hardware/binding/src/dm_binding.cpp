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

#include "dm_binding.h"

#include <string>

#include "device_manager.h"
#include "dm_device_info.h"

#include "devicestatus_define.h"
#include "dm_binding_internal.h"

#undef LOG_TAG
#define LOG_TAG "DmBinding"

namespace {
#define DIS_HARDWARE OHOS::DistributedHardware::DeviceManager::GetInstance()
} // namespace

DeviceInitCallBack::DeviceInitCallBack(void (*cb)()) : callback_(cb)
{
}

void DeviceInitCallBack::OnRemoteDied()
{
    CALL_DEBUG_ENTER;
    CHKPV(callback_);
    callback_();
}

DmDeviceStateCallback::DmDeviceStateCallback(CRegisterDevStateCallback callbacks)
{
    CALL_DEBUG_ENTER;
    InitCallback(callbacks);
}

void DmDeviceStateCallback::OnDeviceOnline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CHKPV(onlineCallback_);
    CDmDeviceInfo* cDeviceInfo = CreateCDeviceInfo(deviceInfo);
    CHKPV(cDeviceInfo);
    onlineCallback_(cDeviceInfo);
}

void DmDeviceStateCallback::OnDeviceChanged(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CHKPV(changedCallback_);
    CDmDeviceInfo* cDeviceInfo = CreateCDeviceInfo(deviceInfo);
    CHKPV(cDeviceInfo);
    changedCallback_(cDeviceInfo);
}

void DmDeviceStateCallback::OnDeviceReady(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CHKPV(readyCallback_);
    CDmDeviceInfo* cDeviceInfo = CreateCDeviceInfo(deviceInfo);
    CHKPV(cDeviceInfo);
    readyCallback_(cDeviceInfo);
}

void DmDeviceStateCallback::OnDeviceOffline(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CHKPV(offlineCallback_);
    CDmDeviceInfo* cDeviceInfo = CreateCDeviceInfo(deviceInfo);
    CHKPV(cDeviceInfo);
    offlineCallback_(cDeviceInfo);
}

void DmDeviceStateCallback::InitCallback(CRegisterDevStateCallback callbacks)
{
    CALL_DEBUG_ENTER;
    CHKPV(callbacks.onDeviceOnline);
    CHKPV(callbacks.onDeviceChanged);
    CHKPV(callbacks.onDeviceReady);
    CHKPV(callbacks.onDeviceOffline);
    onlineCallback_ = callbacks.onDeviceOnline;
    changedCallback_ = callbacks.onDeviceChanged;
    readyCallback_ = callbacks.onDeviceReady;
    offlineCallback_ = callbacks.onDeviceOffline;
}

CDmDeviceInfo* DmDeviceStateCallback::CreateCDeviceInfo(const OHOS::DistributedHardware::DmDeviceInfo &deviceInfo)
{
    CALL_DEBUG_ENTER;
    CDmDeviceInfo* cDeviceInfo = new (std::nothrow) CDmDeviceInfo;
    cDeviceInfo->deviceId = new (std::nothrow) char[sizeof(deviceInfo.deviceId)];
    if (strcpy_s(cDeviceInfo->deviceId, sizeof(deviceInfo.deviceId), deviceInfo.deviceId) != EOK) {
        FI_HILOGE("Invalid deviceId:\'%{public}s\'", GetAnonyString(deviceInfo.deviceId).c_str());
        return nullptr;
    }
    cDeviceInfo->deviceName = new (std::nothrow) char[sizeof(deviceInfo.deviceName)];
    if (strcpy_s(cDeviceInfo->deviceName, sizeof(deviceInfo.deviceName), deviceInfo.deviceName) != EOK) {
        FI_HILOGE("Invalid deviceName");
        return nullptr;
    }
    cDeviceInfo->deviceTypeId = deviceInfo.deviceTypeId;
    cDeviceInfo->networkId = new (std::nothrow) char[sizeof(deviceInfo.networkId)];
    if (strcpy_s(cDeviceInfo->networkId, sizeof(deviceInfo.networkId), deviceInfo.networkId) != EOK) {
        FI_HILOGE("Invalid networkId");
        return nullptr;
    }
    cDeviceInfo->range = deviceInfo.range;
    cDeviceInfo->authForm = static_cast<CDmAuthForm>(deviceInfo.authForm);
    return cDeviceInfo;
}

bool CInitDeviceManager(const char* pkgName, void (*callback)())
{
    CALL_DEBUG_ENTER;
    CHKPF(callback);
    auto initCallback = std::make_shared<DeviceInitCallBack>(callback);
    std::string sPkgName(pkgName);
    int32_t ret = DIS_HARDWARE.InitDeviceManager(sPkgName, initCallback);
    if (ret != 0) {
        FI_HILOGE("Init device manager failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool CRegisterDevState(const char* pkgName, const char* extra, CRegisterDevStateCallback callbacks)
{
    CALL_DEBUG_ENTER;
    auto stateCallback = std::make_shared<DmDeviceStateCallback>(callbacks);
    std::string sPkgName(pkgName);
    std::string sExtra(extra);
    int32_t ret = DIS_HARDWARE.RegisterDevStateCallback(sPkgName, sExtra, stateCallback);
    if (ret != 0) {
        FI_HILOGE("Register devStateCallback failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

bool CUnRegisterDevState(const char* pkgName, const char* extra)
{
    CALL_DEBUG_ENTER;
    std::string sPkgName(pkgName);
    std::string sExtra(extra);
    int32_t ret = DIS_HARDWARE.UnRegisterDevStateCallback(sPkgName, sExtra);
    if (ret != 0) {
        FI_HILOGE("UnRegister devStateCallback failed, ret:%{public}d", ret);
        return false;
    }
    return true;
}

void CDestroyDmDeviceInfo(CDmDeviceInfo* deviceInfo)
{
    CALL_DEBUG_ENTER;
    CHKPV(deviceInfo);
    deviceInfo = nullptr;
    delete deviceInfo;
}