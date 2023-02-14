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

#ifndef DEVICESTATUS_MSDP_MOCK_H
#define DEVICESTATUS_MSDP_MOCK_H

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "devicestatus_data_parse.h"
#include "devicestatus_data_utils.h"
#include "devicestatus_msdp_interface.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DeviceStatusMsdpMock final : public DevicestatusMsdpInterface {
public:
    DeviceStatusMsdpMock() = default;
    ~DeviceStatusMsdpMock() = default;

    enum EventType {
        EVENT_UEVENT_FD,
        EVENT_TIMER_FD,
    };

    ErrCode Enable(DevicestatusDataUtils::DevicestatusType type) override;
    ErrCode Disable(DevicestatusDataUtils::DevicestatusType type) override;
    ErrCode DisableCount(DevicestatusDataUtils::DevicestatusType type) override;
    ErrCode RegisterCallback(std::shared_ptr<MsdpAlgorithmCallback> callback) override;
    ErrCode UnregisterCallback() override;

    bool Init();
    void InitMockStore();
    void SetTimerInterval(int32_t interval);
    void CloseTimer();
    void InitTimer();
    void TimerCallback();
    int32_t RegisterTimerCallback(const int32_t fd, const EventType et);
    void StartThread();
    void LoopingThreadEntry();
    ErrCode NotifyMsdpImpl(const DevicestatusDataUtils::DevicestatusData& data);
    void GetDeviceStatusData();
    std::shared_ptr<MsdpAlgorithmCallback> GetCallbackImpl()
    {
        std::unique_lock lock(mutex_);
        return callback_;
    }

private:
    using Callback = std::function<void(DeviceStatusMsdpMock*)>;
    std::shared_ptr<MsdpAlgorithmCallback> callback_;
    std::map<int32_t, Callback> callbacks_;
    std::unique_ptr<DeviceStatusDataParse> dataParse_;
    bool scFlag_ {true};
    int32_t timerInterval_ {-1};
    int32_t timerFd_ {-1};
    int32_t epFd_ {-1};
    std::mutex mutex_;
    static std::vector<int32_t> enabledType_;
    bool alive_ {true};
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_MSDP_MOCK_H
