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
#include "dummy_values_bucket.h"
#include "devicestatus_common.h"

using namespace OHOS::NativeRdb;
namespace OHOS {
namespace Msdp {
namespace {
const int ERR_OK = 0;
const int ERR_NG = -1;
const std::string DEVICESTATUS_SENSOR_HDI_LIB_PATH = "libdevicestatus_sensorhdi.z.so";
const std::string DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH = "libdevicestatus_msdp.z.so";
std::map<DevicestatusDataUtils::DevicestatusType, DevicestatusDataUtils::DevicestatusValue> devicestatusDataMap_;
DevicestatusMsdpClientImpl::CallbackManager callbacksMgr_;
using clientType = DevicestatusDataUtils::DevicestatusType;
using clientValue = DevicestatusDataUtils::DevicestatusValue;
}

ErrCode DevicestatusMsdpClientImpl::InitMsdpImpl()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (msdpInterface_ == nullptr) {
        msdpInterface_ = GetAlgorithmInst();
        if (msdpInterface_ == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }

    if (sensorHdiInterface_ == nullptr) {
        sensorHdiInterface_ = GetSensorHdiInst();
        if (sensorHdiInterface_ == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "get sensor module instance failed");
            return ERR_NG;
        }
    }

    msdpInterface_->Enable();
    sensorHdiInterface_->Enable();

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::RegisterSensor()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (sensorHdiInterface_ != nullptr) {
        std::shared_ptr<DevicestatusSensorHdiCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
        sensorHdiInterface_->RegisterCallback(callback);
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "sensorHdiInterface_ is not nullptr");
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterSensor(void)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int32_t ret = sensorHdiInterface_->UnregisterCallback();
    if (ret < 0) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "unregister sensor failed");
        return ERR_NG;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::RegisterImpl(CallbackManager& callback)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    callbacksMgr_ = callback;

    if (msdpInterface_ == nullptr) {
        msdpInterface_ = GetAlgorithmInst();
        if (msdpInterface_ == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "get msdp module instance failed");
            return ERR_NG;
        }
    }

    if (sensorHdiInterface_ == nullptr) {
        sensorHdiInterface_ = GetSensorHdiInst();
        if (sensorHdiInterface_ == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "get sensor module instance failed");
            return ERR_NG;
        }
    }

    RegisterMsdp();
    RegisterSensor();

    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::ImplCallback(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (callbacksMgr_ == nullptr) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "callbacksMgr_ is nullptr");
        return ERR_NG;
    }
    callbacksMgr_(data);

    return ERR_OK;
}

void DevicestatusMsdpClientImpl::OnResult(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    MsdpCallback(data);
}

void DevicestatusMsdpClientImpl::OnSensorHdiResult(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    MsdpCallback(data);
}

ErrCode DevicestatusMsdpClientImpl::RegisterMsdp()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (msdpInterface_ != nullptr) {
        std::shared_ptr<MsdpAlgorithmCallback> callback = std::make_shared<DevicestatusMsdpClientImpl>();
        msdpInterface_->RegisterCallback(callback);
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

ErrCode DevicestatusMsdpClientImpl::UnregisterMsdp(void)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    int32_t ret = msdpInterface_->UnregisterCallback();
    if (ret < 0) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "unregister Msdp failed");
        return ERR_NG;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

int32_t DevicestatusMsdpClientImpl::MsdpCallback(DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    SaveObserverData(data);
    if (notifyManagerFlag_) {
        ImplCallback(data);
        notifyManagerFlag_ = false;
    }

    return ERR_OK;
}

DevicestatusDataUtils::DevicestatusData DevicestatusMsdpClientImpl::SaveObserverData(
    DevicestatusDataUtils::DevicestatusData& data)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    for (auto iter = devicestatusDataMap_.begin(); iter != devicestatusDataMap_.end(); ++iter) {
        if (iter->first == data.type) {
            iter->second = data.value;
            notifyManagerFlag_ = true;
            return data;
        }
    }

    devicestatusDataMap_.insert(std::make_pair(data.type, data.value));
    notifyManagerFlag_ = true;

    return data;
}

