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

#include "devicestatus_msdp_client_impl.h"

#include <dlfcn.h>
#include <string>
#include <cerrno>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <errors.h>
#include <linux/netlink.h>

#include "devicestatus_common.h"

using namespace OHOS::NativeRdb;
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NG = -1;
const std::string DEVICESTATUS_SENSOR_HDI_LIB_PATH = "libdevicestatus_sensorhdi.z.so";
const std::string DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH = "libdevicestatus_msdp.z.so";
std::map<DeviceStatusDataUtils::DeviceStatusType, DeviceStatusDataUtils::DeviceStatusValue> g_devicestatusDataMap;
DeviceStatusMsdpClientImpl::CallbackManager g_callbacksMgr;
using clientType = DeviceStatusDataUtils::DeviceStatusType;
using clientValue = DeviceStatusDataUtils::DeviceStatusValue;
DeviceStatusMsdpInterface* g_msdpInterface;
}

ErrCode DeviceStatusMsdpClientImpl::InitMsdpImpl(DeviceStatusDataUtils::DeviceStatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface == nullptr) {
        g_msdpInterface = GetAlgorithmInst();
        if (g_msdpInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }

    g_msdpInterface->Enable(type);

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DeviceStatusMsdpClientImpl::DisableMsdpImpl(DeviceStatusDataUtils::DeviceStatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface == nullptr) {
        DEV_HILOGI(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }

    g_msdpInterface->Disable(type);
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DeviceStatusMsdpClientImpl::RegisterImpl(const CallbackManager& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    g_callbacksMgr = callback;

    if (g_msdpInterface == nullptr) {
        g_msdpInterface = GetAlgorithmInst();
        if (g_msdpInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }

    RegisterMsdp();

    return ERR_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnregisterImpl()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    UnregisterMsdp();

    g_callbacksMgr = nullptr;

    return ERR_OK;
}

ErrCode DeviceStatusMsdpClientImpl::ImplCallback(const DeviceStatusDataUtils::DeviceStatusData& data)
{
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGI(SERVICE, "g_callbacksMgr is nullptr");
        return ERR_NG;
    }
    g_callbacksMgr(data);

    return ERR_OK;
}

void DeviceStatusMsdpClientImpl::OnResult(const DeviceStatusDataUtils::DeviceStatusData& data)
{
    MsdpCallback(data);
}

ErrCode DeviceStatusMsdpClientImpl::RegisterMsdp()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface != nullptr) {
        std::shared_ptr<MsdpAlgorithmCallback> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
        g_msdpInterface->RegisterCallback(callback);
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnregisterMsdp(void)
{
    DEV_HILOGI(SERVICE, "Enter");

    if (g_msdpInterface == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    g_msdpInterface->UnregisterCallback();
    g_msdpInterface = nullptr;

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DeviceStatusMsdpClientImpl::MsdpCallback(const DeviceStatusDataUtils::DeviceStatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    DeviceStatusDumper::GetInstance().pushDeviceStatus(data);
    SaveObserverData(data);
    if (notifyManagerFlag_) {
        ImplCallback(data);
        notifyManagerFlag_ = false;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

DeviceStatusDataUtils::DeviceStatusData DeviceStatusMsdpClientImpl::SaveObserverData(
    const DeviceStatusDataUtils::DeviceStatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    for (auto iter = g_devicestatusDataMap.begin(); iter != g_devicestatusDataMap.end(); ++iter) {
        if (iter->first == data.type) {
            iter->second = data.value;
            notifyManagerFlag_ = true;
            return data;
        }
    }

    g_devicestatusDataMap.insert(std::make_pair(data.type, data.value));
    notifyManagerFlag_ = true;

    return data;
}

std::map<clientType, clientValue> DeviceStatusMsdpClientImpl::GetObserverData() const
{
    DEV_HILOGI(SERVICE, "Enter");
    return g_devicestatusDataMap;
}

int32_t DeviceStatusMsdpClientImpl::LoadAlgorithmLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle != nullptr) {
        return ERR_OK;
    }

    mAlgorithm_.handle = dlopen(static_cast<std::string>(DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH).c_str(), RTLD_LAZY);
    if (mAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEV_HILOGI(SERVICE, "start create pointer");
    mAlgorithm_.create = (DeviceStatusMsdpInterface* (*)()) dlsym(mAlgorithm_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    mAlgorithm_.destroy = (void *(*)(DeviceStatusMsdpInterface*))dlsym(mAlgorithm_.handle, "Destroy");

    if (mAlgorithm_.create == nullptr || mAlgorithm_.destroy == nullptr) {
        DEV_HILOGE(SERVICE, "%{public}s dlsym Create or Destroy failed",
            static_cast<std::string>(DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH).c_str());
        dlclose(mAlgorithm_.handle);
        mAlgorithm_.Clear();
        if (mAlgorithm_.handle == nullptr) {
            return ERR_OK;
        }
        DEV_HILOGE(SERVICE, "Load algo failed");
        return ERR_NG;
    }

    mAlgorithm_.pAlgorithm = mAlgorithm_.create();
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DeviceStatusMsdpClientImpl::UnloadAlgorithmLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle == nullptr) {
        return ERR_NG;
    }

    if (mAlgorithm_.pAlgorithm != nullptr) {
        mAlgorithm_.destroy(mAlgorithm_.pAlgorithm);
        mAlgorithm_.pAlgorithm = nullptr;
    }

    if (!bCreate) {
        dlclose(mAlgorithm_.handle);
        mAlgorithm_.Clear();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

DeviceStatusMsdpInterface* DeviceStatusMsdpClientImpl::GetAlgorithmInst()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Algorithm not start");
        return nullptr;
    }

    if (mAlgorithm_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        DEV_HILOGI(SERVICE, "Get mAlgorithm.pAlgorithm");
        mAlgorithm_.pAlgorithm = mAlgorithm_.create();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return mAlgorithm_.pAlgorithm;
}
} // namespace DeviceStatus
}
}
