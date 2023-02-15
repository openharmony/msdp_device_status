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
const std::string ALGO_LIB_PATH = "libdevicestatus_algo.z.so";
std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> g_devicestatusDataMap;
DevicestatusMsdpClientImpl::CallbackManager g_callbacksMgr;
using clientType = DevicestatusDataUtils::DevicestatusType;
using clientValue = DevicestatusDataUtils::DevicestatusValue;
DevicestatusMsdpInterface* g_msdpInterface;
DevicestatusMsdpInterface* g_algo = nullptr;
}

ErrCode DevicestatusMsdpClientImpl::InitMsdpImpl(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (g_algo == nullptr) {
        g_algo = GetAlgoObj();
        if (g_algo == nullptr) {
            DEV_HILOGE(SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }
    if (g_algo->Enable(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "Success to enable algo module");
        return RET_OK;
    }
    if (g_msdpInterface == nullptr) {
        g_msdpInterface = GetAlgorithmInst();
        if (g_msdpInterface == nullptr) {
            DEV_HILOGI(SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }
    if (g_msdpInterface->Enable(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "Success to enable mock module");
        return RET_OK;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_NG;
}

ErrCode DevicestatusMsdpClientImpl::DisableMsdpImpl(DevicestatusDataUtils::DevicestatusType type)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (g_algo == nullptr) {
        DEV_HILOGE(SERVICE, "algo object is nullptr");
        return ERR_NG;
    }
    if (g_algo->Disable(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "Success to disable algo module");
        return RET_OK;
    }
    if (g_msdpInterface == nullptr) {
        DEV_HILOGE(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }
    if (g_msdpInterface->Disable(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "Success to disable mock module");
        return RET_OK;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_NG;
}

ErrCode DevicestatusMsdpClientImpl::RegisterImpl(const CallbackManager& callback)
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    g_callbacksMgr = callback;
    RegisterMsdp();
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterImpl()
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGI(SERVICE, "unregister callback failed");
        return ERR_NG;
    }
    UnregisterMsdp();
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

ErrCode DevicestatusMsdpClientImpl::RegisterMsdp()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_algo == nullptr) {
        DEV_HILOGE(SERVICE, "algo object is nullptr");
        return ERR_NG;
    }
    std::shared_ptr<MsdpAlgorithmCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
    if (g_algo->RegisterCallback(callback) == RET_OK) {
        DEV_HILOGI(SERVICE, "success register for algo lib");
        return RET_OK;
    }
    if (g_msdpInterface == nullptr) {
        DEV_HILOGE(SERVICE, "disable msdp impl failed");
        return ERR_NG;
    }
    if (g_msdpInterface->RegisterCallback(callback) == RET_OK) {
        DEV_HILOGI(SERVICE, "success register for mock lib");
        return RET_OK;
    };
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_NG;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterMsdp(void)
{
    DEV_HILOGI(SERVICE, "Enter");
    if (g_algo == nullptr) {
        DEV_HILOGE(SERVICE, "algo object is nullptr");
        return ERR_NG;
    }
    if (g_algo->UnregisterCallback() == RET_OK) {
        DEV_HILOGI(SERVICE, "Success unregister for algo lib");
        g_algo = nullptr;
        return RET_OK;
    }
    if (g_msdpInterface == nullptr) {
        DEV_HILOGE(SERVICE, "Unregister callback failed");
        return ERR_NG;
    }
    if (g_msdpInterface->UnregisterCallback() == RET_OK) {
        DEV_HILOGI(SERVICE, "Success unregister for mock lib");
        g_msdpInterface = nullptr;
        return RET_OK;
    };
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_NG;
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
    std::lock_guard<std::mutex> lock(mutex_);
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

int32_t DevicestatusMsdpClientImpl::LoadAlgorithmLibrary()
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
    mAlgorithm_.create = (DevicestatusMsdpInterface* (*)()) dlsym(mAlgorithm_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    mAlgorithm_.destroy = (void *(*)(DevicestatusMsdpInterface*))dlsym(mAlgorithm_.handle, "Destroy");

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

int32_t DevicestatusMsdpClientImpl::UnloadAlgorithmLibrary()
{
    DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (mAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "handle is nullptr");
        return ERR_NG;
    }

    if (mAlgorithm_.pAlgorithm != nullptr) {
        mAlgorithm_.destroy(mAlgorithm_.pAlgorithm);
        mAlgorithm_.pAlgorithm = nullptr;
    }

    dlclose(mAlgorithm_.handle);
    mAlgorithm_.Clear();
    DEV_HILOGI(SERVICE, "Exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::LoadAlgoLib()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (algoHandler_.handle != nullptr) {
        DEV_HILOGW(SERVICE, "handle has been dlopened");
        return RET_OK;
    }
    algoHandler_.handle = dlopen(ALGO_LIB_PATH.c_str(), RTLD_LAZY);
    if (algoHandler_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error:%{public}s", dlerror());
        return RET_ERR;
    }
    algoHandler_.create = reinterpret_cast<CreateFunc>(dlsym(algoHandler_.handle, "Create"));
    algoHandler_.destroy = reinterpret_cast<DestroyFunc>(dlsym(algoHandler_.handle, "Destroy"));
    if (algoHandler_.create == nullptr || algoHandler_.destroy == nullptr) {
        dlclose(algoHandler_.handle);
        algoHandler_.Clear();
        return RET_ERR;
    }
    return RET_OK;
}

int32_t DevicestatusMsdpClientImpl::UnloadAlgoLib()
{
   DEV_HILOGI(SERVICE, "Enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (algoHandler_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "handle is nullptr");
        return ERR_NG;
    }

    if (algoHandler_.pAlgorithm != nullptr) {
        algoHandler_.destroy(algoHandler_.pAlgorithm);
        algoHandler_.pAlgorithm = nullptr;
    }

    dlclose(algoHandler_.handle);
    algoHandler_.Clear();
    DEV_HILOGI(SERVICE, "Exit");
    return RET_OK;
}

DevicestatusMsdpInterface* DevicestatusMsdpClientImpl::GetAlgorithmInst()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (mAlgorithm_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "handle is nullptr");
        return nullptr;
    }

    if (mAlgorithm_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (mAlgorithm_.pAlgorithm == nullptr) {
            DEV_HILOGI(SERVICE, "Get mAlgorithm.pAlgorithm");
            mAlgorithm_.pAlgorithm = mAlgorithm_.create();
        }
    }

    DEV_HILOGI(SERVICE, "Exit");
    return mAlgorithm_.pAlgorithm;
}

DevicestatusMsdpInterface* DevicestatusMsdpClientImpl::GetAlgoObj()
{
    DEV_HILOGI(SERVICE, "Enter");
    if (algoHandler_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Algorithm not start");
        return nullptr;
    }
    if (algoHandler_.pAlgorithm == nullptr) {
        algoHandler_.pAlgorithm = algoHandler_.create();
    }
    DEV_HILOGI(SERVICE, "Exit");
    return algoHandler_.pAlgorithm;
}
} // namespace Msdp
} // namespace OHOS