std::map<clientType, clientValue> DevicestatusMsdpClientImpl::GetObserverData() const
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    return devicestatusDataMap_;
}

void DevicestatusMsdpClientImpl::GetDevicestatusTimestamp()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

void DevicestatusMsdpClientImpl::GetLongtitude()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

void DevicestatusMsdpClientImpl::GetLatitude()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

int32_t DevicestatusMsdpClientImpl::LoadSensorHdiLibrary(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (sensorHdi_.handle != nullptr) {
        return ERR_OK;
    }
    sensorHdi_.handle = dlopen(DEVICESTATUS_SENSOR_HDI_LIB_PATH.c_str(), RTLD_LAZY);
    if (sensorHdi_.handle == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE,
            "Cannot load sensor hdi library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "start create sensor hdi pointer");
    sensorHdi_.create = (DevicestatusSensorInterface* (*)()) dlsym(sensorHdi_.handle, "Create");
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "start destroy sensor hdi pointer");
    sensorHdi_.destroy = (void *(*)(DevicestatusSensorInterface*))dlsym(sensorHdi_.handle, "Destroy");

    if (sensorHdi_.create == nullptr || sensorHdi_.destroy == nullptr) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "%{public}s dlsym Create or Destory sensor hdi failed!",
            DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str());
        dlclose(sensorHdi_.handle);
        sensorHdi_.Clear();
        bCreate = false;
        return ERR_NG;
    }

    if (bCreate) {
        sensorHdi_.pAlgorithm = sensorHdi_.create();
    }

    return ERR_OK;
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

int32_t DevicestatusMsdpClientImpl::UnloadSensorHdiLibrary(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

DevicestatusSensorInterface* DevicestatusMsdpClientImpl::GetSensorHdiInst()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (sensorHdi_.handle == nullptr) {
        return nullptr;
    }

    if (sensorHdi_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        if (sensorHdi_.pAlgorithm == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Get mAlgorithm.pAlgorithm");
            sensorHdi_.pAlgorithm = sensorHdi_.create();
        }
    }

    return sensorHdi_.pAlgorithm;
}

int32_t DevicestatusMsdpClientImpl::LoadAlgorithmLibrary(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (mAlgorithm_.handle != nullptr) {
        return ERR_OK;
    }
    mAlgorithm_.handle = dlopen(DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str(), RTLD_LAZY);
    if (mAlgorithm_.handle == nullptr) {
        DEVICESTATUS_HILOGE(DEVICESTATUS_MODULE_SERVICE, "Cannot load library error = %{public}s", dlerror());
        return ERR_NG;
    }

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "start create pointer");
    mAlgorithm_.create = (DevicestatusMsdpInterface* (*)()) dlsym(mAlgorithm_.handle, "Create");
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "start destroy pointer");
    mAlgorithm_.destroy = (void *(*)(DevicestatusMsdpInterface*))dlsym(mAlgorithm_.handle, "Destroy");

    if (mAlgorithm_.create == nullptr || mAlgorithm_.destroy == nullptr) {
        DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "%{public}s dlsym Create or Destory failed!",
            DEVICESTATUS_MSDP_ALGORITHM_LIB_PATH.c_str());
        dlclose(mAlgorithm_.handle);
        mAlgorithm_.Clear();
        bCreate = false;
        return ERR_NG;
    }

    if (bCreate) {
        mAlgorithm_.pAlgorithm = mAlgorithm_.create();
    }

    return ERR_OK;
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
}

int32_t DevicestatusMsdpClientImpl::UnloadAlgorithmLibrary(bool bCreate)
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
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

    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "exit");
    return ERR_OK;
}

DevicestatusMsdpInterface* DevicestatusMsdpClientImpl::GetAlgorithmInst()
{
    DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "enter");
    if (mAlgorithm_.handle == nullptr) {
        return nullptr;
    }

    if (mAlgorithm_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        if (mAlgorithm_.pAlgorithm == nullptr) {
            DEVICESTATUS_HILOGI(DEVICESTATUS_MODULE_SERVICE, "Get mAlgorithm.pAlgorithm");
            mAlgorithm_.pAlgorithm = mAlgorithm_.create();
        }
    }

    return mAlgorithm_.pAlgorithm;
}
}
}
