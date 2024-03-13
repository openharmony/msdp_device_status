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

#ifdef DEVICE_STATUS_SENSOR_ENABLE
#include "sensor_data_callback.h"

#include <cmath>
#include <cstdio>

#include "devicestatus_define.h"
#include "include/util.h"

#undef LOG_TAG
#define LOG_TAG "SensorDataCallback"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t RATE_MILLISEC { 100100100 };
} // namespace

SensorDataCallback::SensorDataCallback() {}
SensorDataCallback::~SensorDataCallback()
{
    algoMap_.clear();
    alive_ = false;
    CHKPV(algorithmThread_);
    if (!algorithmThread_->joinable()) {
        FI_HILOGE("Thread join failed");
        return;
    }
    sem_post(&sem_);
    algorithmThread_->join();
    accelDataList_.clear();
}

void SensorDataCallback::Init()
{
    FI_HILOGI("SensorDataCallback is initiated");
    std::lock_guard lock(initMutex_);
    if (algorithmThread_ == nullptr) {
        FI_HILOGI("Create algorithem thread");
        algorithmThread_ = std::make_unique<std::thread>(&SensorDataCallback::AlgorithmLoop, this);
    }
}

bool SensorDataCallback::Unregister()
{
    CALL_DEBUG_ENTER;
    bool ret = UnregisterCallbackSensor(SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER);
    if (!ret) {
        FI_HILOGE("UnregisterCallbackSensor failed");
        return false;
    }
    alive_ = false;
    CHKPF(algorithmThread_);
    if (!algorithmThread_->joinable()) {
        FI_HILOGE("Thread join failed");
        return false;
    }
    sem_post(&sem_);
    algorithmThread_->join();
    return ret;
}

bool SensorDataCallback::SubscribeSensorEvent(int32_t sensorTypeId, SensorCallback callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard lock(callbackMutex_);
    auto ret = algoMap_.insert(std::pair(sensorTypeId, callback));
    if (ret.second) {
        return true;
    }
    FI_HILOGE("SensorCallback is duplicated");
    return false;
}

bool SensorDataCallback::UnsubscribeSensorEvent(int32_t sensorTypeId, SensorCallback callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard lock(callbackMutex_);
    auto callbackIter = algoMap_.find(sensorTypeId);
    if (callbackIter != algoMap_.end()) {
        FI_HILOGE("Erase sensorTypeId:%{public}d", sensorTypeId);
        algoMap_.erase(sensorTypeId);
    }
    return true;
}

bool SensorDataCallback::NotifyCallback(int32_t sensorTypeId, AccelData* data)
{
    CHKPF(data);
    std::lock_guard lock(callbackMutex_);
    for (auto iter = algoMap_.begin(); iter != algoMap_.end(); ++iter) {
        (iter->second)(sensorTypeId, data);
    }
    return true;
}

bool SensorDataCallback::PushData(int32_t sensorTypeId, uint8_t* data)
{
    CALL_DEBUG_ENTER;
    CHKPF(data);
    AccelData* acclData = reinterpret_cast<AccelData*>(data);
    if ((abs(acclData->x) > ACC_VALID_THRHD) ||
        (abs(acclData->y) > ACC_VALID_THRHD) ||
        (abs(acclData->z) > ACC_VALID_THRHD)) {
        FI_HILOGE("Acc data is invalid");
        return false;
    }
    std::lock_guard lock(dataMutex_);
    accelDataList_.emplace_back(*acclData);
    FI_HILOGD("ACCEL pushData:x:%{public}f, y:%{public}f, z:%{public}f, PushData sensorTypeId:%{public}d",
        acclData->x, acclData->y, acclData->z, sensorTypeId);
    sem_post(&sem_);
    return true;
}

bool SensorDataCallback::PopData(int32_t sensorTypeId, AccelData& data)
{
    CALL_DEBUG_ENTER;
    if (sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        FI_HILOGE("Invalid sensorTypeId:%{public}d", sensorTypeId);
        return false;
    }
    std::lock_guard lock(dataMutex_);
    if (accelDataList_.empty()) {
        FI_HILOGE("No accel data");
        return false;
    }
    data = accelDataList_.front();
    accelDataList_.pop_front();
    FI_HILOGD("ACCEL popData:x:%{public}f, y:%{public}f, z:%{public}f, PopData sensorTypeId:%{public}d",
        data.x, data.y, data.z, sensorTypeId);
    return true;
}

static void SensorDataCallbackImpl(SensorEvent *event)
{
    CALL_DEBUG_ENTER;
    CHKPV(event);
    FI_HILOGI("SensorDataCallbackImpl sensorTypeId:%{public}d", event->sensorTypeId);
    SENSOR_DATA_CB.PushData(event->sensorTypeId, event->data);
}

bool SensorDataCallback::RegisterCallbackSensor(int32_t sensorTypeId)
{
    std::lock_guard lock(sensorMutex_);
    user_.callback = SensorDataCallbackImpl;
    int32_t ret = SubscribeSensor(sensorTypeId, &user_);
    if (ret != 0) {
        FI_HILOGE("SubscribeSensor failed");
        return false;
    }
    ret = SetBatch(sensorTypeId, &user_, RATE_MILLISEC, RATE_MILLISEC);
    if (ret != 0) {
        FI_HILOGE("SetBatch failed");
        return false;
    }
    ret = ActivateSensor(sensorTypeId, &user_);
    if (ret != 0) {
        FI_HILOGE("ActivateSensor failed");
        return false;
    }
    return true;
}

bool SensorDataCallback::UnregisterCallbackSensor(int32_t sensorTypeId)
{
    CALL_DEBUG_ENTER;
    std::lock_guard lock(sensorMutex_);
    CHKPF(user_.callback);
    int32_t ret = DeactivateSensor(sensorTypeId, &user_);
    if (ret != 0) {
        FI_HILOGE("DeactivateSensor failed");
        return false;
    }
    ret = UnsubscribeSensor(sensorTypeId, &user_);
    if (ret != 0) {
        FI_HILOGE("UnsubscribeSensor failed");
        return false;
    }
    return true;
}

void SensorDataCallback::AlgorithmLoop()
{
    SetThreadName(std::string("os_loop_sensor"));
    CALL_DEBUG_ENTER;
    while (alive_) {
        sem_wait(&sem_);
        HandleSensorEvent();
    }
}

void SensorDataCallback::HandleSensorEvent()
{
    CALL_DEBUG_ENTER;
    AccelData acclData;
    if (PopData(SENSOR_TYPE_ID_ACCELEROMETER, acclData)) {
        NotifyCallback(SENSOR_TYPE_ID_ACCELEROMETER, static_cast<AccelData*>(&acclData));
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICE_STATUS_SENSOR_ENABLE
