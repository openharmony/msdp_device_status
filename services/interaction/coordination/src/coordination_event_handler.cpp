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

#include "coordination_event_handler.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationEventHandler"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CoordinationEventHandler::CoordinationEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner) {}

bool CoordinationEventHandler::ProxyPostTask(const Callback &callback, int64_t delayTime)
{
    CALL_DEBUG_ENTER;
    return AppExecFwk::EventHandler::PostTask(callback, delayTime);
}

bool CoordinationEventHandler::ProxyPostTask(const Callback &callback, const std::string &name,
    int64_t delayTime)
{
    CALL_DEBUG_ENTER;
    return AppExecFwk::EventHandler::PostTask(callback, name, delayTime);
}

void CoordinationEventHandler::ProxyRemoveTask(const std::string &name)
{
    CALL_DEBUG_ENTER;
    AppExecFwk::EventHandler::RemoveTask(name);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
