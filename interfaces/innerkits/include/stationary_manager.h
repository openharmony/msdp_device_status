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

#ifndef STATIONARY_MANAGER_H
#define STATIONARY_MANAGER_H

#include <functional>
#include <memory>

#include "nocopyable.h"

#include "devicestatus_common.h"
#include "stationary_callback.h"
#include "stationary_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class StationaryManager {
public:

    /**
    * @brief 获取StationaryManager实例。
    * @return StationaryManager实例。
    * @since 9
    */
    static StationaryManager *GetInstance();

    /**
     * @brief 订阅设备状态。
     * @param type 设备状态类型。
     * @param event 订阅的事件（进入/退出/进入和退出）。
     * @param latency 上报周期。
     * @param callback 用于接收设备状态事件变化的回调。
     * @return 返回0表示接口调用成功，返回其他表示接口调用失败。
     * @since 9
     */
    int32_t SubscribeCallback(Type type, ActivityEvent event, ReportLatencyNs latency,
        sptr<IRemoteDevStaCallback> callback);

    /**
     * @brief 取消订阅设备状态。
     * @param type 设备状态类型。
     * @param event 订阅的事件（进入/退出/进入和退出）。
     * @param callback 用于接收设备状态事件变化的回调。
     * @return 返回0表示接口调用成功，返回其他表示接口调用失败。
     * @since 9
     */
    int32_t UnsubscribeCallback(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback);

    /**
     * @brief 获取当前设备状态数据。
     * @param type 设备状态类型。
     * @return 设备状态数据。
     * @since 9
     */
    Data GetDeviceStatusData(const Type type);
private:
    StationaryManager() = default;
    DISALLOW_COPY_AND_MOVE(StationaryManager);
    static StationaryManager *instance_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#define StationaryMgr OHOS::Msdp::DeviceStatus::StationaryManager::GetInstance()

#endif // STATIONARY_MANAGER_H