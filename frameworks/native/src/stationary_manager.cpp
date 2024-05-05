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

#include "stationary_manager.h"

#include "include/util.h"

#include "devicestatus_client.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
#include "intention_manager.h"
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

StationaryManager *StationaryManager::instance_ = new (std::nothrow) StationaryManager();

StationaryManager *StationaryManager::GetInstance()
{
    return instance_;
}

int32_t StationaryManager::SubscribeCallback(Type type, ActivityEvent event, ReportLatencyNs latency,
    sptr<IRemoteDevStaCallback> callback)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.SubscribeCallback(type, event, latency, callback);
#else
    return DeviceStatusClient::GetInstance().SubscribeCallback(type, event, latency, callback);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

int32_t StationaryManager::UnsubscribeCallback(Type type, ActivityEvent event, sptr<IRemoteDevStaCallback> callback)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.UnsubscribeCallback(type, event, callback);
#else
    return DeviceStatusClient::GetInstance().UnsubscribeCallback(type, event, callback);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}

Data StationaryManager::GetDeviceStatusData(Type type)
{
#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    return INTER_MGR_IMPL.GetDeviceStatusData(type);
#else
    return DeviceStatusClient::GetInstance().GetDeviceStatusData(type);
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS