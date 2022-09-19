      
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

#ifndef SENSOR_DATA_CALLBACK_H
#define SENSOR_DATA_CALLBACK_H

#include <list>
#include <mutex>
#include <thread>
#include "devicestatus_data_define.h"
#include "sensor_agent.h"

constexpr bool PS_VALUE = true;
constexpr float ALS_VALUE = 5;

namespace OHOS {
namespace Msdp {
class SensorDataCallback {
public:
    static std::shared_ptr<SensorDataCallback> GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = std::shared_ptr<SensorDataCallback>(new SensorDataCallback());
        }
        return instance_;
    }
    static void ReleaseInstance()
    {
        instance_ = nullptr;
    }
    ~SensorDataCallback();
    bool RegisterCallbackSensor(int32_t sensorTypeId);
    bool Initiate();
    void SubscribeSensorEvent(SensorCallback callback);
    void PushData(int32_t sensorTypeId, uint8_t* data);
    bool PopData(int32_t sensorTypeId, float& data);
    bool PopData(int32_t sensorTypeId, AccelData& data);
    void AlgorithmLoop();
    void HandleSensorEvent();
    void NotifyCallback(int32_t sensorTypeId, void* data);
private:
    static std::shared_ptr<SensorDataCallback> instance_;
    SensorDataCallback();
    SensorUser user_;
    std::list<SensorCallback> callbackList_;
    std::list<AccelData> accelDataList_;
    std::list<float> ambientLightDataList_;
    std::list<float> proximityDataList_;

    std::unique_ptr<std::thread> algorithmThread_;
    sem_t sem_;
    std::mutex callbackMutex_;
    std::mutex dataMutex_;
    bool alive_;
};
} // namespace Msdp
} // namespace OHOS
#endif // SENSOR_DATA_CALLBACK_H

    