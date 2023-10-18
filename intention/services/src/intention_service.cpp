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

#include "intention_service.h"

#include <unistd.h>
#include <vector>

#include "if_system_ability_manager.h"
#include <ipc_skeleton.h>
#include "system_ability_definition.h"

#include "devicestatus_common.h"
#include "i_plugin.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionService" };
} // namespace

IntentionService::IntentionService() : SystemAbility(MSDP_DEVICESTATUS_SERVICE_ID, true)
{}

IntentionService::~IntentionService()
{}

void IntentionService::OnDump()
{}

void IntentionService::OnStart()
{
    CALL_INFO_TRACE;
    if (!Publish(DelayedSpSingleton<IntentionService>::GetInstance())) {
        FI_HILOGE("On start register to system ability manager failed");
    }
}

void IntentionService::OnStop()
{
    CALL_INFO_TRACE;
}

int32_t IntentionService::Enable(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::Enable, m, context, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Enable failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Disable(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::Disable, m, context, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Disable failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Start(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::Start, m, context, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Start failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Stop(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::Stop, m, context, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Stop failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::AddWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::AddWatch, m, context, id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("AddWatch failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::RemoveWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::RemoveWatch, m, context, id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("RemoveWatch failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::SetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::SetParam, m, context, id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("SetParam failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::GetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::GetParam, m, context, id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("GetParam failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Control(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    IPlugin *m = pluginMgr_.LoadPlugin(static_cast<Intention>(intention));
    CHKPR(m, RET_ERR);
    CallingContext context;

    int32_t ret = taskScheduler_.PostSyncTask(
        std::bind(&IPlugin::Control, m, context, id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Control failed, ret:%{public}d", ret);
    }
    return ret;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
