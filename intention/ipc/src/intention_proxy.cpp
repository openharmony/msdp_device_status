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

#include "intention_proxy.h"

#include "hitrace_meter.h"
#include "iremote_object.h"
#include <message_option.h>
#include <message_parcel.h>

#include "fi_log.h"
#include "i_plugin.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionProxy" };
} // namespace

int32_t IntentionProxy::Enable(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::ENABLE, intention, 0u)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::Disable(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::DISABLE, intention, 0u)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::Start(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::START, intention, 0u)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::Stop(uint32_t intention, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::STOP, intention, 0u)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::AddWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::ADD_WATCH, intention, id)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::RemoveWatch(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::REMOVE_WATCH, intention, id)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::SetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::SET_PARAM, intention, id)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::GetParam(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::GET_PARAM, intention, id)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t IntentionProxy::Control(uint32_t intention, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        return RET_ERR;
    }
    MessageOption option;

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(
        PARAMID(CommonAction::CONTROL, intention, id)),
        data, reply, option);
    if (ret != RET_OK) {
        FI_HILOGE("SendRequest is failed, ret:%{public}d", ret);
    }
    return ret;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
