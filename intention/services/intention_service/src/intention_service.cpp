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

#include "ipc_skeleton.h"

#include "devicestatus_define.h"
#include "i_plugin.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionService" };
} // namespace

IntentionService::IntentionService(IContext *context)
    : context_(context), pluginMgr_(context), socketServer_(context)
{}

int32_t IntentionService::Enable(Intention intention, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::Enable1, this,
        std::ref(context), std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Enable failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Disable(Intention intention, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::Disable1, this,
        std::ref(context), std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Disable failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Start(Intention intention, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::Start1, this,
        std::ref(context), std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Start failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Stop(Intention intention, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::Stop1, this,
        std::ref(context), std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Stop failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::AddWatch(Intention intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::AddWatch1, this,
        std::ref(context), id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("AddWatch failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::RemoveWatch(Intention intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::RemoveWatch1, this,
        std::ref(context), id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("RemoveWatch failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::SetParam(Intention intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::SetParam1, this,
        std::ref(context), id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("SetParam failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::GetParam(Intention intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::GetParam1, this,
        std::ref(context), id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("GetParam failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Control(Intention intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CallingContext context {
        .intention = intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    CHKPR(context_, RET_ERR);
    int32_t ret = context_->GetDelegateTasks().PostSyncTask(std::bind(&IntentionService::Control1, this,
        std::ref(context), id, std::ref(data), std::ref(reply)));
    if (ret != RET_OK) {
        FI_HILOGE("Control failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionService::Enable1(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->Enable(context, data, reply);
}

int32_t IntentionService::Disable1(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->Disable(context, data, reply);
}

int32_t IntentionService::Start1(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->Start(context, data, reply);
}

int32_t IntentionService::Stop1(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->Stop(context, data, reply);
}

int32_t IntentionService::AddWatch1(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->AddWatch(context, id, data, reply);
}

int32_t IntentionService::RemoveWatch1(CallingContext &context, uint32_t id,
                                       MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->RemoveWatch(context, id, data, reply);
}

int32_t IntentionService::SetParam1(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->SetParam(context, id, data, reply);
}

int32_t IntentionService::GetParam1(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->GetParam(context, id, data, reply);
}

int32_t IntentionService::Control1(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    IPlugin *plugin = LoadPlugin(context.intention);
    CHKPR(plugin, RET_ERR);
    return plugin->Control(context, id, data, reply);
}

IPlugin* IntentionService::LoadPlugin(Intention intention)
{
    CALL_DEBUG_ENTER;
    switch (intention) {
        case Intention::SOCKET: {
            return &socketServer_;
        }
        default: {
            return pluginMgr_.LoadPlugin(intention);
        }
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
