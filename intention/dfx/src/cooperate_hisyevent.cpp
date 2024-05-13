/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cooperate_hisysevent.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "CooperateHiSysEvent"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

std::map<CooperateState, std::string> CooperateDFX::cooperateState_ = {
    { CooperateState::COOPRERATE_STATE_FREE, "STATE_FREE" },
    { CooperateState::COOPRERATE_STATE_IN, "STATE_IN" },
    { CooperateState::COOPRERATE_STATE_OUT, "STATE_OUT" }
};

std::map<CooperateType, std::pair<std::string, std::string>> CooperateDFX::serialStr_ = {
    { CooperateType::ENABLE_SUCC, { "ENABLE_SUCCESS", "Enable cooperate successfully" } },
    { CooperateType::ENABLE_FAIL, { "ENABLE_FAILED", "Enable cooperate failed" } },
    { CooperateType::DISABLE_SUCC, { "DISABLE_SUCCESS", "Disenable cooperate successfully" } },
    { CooperateType::DISABLE_FAIL, { "DISABLE_FAILED", "Disenable cooperate failed" } },
    { CooperateType::ACTIVATE_SUCC0, { "LOCAL_ACTIVATE_SUCCESS", "Local start cooperate successfully" } },
    { CooperateType::ACTIVATE_FAIL0, { "LOCAL_ACTIVATEE_FAILED", "Local start cooperate failed" } },
    { CooperateType::ACTIVATE_SUCC1, { "REMOTE_ACTIVATE_SUCCESS", "Remote start cooperate successfully" } },
    { CooperateType::ACTIVATE_FAIL1, { "REMOTE_ACTIVATE_FAILED", "Remote start cooperate failed" } },
    { CooperateType::DISACTIVATE_SUCC0, { "LOCAL_DISACTIVATE_SUCCESS", "Local stop cooperate successfully" } },
    { CooperateType::DISACTIVATE_FAIL0, { "LOCAL_DISACTIVATE_FAILED", "Local stop cooperate failed" } },
    { CooperateType::DISACTIVATE_SUCC1, { "REMOTE_DISACTIVATE_SUCCESS", "Remote stop cooperate successfully" } },
    { CooperateType::DISACTIVATE_FAIL1, { "REMOTE_DISACTIVATE_FAILED", "Remote stop cooperate failed" } },
    { CooperateType::OPENSESSION_SUCC, { "OPENSESSION_SUCCESS", "Open session successfully" } },
    { CooperateType::OPENSESSION_FAIL, { "OPENSESSION_FAILED", "Open session cooperate failed" } },
    { CooperateType::UPDATESTATE_SUCC, { "UPDATESTATE_SUCCESS", "Update cooperatestate successfully" } },
};


template<typename... Types>
int32_t CooperateDFX::WriteInputFunc(const CooperateType &cooperateType, Types... paras)
{
    if (serialStr_.find(CooperateType) == serialStr_.end()) {
        FI_HILOGE("serialStr_ can't find the cooperate hisysevent type");
        return RET_ERR;
    }
    auto &[label, dec] = serialStr_[CooperateType];
    OHOS::HiviewDFX::HiSysEvent::EventType eventType = (static_cast<uint32_t>(CooperateType) & 1) ?
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT : OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR;
    int32_t ret = HiSysEventWrite(
        OHOS::HiviewDFX::HiSysEvent::Domain::MSDP,
        label,
        eventType,
        paras...,
        "MSG",
        dec);
    if (ret != RET_OK) {
        FI_HILOGE("HiviewDFX write failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t CooperateDFX::WriteEnable(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::ENABLE_SUCC, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::ENABLE_FAIL, "IsClose", false);
}

int32_t CooperateDFX::WriteDisable(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::DISABLE_SUCC, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::DISABLE_FAIL, "IsClose", false);
}

int32_t CooperateDFX::WriteLocalStart(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::ACTIVATE_SUCC0, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::ACTIVATE_FAIL0, "IsClose", false);
}

int32_t CooperateDFX::WriteRemoteStart(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::ACTIVATE_SUCC1, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::ACTIVATE_FAIL1, "IsClose", false);
}

int32_t CooperateDFX::WriteLocalStop(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::DISACTIVATE_SUCC0, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::DISACTIVATE_FAIL0, "IsClose", false);
}

int32_t CooperateDFX::WriteRemoteStop(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::DISACTIVATE_SUCC1, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::DISACTIVATE_FAIL1, "IsClose", false);
}

int32_t CooperateDFX::WriteOpenSession(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::OPENSESSION_SUCC, "IsClose", true);
    }
    return WriteInputFunc(CooperateType::OPENSESSION_FAIL, "IsClose", false);
}

int32_t CooperateDFX::WriteDeactivate(const std::string &remoteNetworkId,
    std::map<std::string, int32_t> sessionDevMap_, OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (sessionDevMap_.find(remoteNetworkId) == sessionDevMap_.end()) {
        FI_HILOGE("sessionDevMap_ can't find the remoteNetworkId");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevMap_[remoteNetworkId];
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CooperateType::DEACTIVATE_SUCC, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
            "sessionId", sessionId);
    }
    return WriteInputFunc(CooperateType::DEACTIVATE_FAIL, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
        "sessionId", sessionId);
}

int32_t CooperateDFX::WriteDeactivateResult(const std::string &remoteNetworkId,
    std::map<std::string, int32_t> sessionDevMap_)
{
    if (sessionDevMap_.find(remoteNetworkId) == sessionDevMap_.end()) {
        FI_HILOGE("sessionDevMap_ can't find the remoteNetworkId");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevMap_[remoteNetworkId];
    return WriteInputFunc(CooperateType::DEACTIVATE_RESULT, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
        "sessionId", sessionId);
}

int32_t CooperateDFX::WriteCooperateState(CooperateState currentSta)
{
    if (currentSta != CooperateState::N_COOPERATE_STATUS) {
        return RET_ERR;
    }
    if (cooperateState_.find(currentSta) == cooperateState_.end()) {
        FI_HILOGE("cooperateState_ can't find the current cooperate state");
        return RET_ERR;
    }
    std::string curState = cooperateState_[currentSta];
    return WriteInputFunc(CooperateType::UPDATESTATE_SUCC, "CurrentState", curState);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
