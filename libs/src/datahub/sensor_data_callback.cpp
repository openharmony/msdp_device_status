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

#include "sensor_data_callback.h"

#include <cmath>
#include <cstdio>

#include "devicestatus_common.h"

namespace OHOS {
namespace Msdp {
namespace {
constexpr int32_t RATE_MILLISEC  = 100100100;
} // namespace

SensorDataCallback::~SensorDataCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    algoMap_.clear();
    alive_ = false;
    if (algorithmThread_ == nullptr) {
        DEV_HILOGE(SERVICE, "algorithmThread_ is nullptr");
        return;
    }
    if (!algorithmThread_->joinable()) {
        DEV_HILOGE(SERVICE, "thread join fail");
        return;
    }
    sem_post(&sem_);
    algorithmThread_->join();
    accelDataList_.clear();
}

void SensorDataCallback::Init()
{
    DEV_HILOGI(SERVICE, "SensorDataCallback is initiated");
    std::lock_guard<std::mutex> lock(mutex_);
    if (algorithmThread_ == nullptr) {
        DEV_HILOGE(SERVICE, "algorithmThread_ is nullptr, create");
        algorithmThread_ = std::make_unique<std::thread>(&SensorDataCallback::AlgorithmLoop, this);
    }
}

bool SensorDataCallback::Unregister()
{
    DEV_HILOGI(SERVICE, "Unregister");
    bool ret = UnregisterCallbackSensor(SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER);
    if (!ret) {
        DEV_HILOGE(SERVICE, "UnregisterCallbackSensor failed");
        return false;
    }
    alive_ = false;
    if (!algorithmThread_->joinable()) {
        DEV_HILOGE(SERVICE, "thread join fail");
        return false;
    }
    sem_post(&sem_);
    algorithmThread_->join();
    return ret;
}

bool SensorDataCallback::SubscribeSensorEvent(int32_t sensorTypeId, SensorCallback callback)
{
    DEV_HILOGI(SERVICE, "SubscribeSensorEvent");
    std::lock_guard<std::mutex> lock(mutex_);
    auto ret = algoMap_.insert(std::pair(sensorTypeId, callback));
    if (!ret.second) {
        DEV_HILOGE(SERVICE, "SensorCallback is duplicated");
        return false;
    }
    DEV_HILOGI(SERVICE, "Exit");
    return false;
}

bool SensorDataCallback::UnsubscribeSensorEvent(int32_t sensorTypeId, SensorCallback callback)
{
    DEV_HILOGI(SERVICE, "UnsubscribeSensorEvent");
    std::lock_guard<std::mutex> lock(mutex_);
    auto callbackIter = algoMap_.find(sensorTypeId);
    if (callbackIter != algoMap_.end()) {
        DEV_HILOGE(SERVICE, "erase sensorTypeId:%{public}d", sensorTypeId);
        algoMap_.erase(sensorTypeId);
    }
    DEV_HILOGI(SERVICE, "exit");
    return true;
}

bool SensorDataCallback::NotifyCallback(int32_t sensorTypeId, AccelData* data)
{
    if (data == nullptr) {
        DEV_HILOGE(SERVICE, "data is nullptr");
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = algoMap_.begin(); iter != algoMap_.end(); ++iter) {
        (iter->second)(sensorTypeId, data);
    }
    return true;
}

bool SensorDataCallback::PushData(int32_t sensorTypeId, uint8_t* data)
{
    if (data == nullptr) {
        DEV_HILOGE(SERVICE, "data is nullptr");
        return false;
    }
    AccelData* acclData = reinterpret_cast<AccelData*>(data);
    if ((abs(acclData->x) > ACC_VALID_THRHD) ||
        (abs(acclData->y) > ACC_VALID_THRHD) ||
        (abs(acclData->z) > ACC_VALID_THRHD)) {
        DEV_HILOGE(SERVICE, "Acc data is invalid");
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    accelDataList_.emplace_back(*acclData);
    sem_post(&sem_);
    return true;
}

bool SensorDataCallback::PopData(int32_t sensorTypeId, AccelData& data)
{
    if (sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        DEV_HILOGE(SERVICE, "invalid sensorTypeId:%{public}d", sensorTypeId);
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (accelDataList_.empty()) {
        DEV_HILOGD(SERVICE, "No accel data");
        return false;
    }
    data = accelDataList_.front();
    accelDataList_.pop_front();
    return true;
}

static void SensorDataCallbackImpl(SensorEvent *event)
{
    if (event == nullptr) {
        DEV_HILOGE(SERVICE, "SensorDataCallbackImpl event is nullptr");
        return;
    }
    SensorDataCallback::GetInstance().PushData(event->sensorTypeId, event->data);
}

bool SensorDataCallback::RegisterCallbackSensor(int32_t sensorTypeId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    user_.callback = SensorDataCallbackImpl;
    int32_t ret = SubscribeSensor(sensorTypeId, &user_);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "SubscribeSensor failed");
        return false;
    }
    ret = SetBatch(sensorTypeId, &user_, RATE_MILLISEC, RATE_MILLISEC);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "SetBatch failed");
        return false;
    }
    ret = ActivateSensor(sensorTypeId, &user_);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "ActivateSensor failed");
        return false;
    }
    return true;
}

bool SensorDataCallback::UnregisterCallbackSensor(int32_t sensorTypeId)
{
    DEV_HILOGI(SERVICE, "enter");
    std::lock_guard<std::mutex> lock(mutex_);
    if (user_.callback == nullptr) {
        DEV_HILOGE(SERVICE, "callback is nullptr");
        return false;
    }
    int32_t ret = DeactivateSensor(sensorTypeId, &user_);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "DeactivateSensor failed");
        return false;
    }
    ret = UnsubscribeSensor(sensorTypeId, &user_);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "UnsubscribeSensor failed");
        return false;
    }
    return true;
}

void SensorDataCallback::AlgorithmLoop()
{
    DEV_HILOGI(SERVICE, "enter");
    while (alive_) {
        sem_wait(&sem_);
        HandleSensorEvent();
    }
    DEV_HILOGI(SERVICE, "exit");
}

void SensorDataCallback::HandleSensorEvent()
{
    DEV_HILOGI(SERVICE, "enter");
    AccelData acclData;
    if (PopData(SENSOR_TYPE_ID_ACCELEROMETER, acclData)) {
        NotifyCallback(SENSOR_TYPE_ID_ACCELEROMETER, static_cast<AccelData*>(&acclData));
    }
}
} // namespace Msdp
} // namespace OHOS
