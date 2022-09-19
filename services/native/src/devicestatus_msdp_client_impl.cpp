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
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NG = -1;
const std::string DEVICESTATUS_SENSOR_HDI_LIB_PATH = "libdevicestatus_sensorhdi.z.so";
const std::string DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH = "libdevicestatus_msdp.z.so";
const std::string DEVICESTATUS_ALGORITHM_MANAGER_LIB_PATH = "libdevicestatus_algorithm_manager.z.so";
std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> g_devicestatusDataMap;
DevicestatusMsdpClientImpl::CallbackManager g_callbacksMgr;
using clientType = DevicestatusDataUtils::DevicestatusType;
using clientValue = DevicestatusDataUtils::DevicestatusValue;
DevicestatusMsdpInterface* g_msdpInterface;
DevicestatusSensorInterface* g_sensorHdiInterface;
DevicestatusAlgorithmManagerInterface* g_devAlgorithmInterface;
}

ErrCode DevicestatusMsdpClientImpl::InitMsdpImpl(const DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface == nullptr) {
        g_msdpInterface = GetAlgorithmInst();
        if (g_msdpInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }

    if (g_sensorHdiInterface == nullptr) {
        g_sensorHdiInterface = GetSensorHdiInst();
        if (g_sensorHdiInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get sensor module instance failed");
            return ERR_NG;
        }
    }

    if (g_devAlgorithmInterface == nullptr) {
        g_devAlgorithmInterface = GetDevAlgorithmInst();
        if (g_devAlgorithmInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get dev_alogrithm_manager module instance failed");
            return ERR_NG;
        }
    }

    try {
        if (type == DevicestatusDataUtils::DevicestatusType::TYPE_LID_OPEN) {
            g_sensorHdiInterface->Enable(type);
        } else {
            g_msdpInterface->Enable();
            g_sensorHdiInterface->Enable(type);
            g_devAlgorithmInterface->Enable(type);
        }
    }
    catch (std::exception const& e) {
        DEV_HILOGE(SERVICE, "load algorithm library failed");
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::DisableMsdpImpl(const DevicestatusDataUtils::DevicestatusType& type)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface == nullptr) {
        DEV_HILOGI(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }

    if (g_sensorHdiInterface == nullptr) {
        DEV_HILOGI(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }

    if (g_devAlgorithmInterface == nullptr) {
        DEV_HILOGI(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }

    try {
        g_msdpInterface->Disable();
        g_sensorHdiInterface->Disable(type);
        g_devAlgorithmInterface->Disable(type);
    }
    catch (std::exception const& e) {
        DEV_HILOGE(SERVICE, "disable msdp impl failed");
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::RegisterSensor()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_sensorHdiInterface != nullptr) {
        std::shared_ptr<DevicestatusSensorHdiCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
        try {
            g_sensorHdiInterface->RegisterCallback(callback);
        }
        catch (std::exception const& e) {
            DEV_HILOGE(SERVICE, "register callback failed");
        }
        DEV_HILOGI(SERVICE, "g_sensorHdiInterface is not nullptr");
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterSensor(void)
{
    DEV_HILOGI(SERVICE, "Enter");

    if (g_sensorHdiInterface == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    try {
        g_sensorHdiInterface->UnregisterCallback();
    }
    catch (std::exception const& e) {
        DEV_HILOGE(SERVICE, "unregister callback failed");
    }

    g_sensorHdiInterface = nullptr;

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::RegisterImpl(const CallbackManager& callback)
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

    if (g_sensorHdiInterface == nullptr) {
        g_sensorHdiInterface = GetSensorHdiInst();
        if (g_sensorHdiInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get sensor module instance failed");
            return ERR_NG;
        }
    }

    if (g_devAlgorithmInterface == nullptr) {
        g_devAlgorithmInterface = GetDevAlgorithmInst();
        if (g_devAlgorithmInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get dev_alogrithm_manager module instance failed");
            return ERR_NG;
        }
    }

    RegisterMsdp();
    RegisterSensor();
    RegisterDevAlgorithm();

    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterImpl()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    UnregisterMsdp();
    UnregisterSensor();
    UnregisterDevAlgorithm();

    g_callbacksMgr = nullptr;

    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::ImplCallback(const DevicestatusDataUtils::DevicestatusData& data)
{
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGI(SERVICE, "g_callbacksMgr is nullptr");
        return ERR_NG;
    }
    g_callbacksMgr(data);

    return ERR_OK;
}

void DevicestatusMsdpClientImpl::OnResult(const DevicestatusDataUtils::DevicestatusData& data)
{
    MsdpCallback(data);
}

void DevicestatusMsdpClientImpl::OnSensorHdiResult(const DevicestatusDataUtils::DevicestatusData& data)
{
    MsdpCallback(data);
}

void DevicestatusMsdpClientImpl::OnAlogrithmResult(const DevicestatusDataUtils::DevicestatusData& data)
{
    MsdpCallback(data);
}

ErrCode DevicestatusMsdpClientImpl::RegisterMsdp()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_msdpInterface != nullptr) {
        std::shared_ptr<MsdpAlgorithmCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
        try {
            g_msdpInterface->RegisterCallback(callback);
        }
        catch (std::exception const& e) {
            DEV_HILOGE(SERVICE, "register callback failed");
        }
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterMsdp(void)
{
    DEV_HILOGI(SERVICE, "Enter");

    if (g_msdpInterface == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    try {
        g_msdpInterface->UnregisterCallback();
    }
    catch (std::exception const& e) {
        DEV_HILOGE(SERVICE, "register callback failed");
    }

    g_msdpInterface = nullptr;

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::RegisterDevAlgorithm()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_devAlgorithmInterface != nullptr) {
        std::shared_ptr<DevicestatusAlgorithmCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
        try {
            g_devAlgorithmInterface->RegisterCallback(callback);
        }
        catch (std::exception const& e) {
            DEV_HILOGE(SERVICE, "register callback failed");
        }
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterDevAlgorithm(void)
{
    DEV_HILOGI(SERVICE, "Enter");

    if (g_devAlgorithmInterface == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }

    try {
        g_devAlgorithmInterface->UnregisterCallback();
    }
    catch (std::exception const& e) {
        DEV_HILOGE(SERVICE, "register callback failed");
    }

    g_devAlgorithmInterface = nullptr;

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::MsdpCallback(const DevicestatusDataUtils::DevicestatusData& data)
{
    DEV_HILOGI(SERVICE, "Enter");
    DevicestatusDumper::GetInstance().pushDeviceStatus(data);
    SaveObserverData(data);
    if (notifyManagerFlag_) {
        ImplCallback(data);
        notifyManagerFlag_ = false;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusMsdpClientImpl::SaveObserverData(
    const DevicestatusDataUtils::DevicestatusData& data)
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

std::map<clientType, clientValue> DevicestatusMsdpClientImpl::GetObserverData() const
{
    DEV_HILOGI(SERVICE, "Enter");
    return g_devicestatusDataMap;
}

void DevicestatusMsdpClientImpl::GetDevicestatusTimestamp()
{
    DEV_HILOGI(SERVICE, "Enter");

    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusMsdpClientImpl::GetLongtitude()
{
    DEV_HILOGI(SERVICE, "Enter");

    DEV_HILOGI(SERVICE, "Exit");
}

void DevicestatusMsdpClientImpl::GetLatitude()
{
    DEV_HILOGI(SERVICE, "Enter");

    DEV_HILOGI(SERVICE, "Exit");
}

int32_t DevicestatusMsdpClientImpl::LoadSensorHdiLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (sensorHdi_.handle != nullptr) {
        return ERR_OK;
    }
    sensorHdi_.handle = dlopen(DEVICESTATUS_SENSOR_HDI_LIB_PATH.c_str(), RTLD_LAZY);
    if (sensorHdi_.handle == nullptr) {
        DEV_HILOGE(SERVICE,
            "Cannot load sensor hdi library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEV_HILOGI(SERVICE, "start create sensor hdi pointer");
    sensorHdi_.create = (DevicestatusSensorInterface* (*)()) dlsym(sensorHdi_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy sensor hdi pointer");
    sensorHdi_.destroy = (void *(*)(DevicestatusSensorInterface*))dlsym(sensorHdi_.handle, "Destroy");

    if (sensorHdi_.create == nullptr || sensorHdi_.destroy == nullptr) {
        DEV_HILOGI(SERVICE, "%{public}s dlsym Create or Destroy sensor hdi failed!",
            DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str());
        dlclose(sensorHdi_.handle);
        sensorHdi_.Clear();
        bCreate = false;
        return ERR_NG;
    }

    if (bCreate) {
        sensorHdi_.pAlgorithm = sensorHdi_.create();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::UnloadSensorHdiLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (sensorHdi_.handle == nullptr) {
        return ERR_NG;
    }

    if (sensorHdi_.pAlgorithm != nullptr) {
        sensorHdi_.destroy(sensorHdi_.pAlgorithm);
        sensorHdi_.pAlgorithm = nullptr;
    }

    if (!bCreate) {
        dlclose(sensorHdi_.handle);
        sensorHdi_.Clear();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

DevicestatusSensorInterface* DevicestatusMsdpClientImpl::GetSensorHdiInst()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (sensorHdi_.handle == nullptr) {
        return nullptr;
    }

    if (sensorHdi_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        if (sensorHdi_.pAlgorithm == nullptr) {
            DEV_HILOGI(SERVICE, "Get mAlgorithm.pAlgorithm");
            sensorHdi_.pAlgorithm = sensorHdi_.create();
        }
    }

    return sensorHdi_.pAlgorithm;
}

int32_t DevicestatusMsdpClientImpl::LoadAlgorithmLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle != nullptr) {
        return ERR_OK;
    }
    mAlgorithm_.handle = dlopen(DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str(), RTLD_LAZY);
    if (mAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEV_HILOGI(SERVICE, "start create pointer");
    mAlgorithm_.create = (DevicestatusMsdpInterface* (*)()) dlsym(mAlgorithm_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    mAlgorithm_.destroy = (void *(*)(DevicestatusMsdpInterface*))dlsym(mAlgorithm_.handle, "Destroy");

    if (mAlgorithm_.create == nullptr || mAlgorithm_.destroy == nullptr) {
        DEV_HILOGI(SERVICE, "%{public}s dlsym Create or Destroy failed!",
            DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str());
        dlclose(mAlgorithm_.handle);
        mAlgorithm_.Clear();
        bCreate = false;
        return ERR_NG;
    }

    if (bCreate) {
        mAlgorithm_.pAlgorithm = mAlgorithm_.create();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::UnloadAlgorithmLibrary(bool bCreate)
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

DevicestatusMsdpInterface* DevicestatusMsdpClientImpl::GetAlgorithmInst()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle == nullptr) {
        return nullptr;
    }

    if (mAlgorithm_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        if (mAlgorithm_.pAlgorithm == nullptr) {
            DEV_HILOGI(SERVICE, "Get mAlgorithm.pAlgorithm");
            mAlgorithm_.pAlgorithm = mAlgorithm_.create();
        }
    }

    return mAlgorithm_.pAlgorithm;
}

int32_t DevicestatusMsdpClientImpl::LoadDevAlgorithmLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devAlgorithm_.handle != nullptr) {
        return ERR_OK;
    }
    devAlgorithm_.handle = dlopen(DEVICESTATUS_ALGORITHM_MANAGER_LIB_PATH.c_str(), RTLD_LAZY);
    if (devAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEV_HILOGI(SERVICE, "start create pointer");
    devAlgorithm_.create = (DevicestatusAlgorithmManagerInterface* (*)()) dlsym(devAlgorithm_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    devAlgorithm_.destroy = (void *(*)(DevicestatusAlgorithmManagerInterface*))dlsym(devAlgorithm_.handle, "Destroy");

    if (devAlgorithm_.create == nullptr || devAlgorithm_.destroy == nullptr) {
        DEV_HILOGI(SERVICE, "%{public}s dlsym Create or Destroy failed!",
            DEVICESTATUS_ALGORITHM_MANAGER_LIB_PATH.c_str());
        dlclose(devAlgorithm_.handle);
        devAlgorithm_.Clear();
        bCreate = false;
        return ERR_NG;
    }

    if (bCreate) {
        devAlgorithm_.pAlgorithm = devAlgorithm_.create();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::UnloadDevAlgorithmLibrary(bool bCreate)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devAlgorithm_.handle == nullptr) {
        return ERR_NG;
    }

    if (devAlgorithm_.pAlgorithm != nullptr) {
        devAlgorithm_.destroy(devAlgorithm_.pAlgorithm);
        devAlgorithm_.pAlgorithm = nullptr;
    }

    if (!bCreate) {
        dlclose(devAlgorithm_.handle);
        devAlgorithm_.Clear();
    }

    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

DevicestatusAlgorithmManagerInterface* DevicestatusMsdpClientImpl::GetDevAlgorithmInst()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (devAlgorithm_.handle == nullptr) {
        return nullptr;
    }

    if (devAlgorithm_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        if (devAlgorithm_.pAlgorithm == nullptr) {
            DEV_HILOGI(SERVICE, "Get mAlgorithm.pAlgorithm");
            devAlgorithm_.pAlgorithm = devAlgorithm_.create();
        }
    }

    return devAlgorithm_.pAlgorithm;
}
}
}
