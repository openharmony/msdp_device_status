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

#include <string>
#include <string_view>

#include <dlfcn.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include "devicestatus_common.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr std::string_view DEVICESTATUS_MOCK_LIB_PATH = "libdevicestatus_mock.z.so";
constexpr std::string_view DEVICESTATUS_ALGO_LIB_PATH = "libdevicestatus_algo.z.so";
std::map<Type, OnChangedValue> g_devicestatusDataMap;
DeviceStatusMsdpClientImpl::CallbackManager g_callbacksMgr;
using ClientType = Type;
using ClientValue = OnChangedValue;
IMsdp* g_IAlgo = nullptr;
IMsdp* g_IMock = nullptr;
} // namespace

DeviceStatusMsdpClientImpl::DeviceStatusMsdpClientImpl()
{
    for (int32_t type = 0; type < static_cast<int32_t>(Type::TYPE_MAX); ++type) {
        algoCallCount_[static_cast<Type>(type)] = 0;
        mockCallCount_[static_cast<Type>(type)] = 0;
    }
}

ErrCode DeviceStatusMsdpClientImpl::InitMsdpImpl(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (GetSensorHdi(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "GetSensorHdi is support");
        return RET_OK;
    }

    if (AlgoHandle(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "AlgoHandle is support");
        return RET_OK;
    }
    if (MockHandle(type) == RET_OK) {
        DEV_HILOGI(SERVICE, "MockHandle is support");
        return RET_OK;
    }
    DEV_HILOGE(SERVICE, "Exit");
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::MockHandle(Type type)
{
    if (StartMock(type) == RET_ERR) {
        DEV_HILOGE(SERVICE, "Start mock Library failed");
        return RET_ERR;
    }
    if (g_IMock == nullptr) {
        DEV_HILOGE(SERVICE, "Start mock failed, g_IMock is nullptr");
        return RET_ERR;
    }
    g_IMock->Enable(type);
    if (mockCallCount_.find(type) == mockCallCount_.end()) {
        DEV_HILOGE(SERVICE, "Mock enable failed");
        return RET_ERR;
    }
    mockCallCount_[type]++;
    RegisterMock();
    DEV_HILOGI(SERVICE, "mockCallCount_ %{public}d", mockCallCount_[type]);
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::AlgoHandle(Type type)
{
    if (GetAlgoAbility(type) == RET_ERR) {
        DEV_HILOGE(SERVICE, "Algo Library is not support");
        return RET_ERR;
    }
    if (StartAlgo(type) == RET_ERR) {
        DEV_HILOGE(SERVICE, "Start algo Library failed");
        return RET_ERR;
    }
    if (g_IAlgo == nullptr) {
        DEV_HILOGE(SERVICE, "Start algo Library failed, g_IAlgo is nullptr");
        return RET_ERR;
    }
    if ((g_IAlgo->Enable(type)) == RET_ERR) {
        DEV_HILOGE(SERVICE, "Enable algo Library failed");
        return RET_ERR;
    }
    if (algoCallCount_.find(type) == algoCallCount_.end()) {
        DEV_HILOGE(SERVICE, "algo enable failed");
        return RET_ERR;
    }
    algoCallCount_[type]++;
    RegisterAlgo();
    DEV_HILOGI(SERVICE, "algoCallCount_ %{public}d", algoCallCount_[type]);
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::StartAlgo(Type type)
{
    if (LoadAlgoLibrary() == RET_ERR) {
        DEV_HILOGE(SERVICE, "Load algo Library failed");
        return RET_ERR;
    }
    g_IAlgo = GetAlgoInst(type);
    if (g_IAlgo == nullptr) {
        DEV_HILOGE(SERVICE, "Get algo module failed");
        return RET_ERR;
    }
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::StartMock(Type type)
{
    if (LoadMockLibrary() == RET_ERR) {
        DEV_HILOGE(SERVICE, "Load mock Library failed");
        return RET_ERR;
    }
    g_IMock = GetMockInst(type);
    if (g_IMock == nullptr) {
        DEV_HILOGE(SERVICE, "Get mock module failed");
        return RET_ERR;
    }
    return RET_OK;
}


ErrCode DeviceStatusMsdpClientImpl::GetSensorHdi(Type type)
{
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::GetAlgoAbility(Type type)
{
    if (type == Type::TYPE_ABSOLUTE_STILL ||type == Type::TYPE_HORIZONTAL_POSITION ||
        type == Type::TYPE_VERTICAL_POSITION) {
        return RET_OK;
    }
    DEV_HILOGI(SERVICE, "Not support ability");
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::Disable(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (SensorHdiDisable(type) == RET_OK) {
        return RET_OK;
    }
    if (AlgoDisable(type) == RET_OK) {
        return RET_OK;
    }
    if (MockDisable(type) == RET_OK) {
        return RET_OK;
    }
    DEV_HILOGD(SERVICE, "Exit");
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::SensorHdiDisable(Type type)
{
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::AlgoDisable(Type type)
{
    if (g_IAlgo == nullptr) {
        DEV_HILOGE(SERVICE, "Algo disable failed, g_IAlgo is nullptr");
        return RET_ERR;
    }
    if (algoCallCount_[type] == 0) {
        DEV_HILOGE(SERVICE, "Algo not start");
        return RET_ERR;
    }
    algoCallCount_[type]--;
    if (algoCallCount_[type] != 0) {
        DEV_HILOGE(SERVICE, "The number of subscriptions is greater than 1");
        return RET_ERR;
    }
    g_IAlgo->Disable(type);
    UnregisterAlgo();
    algoCallCount_.erase(type);
    if (algoCallCount_.empty()) {
        if (UnloadAlgoLibrary() == RET_ERR) {
            DEV_HILOGE(SERVICE, "Failed to close algorithm library");
            return RET_ERR;
        }
        DEV_HILOGI(SERVICE, "Close algorithm library");
        g_IAlgo = nullptr;
        g_callbacksMgr = nullptr;
        return RET_OK;
    }
    DEV_HILOGI(SERVICE, "algoCallCount_ %{public}d", algoCallCount_[type]);
    return RET_ERR;
}

ErrCode DeviceStatusMsdpClientImpl::MockDisable(Type type)
{
    if (g_IMock == nullptr) {
        DEV_HILOGE(SERVICE, "Mock disable failed, g_IMock is nullptr");
        return RET_ERR;
    }
    if (mockCallCount_[type] == 0) {
        DEV_HILOGE(SERVICE, "Mock not start");
        return RET_ERR;
    }
    mockCallCount_[type]--;
    if (mockCallCount_[type] != 0) {
        DEV_HILOGE(SERVICE, "The number of subscriptions is greater than 1");
        return RET_ERR;
    }
    mockCallCount_.erase(type);
    g_IMock->Disable(type);
    UnregisterMock();
    if (mockCallCount_.empty()) {
        if (UnloadMockLibrary() == RET_ERR) {
            DEV_HILOGE(SERVICE, "Failed to close library");
            return RET_ERR;
        }
        g_IMock = nullptr;
        g_callbacksMgr = nullptr;
    }
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::ImplCallback(const Data& data)
{
    if (g_callbacksMgr == nullptr) {
        DEV_HILOGE(SERVICE, "g_callbacksMgr is nullptr");
        return RET_ERR;
    }
    g_callbacksMgr(data);

    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::RegisterImpl(const CallbackManager& callback)
{
    DEV_HILOGD(SERVICE, "Enter");
    g_callbacksMgr = callback;
    return RET_OK;
}

void DeviceStatusMsdpClientImpl::OnResult(const Data& data)
{
    MsdpCallback(data);
}

ErrCode DeviceStatusMsdpClientImpl::RegisterMock()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (g_IMock != nullptr) {
        std::shared_ptr<IMsdp::MsdpAlgoCallback> callback = std::make_shared<DeviceStatusMsdpClientImpl>();
        g_IMock->RegisterCallback(callback);
    }

    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnregisterMock()
{
    DEV_HILOGD(SERVICE, "Enter");

    if (g_IMock == nullptr) {
        DEV_HILOGE(SERVICE, "Unregister mock callback failed");
        return RET_ERR;
    }

    g_IMock->UnregisterCallback();
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::RegisterAlgo()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (g_IAlgo != nullptr) {
        std::shared_ptr<IMsdp::MsdpAlgoCallback> callback_ = std::make_shared<DeviceStatusMsdpClientImpl>();
        g_IAlgo->RegisterCallback(callback_);
    }
    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnregisterAlgo()
{
    DEV_HILOGD(SERVICE, "Enter");

    if (g_IAlgo == nullptr) {
        DEV_HILOGE(SERVICE, "Unregister algo callback failed");
        return RET_ERR;
    }

    g_IAlgo->UnregisterCallback();

    DEV_HILOGD(SERVICE, "Exit");
    return RET_OK;
}

int32_t DeviceStatusMsdpClientImpl::MsdpCallback(const Data& data)
{
    DEV_HILOGD(SERVICE, "Enter");
    DeviceStatusDumper::GetInstance().PushDeviceStatus(data);
    SaveObserverData(data);
    if (notifyManagerFlag_) {
        ImplCallback(data);
        notifyManagerFlag_ = false;
    }
    return RET_OK;
}

Data DeviceStatusMsdpClientImpl::SaveObserverData(const Data& data)
{
    DEV_HILOGD(SERVICE, "Enter");
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

std::map<ClientType, ClientValue> DeviceStatusMsdpClientImpl::GetObserverData() const
{
    DEV_HILOGD(SERVICE, "Enter");
    return g_devicestatusDataMap;
}

void DeviceStatusMsdpClientImpl::GetDeviceStatusTimestamp()
{
    DEV_HILOGD(SERVICE, "Enter");
    DEV_HILOGD(SERVICE, "Exit");
}

void DeviceStatusMsdpClientImpl::GetLongtitude()
{
    DEV_HILOGD(SERVICE, "Enter");
    DEV_HILOGD(SERVICE, "Exit");
}

void DeviceStatusMsdpClientImpl::GetLatitude()
{
    DEV_HILOGD(SERVICE, "Enter");
    DEV_HILOGD(SERVICE, "Exit");
}

ErrCode DeviceStatusMsdpClientImpl::LoadMockLibrary()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (mock_.handle != nullptr) {
        return RET_OK;
    }
    mock_.handle = dlopen(static_cast<std::string>(DEVICESTATUS_MOCK_LIB_PATH).c_str(), RTLD_LAZY);
    if (mock_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error = %{public}s", dlerror());
        return RET_ERR;
    }
    DEV_HILOGI(SERVICE, "start create pointer");
    mock_.create = (IMsdp* (*)()) dlsym(mock_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    mock_.destroy = (void *(*)(IMsdp*))dlsym(mock_.handle, "Destroy");
    if (mock_.create == nullptr || mock_.destroy == nullptr) {
        DEV_HILOGE(SERVICE, "%{public}s dlsym Create or Destroy failed",
            static_cast<std::string>(DEVICESTATUS_MOCK_LIB_PATH).c_str());
        dlclose(mock_.handle);
        mock_.Clear();
        if (mock_.handle == nullptr) {
            return RET_OK;
        }
        DEV_HILOGE(SERVICE, "Load mock failed");
        return RET_ERR;
    }
    mock_.pAlgorithm = mock_.create();
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnloadMockLibrary()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (mock_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Unload mock failed");
        return RET_ERR;
    }
    if (mock_.pAlgorithm != nullptr) {
        mock_.destroy(mock_.pAlgorithm);
        mock_.pAlgorithm = nullptr;
    }
    dlclose(mock_.handle);
    mock_.Clear();
    return RET_OK;
}

IMsdp* DeviceStatusMsdpClientImpl::GetMockInst(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (mock_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Mock not start");
        return nullptr;
    }
    if (mock_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        mock_.pAlgorithm = mock_.create();
        mockCallCount_[type] = 0;
    }
    return mock_.pAlgorithm;
}

ErrCode DeviceStatusMsdpClientImpl::LoadAlgoLibrary()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (algo_.handle != nullptr) {
        return RET_OK;
    }
    algo_.handle = dlopen(static_cast<std::string>(DEVICESTATUS_ALGO_LIB_PATH).c_str(), RTLD_LAZY);
    if (algo_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Cannot load library error = %{public}s", dlerror());
        return RET_ERR;
    }
    DEV_HILOGI(SERVICE, "start create pointer");
    algo_.create = (IMsdp* (*)()) dlsym(algo_.handle, "Create");
    DEV_HILOGI(SERVICE, "start destroy pointer");
    algo_.destroy = (void *(*)(IMsdp*))dlsym(algo_.handle, "Destroy");
    if (algo_.create == nullptr || algo_.destroy == nullptr) {
        DEV_HILOGE(SERVICE, "%{public}s dlsym Create or Destroy failed",
            static_cast<std::string>(DEVICESTATUS_ALGO_LIB_PATH).c_str());
        dlclose(algo_.handle);
        algo_.Clear();
        if (algo_.handle == nullptr) {
            return RET_OK;
        }
        DEV_HILOGE(SERVICE, "Load algo failed");
        return RET_ERR;
    }
    algo_.pAlgorithm = algo_.create();
    return RET_OK;
}

ErrCode DeviceStatusMsdpClientImpl::UnloadAlgoLibrary()
{
    DEV_HILOGD(SERVICE, "Enter");
    if (algo_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Unload algo failed");
        return RET_ERR;
    }
    if (algo_.pAlgorithm != nullptr) {
        algo_.destroy(algo_.pAlgorithm);
        algo_.pAlgorithm = nullptr;
    }
    dlclose(algo_.handle);
    algo_.Clear();
    return RET_OK;
}

IMsdp* DeviceStatusMsdpClientImpl::GetAlgoInst(Type type)
{
    DEV_HILOGD(SERVICE, "Enter");
    if (algo_.handle == nullptr) {
        DEV_HILOGE(SERVICE, "Algo not start");
        return nullptr;
    }
    if (algo_.pAlgorithm == nullptr) {
        std::unique_lock<std::mutex> lock(mMutex_);
        algoCallCount_[type] = 0;
        algo_.pAlgorithm = algo_.create();
    }

    return algo_.pAlgorithm;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
