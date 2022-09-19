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

#include "sensor_data_callback.h"
#include "devicestatus_common.h"
#include <cstdio>

namespace OHOS {
namespace Msdp {

std::shared_ptr<SensorDataCallback> SensorDataCallback::instance_ = nullptr;

SensorDataCallback::SensorDataCallback()
{
    DEV_HILOGI(SERVICE, "SensorDataCallback is created");
    alive_ = true;
}

SensorDataCallback::~SensorDataCallback()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    std::lock_guard cbLock(callbackMutex_);
    std::lock_guard dataLock(dataMutex_);
    callbackList_.clear();
    alive_ = false;
    algorithmThread_->join();
    accelDataList_.clear();
    ambientLightDataList_.clear();
    proximityDataList_.clear();
}

bool SensorDataCallback::Initiate()
{
    DEV_HILOGI(SERVICE, "SensorDataCallback is initiated");

    RegisterCallbackSensor(SensorTypeId::SENSOR_TYPE_ID_ACCELEROMETER);
    RegisterCallbackSensor(SensorTypeId::SENSOR_TYPE_ID_AMBIENT_LIGHT);
    RegisterCallbackSensor(SensorTypeId::SENSOR_TYPE_ID_PROXIMITY);
    algorithmThread_ = std::make_unique<std::thread>(&SensorDataCallback::AlgorithmLoop, this);
    return true;
}

void SensorDataCallback::SubscribeSensorEvent(SensorCallback callback)
{
    std::lock_guard lock(callbackMutex_);
    DEV_HILOGI(SERVICE, "SubscribeSensorEvent");
    callbackList_.push_back(callback);
}

void SensorDataCallback::NotifyCallback(int32_t sensorTypeId, void* data)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(callbackMutex_);
    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); callback++) {
        (*callback)(sensorTypeId, data);
    }
}

void SensorDataCallback::PushData(int32_t sensorTypeId, uint8_t* data)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(dataMutex_);
    DEV_HILOGI(SERVICE, "PushData sensorTypeId: %{public}d", sensorTypeId);
    switch (sensorTypeId) {
        case SENSOR_TYPE_ID_ACCELEROMETER: {
            AccelData* ad = reinterpret_cast<AccelData*>(data);
            accelDataList_.emplace_back(*ad);
            DEV_HILOGI(SERVICE,
                "ACCEL PushData: x=%{public}f, y=%{public}f, z=%{public}f",
                ad->x, ad->y, ad->z);
            break;
        }
        case SENSOR_TYPE_ID_AMBIENT_LIGHT: {
            AmbientLightData* al = reinterpret_cast<AmbientLightData*>(data);
            ambientLightDataList_.push_back(al->intensity);
            DEV_HILOGI(SERVICE, "AL PushData: %{public}f", al->intensity);
            break;
        }
        case SENSOR_TYPE_ID_PROXIMITY: {
            ProximityData* p = reinterpret_cast<ProximityData*>(data);
            proximityDataList_.push_back(p->distance);
            DEV_HILOGI(SERVICE, "PRO PushData: %{public}f", p->distance);
            break;
        }
        default:
            DEV_HILOGI(SERVICE, "wrong sensorTypeId: %{public}d", sensorTypeId);
            return;
    }
    sem_post(&sem_);
}

bool SensorDataCallback::PopData(int32_t sensorTypeId, float& data)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(dataMutex_);
    switch (sensorTypeId) {
        case SENSOR_TYPE_ID_AMBIENT_LIGHT: {
            if (ambientLightDataList_.empty()) {
                DEV_HILOGI(SERVICE, "No Ambient Light Data");
                return false;
            }
            data = ambientLightDataList_.front();
            ambientLightDataList_.pop_front();
            DEV_HILOGI(SERVICE, "AL PopData: %{public}f", data);
            return true;
        }
        case SENSOR_TYPE_ID_PROXIMITY: {
            if (proximityDataList_.empty()) {
                DEV_HILOGI(SERVICE, "No Proximity Data");
                return false;
            }
            data = proximityDataList_.front();
            proximityDataList_.pop_front();
            DEV_HILOGI(SERVICE, "PRO PopData: %{public}f", data);
            return true;
        }
        default:
            break;
    }
    DEV_HILOGI(SERVICE, "wrong sensorTypeId: %{public}d", sensorTypeId);
    return false;
}

bool SensorDataCallback::PopData(int32_t sensorTypeId, AccelData& data)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    std::lock_guard lock(dataMutex_);
    DEV_HILOGI(SERVICE, "PopData sensorTypeId: %{public}d", sensorTypeId);
    if (sensorTypeId != SENSOR_TYPE_ID_ACCELEROMETER) {
        DEV_HILOGI(SERVICE, "wrong sensorTypeId: %{public}d", sensorTypeId);
        return false;
    }
    if (accelDataList_.empty()) {
        DEV_HILOGI(SERVICE, "No Accel Data");
        return false;
    }
    data = accelDataList_.front();
    accelDataList_.pop_front();
    DEV_HILOGI(SERVICE, "ACCEL PopData: x=%{public}f, y=%{public}f, z=%{public}f",
        data.x, data.y, data.z);
    return true;
}

static void SensorDataCallbackImpl(SensorEvent *event)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    if (event == nullptr) {
        DEV_HILOGE(SERVICE, "SensorDataCallbackImpl event is null");
        return;
    }
    DEV_HILOGI(SERVICE, "SensorDataCallbackImpl sensorTypeId: %{public}d",
        event->sensorTypeId);
    std::shared_ptr<SensorDataCallback> instance = SensorDataCallback::GetInstance();
    instance->PushData(event->sensorTypeId, event->data);
}

bool SensorDataCallback::RegisterCallbackSensor(int32_t sensorTypeId)
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    user_.callback = SensorDataCallbackImpl; 
    int32_t ret = SubscribeSensor(sensorTypeId, &user_);
    if (ret != 0) {
        DEV_HILOGE(SERVICE, "SubscribeSensor failed");
        return false;
    } 
    ret = SetBatch(sensorTypeId, &user_, 100000000, 100000000);
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

void SensorDataCallback::AlgorithmLoop()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    while (alive_) {
        sem_wait(&sem_);
        HandleSensorEvent();
    }
}

void SensorDataCallback::HandleSensorEvent()
{
    DEV_HILOGI(SERVICE, "%{public}s enter", __func__);
    AccelData ad;
    bool exist = PopData(SENSOR_TYPE_ID_ACCELEROMETER, ad);
    if (exist) {
        NotifyCallback(SENSOR_TYPE_ID_ACCELEROMETER, (void*)(&ad));
    }
    float ambientLight;
    exist = PopData(SENSOR_TYPE_ID_AMBIENT_LIGHT, ambientLight);
    if (exist) {
        NotifyCallback(SENSOR_TYPE_ID_AMBIENT_LIGHT, (void*)(&ambientLight));
    }
    float proximty;
    exist = PopData(SENSOR_TYPE_ID_PROXIMITY, proximty);
    if (exist) {
        NotifyCallback(SENSOR_TYPE_ID_PROXIMITY, (void*)(&proximty));
    }
}
} // namespace Msdp
} // namespace OHOS