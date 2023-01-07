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
#include "devicestatus_func_callback.h"
#ifdef OHOS_BUILD_ENABLE_COORDINATION
#include "coordination_manager_impl.h"
#endif // OHOS_BUILD_ENABLE_COORDINATION
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
#ifdef OHOS_BUILD_ENABLE_COORDINATION
        {MessageId::COORDINATION_ADD_LISTENER, MsgCallbackBind2(&ClientMsgHandler::OnCoordinationListener, this)},
        {MessageId::COORDINATION_MESSAGE, MsgCallbackBind2(&ClientMsgHandler::OnCoordinationMessage, this)},
        {MessageId::COORDINATION_GET_STATE, MsgCallbackBind2(&ClientMsgHandler::OnCoordinationState, this)},
#endif // OHOS_BUILD_ENABLE_COORDINATION
    };
    for (auto &it : funs) {
        if (!RegistrationEvent(it)) {
            FI_HILOGW("Failed to register event errCode:%{public}d", EVENT_REG_FAIL);
            continue;
        }
    }
}

void ClientMsgHandler::OnMsgHandler(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
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

#ifdef OHOS_BUILD_ENABLE_COORDINATION
int32_t ClientMsgHandler::OnCoordinationListener(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    std::string deviceId;
    int32_t nType;
    pkt >> userData >> deviceId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read type failed");
        return RET_ERR;
    }
    CoordinationMgrImpl.OnDevCoordinationListener(deviceId, CoordinationMessage(nType));
    return RET_OK;
}

int32_t ClientMsgHandler::OnCoordinationMessage(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    std::string deviceId;
    int32_t nType;
    pkt >> userData >> deviceId >> nType;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read coordination msg failed");
        return RET_ERR;
    }
    CoordinationMgrImpl.OnCoordinationMessageEvent(userData, deviceId, CoordinationMessage(nType));
    return RET_OK;
}

int32_t ClientMsgHandler::OnCoordinationState(const StreamClient& client, NetPacket& pkt)
{
    CALL_DEBUG_ENTER;
    int32_t userData;
    bool state;
    pkt >> userData >> state;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet read coordination msg failed");
        return RET_ERR;
    }
    CoordinationMgrImpl.OnCoordinationState(userData, state);
    return RET_OK;
}
#endif // OHOS_BUILD_ENABLE_COORDINATION
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
