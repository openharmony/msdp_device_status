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

#include "client_msg_handler.h"

#include <cinttypes>
#include <iostream>
#include <sstream>

#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "devicestatus_func_callback.h"
#include "time_cost_chk.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "ClientMsgHandler" };
} // namespace

void ClientMsgHandler::Init()
{
    MsgCallback funs[] = {
        {MmiMessageId::COOPERATION_ADD_LISTENER, MsgCallbackBind2(&ClientMsgHandler::OnCooperationListener, this)},
        {MmiMessageId::COOPERATION_MESSAGE, MsgCallbackBind2(&ClientMsgHandler::OnCooperationMessage, this)},
        {MmiMessageId::COOPERATION_GET_STATE, MsgCallbackBind2(&ClientMsgHandler::OnCooperationState, this)},
    };
    for (auto &it : funs) {
        if (!RegistrationEvent(it)) {
            FI_HILOGW("Failed to register event errCode:%{public}d", EVENT_REG_FAIL);
            continue;
        }
    }
}

void ClientMsgHandler::InitProcessedCallback()
{
}

void ClientMsgHandler::OnMsgHandler(const UDSClient& client, NetPacket& pkt)
{
    auto id = pkt.GetMsgId();
    TimeCostChk chk("ClientMsgHandler::OnMsgHandler", "overtime 300(us)", MAX_OVER_TIME, id);
    auto callback = GetMsgCallback(id);
    if (callback == nullptr) {
        FI_HILOGE("Unknown msg id:%{public}d", id);
        return;
    }
    auto ret = (*callback)(client, pkt);
    if (ret < 0) {
        FI_HILOGE("Msg handling failed. id:%{public}d,ret:%{public}d", id, ret);
        return;
    }
}

void ClientMsgHandler::OnDispatchEventProcessed(int32_t eventId)
{
}

int32_t ClientMsgHandler::OnCooperationListener(const UDSClient& client, NetPacket& pkt)
{
    return RET_OK;
}

int32_t ClientMsgHandler::OnCooperationMessage(const UDSClient& client, NetPacket& pkt)
{
    return RET_OK;
}

int32_t ClientMsgHandler::OnCooperationState(const UDSClient& client, NetPacket& pkt)
{
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
