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

#ifndef INTENTION_SERVICE_H
#define INTENTION_SERVICE_H

#include <iremote_object.h>
#include <system_ability.h>

#include "devicestatus_delayed_sp_singleton.h"
#include "i_context.h"
#include "intention_stub.h"
#include "plugin_manager.h"
#include "task_scheduler.h"
#include "timer_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IntentionService final : public IContext,
                               public SystemAbility,
                               public IntentionStub {
    DECLARE_SYSTEM_ABILITY(IntentionService);
    DECLARE_DELAYED_SP_SINGLETON(IntentionService);

public:
    virtual void OnDump() override;
    virtual void OnStart() override;
    virtual void OnStop() override;

    ITaskScheduler& GetTaskScheduler() override;
    ITimerManager& GetTimerManager() override;
    IPluginManager& GetPluginManager() override;

private:
    int32_t Enable(uint32_t intention, MessageParcel &data, MessageParcel &reply) override;
    int32_t Disable(uint32_t intention, MessageParcel &data, MessageParcel &reply) override;
    int32_t Start(uint32_t intention, MessageParcel &data, MessageParcel &reply) override;
    int32_t Stop(uint32_t intention, MessageParcel &data, MessageParcel &reply) override;
    int32_t AddWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply) override;
    int32_t RemoveWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply) override;
    int32_t SetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply) override;
    int32_t GetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply) override;
    int32_t Control(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply) override;

private:
    TaskScheduler taskScheduler_;
    TimerManager timerMgr_;
    PluginManager pluginMgr_;
};

ITaskScheduler& IntentionService::GetTaskScheduler()
{
    return taskScheduler_;
}

ITimerManager& IntentionService::GetTimerManager()
{
    return timerMgr_;
}

IPluginManager& IntentionService::GetPluginManager()
{
    return pluginMgr_;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // INTENTION_SERVICE_H
