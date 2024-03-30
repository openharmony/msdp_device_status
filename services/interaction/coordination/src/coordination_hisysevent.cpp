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

#include "coordination_hisysevent.h"

#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationHiSysEvent"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

std::map<CoordinationState, std::string> CoordinationDFX::coordinationState_ = {
    { CoordinationState::STATE_FREE, "STATE_FREE" },
    { CoordinationState::STATE_IN, "STATE_IN" },
    { CoordinationState::STATE_OUT, "STATE_OUT" }
};

std::map<CoorType, std::pair<std::string, std::string>> CoordinationDFX::serialStr_ = {
    { CoorType::PREPARE_SUCC, { "PREPARE_SUCCESS", "Prepare coordination successfully" } },
    { CoorType::PREPARE_FAIL, { "PREPARE_FAILED", "Prepare coordination failed" } },
    { CoorType::UNPREPARE_SUCC, { "UNPREPARE_SUCCESS", "Unprepare coordination successfully" } },
    { CoorType::UNPREPARE_FAIL, { "UNPREPARE_FAILED", "Unprepare coordination failed" } },
    { CoorType::ACTIVATE_SUCC, { "ACTIVATE_SUCCESS", "Start remote coordination successfully" } },
    { CoorType::ACTIVATE_FAIL, { "ACTIVATE_FAILED", "Start remote coordination failed" } },
    { CoorType::ACTIVATE_RESULT, { "ACTIVATE_RESULT", "Start remote accordination result failed" } },
    { CoorType::DEACTIVATE_SUCC, { "DEACTIVATE_SUCCESS", "Stop remote accordination successfully" } },
    { CoorType::DEACTIVATE_FAIL, { "DEACTIVATE_FAILED", "Stop remote accordination failed" } },
    { CoorType::DEACTIVATE_RESULT, { "DEACTIVATE_RESULT", "Stop remote accordination result failed" } },
    { CoorType::OPEN_SOFUBUS_RESULT_ONE, { "OPEN_SOFUBUS_RESULT", "Can Not Open Softbus As Init Failed" } },
    { CoorType::OPEN_SOFUBUS_RESULT_TWO, { "OPEN_SOFUBUS_RESULT", "Can Not Open Softbus As Open Session Failed" } },
    { CoorType::OPEN_SOFUBUS_RESULT_THREE, { "OPEN_SOFUBUS_RESULT", "Can Not Open Softbus As Waiting Time Out" } },
    { CoorType::INPUT_START_REMOTE_INPUT, { "D_INPUT_START_REMOTE_INPUT", "D_input start remote input failed" } },
    { CoorType::INPUT_STOP_REMOTE_O, { "D_INPUT_STOP_REMOTE_INPUT_O", "D_input stop remote input failed" } },
    { CoorType::INPUT_STOP_REMOTE_T, { "D_INPUT_STOP_REMOTE_INPUT_T", "D_input stop remote input failed" } },
    { CoorType::INPUT_PRE_REMOTE_O, { "D_INPUT_PREPARE_REMOTE_INPUT_O", "D_input prepare remote input failed" } },
    { CoorType::INPUT_PRE_REMOTE_T, { "D_INPUT_PREPARE_REMOTE_INPUT_T", "D_input prepare remote input failed" } },
    { CoorType::INPUT_UNPRE_REMOTE_O, { "D_INPUT_UNPREPARE_REMOTE_INPUT_O", "D_input unprepare remote input failed" } },
    { CoorType::INPUT_UNPRE_REMOTE_T, { "D_INPUT_UNPREPARE_REMOTE_INPUT_T", "D_input unprepare remote input failed" } },
    { CoorType::COOP_DRAG_SUCC, { "COOPERATE_DRAG_SUCCESS", "On coordination and the state change successfully" } },
    { CoorType::COOP_DRAG_FAIL, { "COOPERATE_DRAG_FAILED", "The current coordination state is out" } },
    { CoorType::COOP_DRAG_RESULT_SUCC, { "COOPERATE_DRAG_RESULT_SUCCESS", "Coordination drag result successfully" } },
    { CoorType::COOP_DRAG_RESULT_FAIL, { "COOPERATE_DRAG_RESULT_FAILED", "Coordination drag result failed" } }
};


template<typename... Types>
int32_t CoordinationDFX::WriteInputFunc(const CoorType &coorType, Types... paras)
{
    if (serialStr_.find(coorType) == serialStr_.end()) {
        FI_HILOGE("serialStr_ can't find the coordination hisysevent type");
        return RET_ERR;
    }
    auto &[label, dec] = serialStr_[coorType];
    OHOS::HiviewDFX::HiSysEvent::EventType eventType = (static_cast<uint32_t>(coorType) & 1) ?
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

int32_t CoordinationDFX::WritePrepare(int32_t monitorId, OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CoorType::PREPARE_SUCC, "Monitor_Id", monitorId, "IsOpen", true);
    }
    return WriteInputFunc(CoorType::PREPARE_FAIL, "Monitor_Id", monitorId, "IsOpen", false);
}

int32_t CoordinationDFX::WriteUnprepare(OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CoorType::UNPREPARE_SUCC, "IsClose", true);
    }
    return WriteInputFunc(CoorType::UNPREPARE_FAIL, "IsClose", false);
}

int32_t CoordinationDFX::WriteActivate(const std::string &localNetworkId, const std::string &remoteNetworkId,
    std::map<std::string, int32_t> sessionDevMap_, OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CoorType::ACTIVATE_SUCC, "localNetworkId", localNetworkId.substr(0, SUB_LEN),
            "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN));
    }
    return WriteInputFunc(CoorType::ACTIVATE_FAIL, "localNetworkId", localNetworkId.substr(0, SUB_LEN),
        "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN));
}

int32_t CoordinationDFX::WriteActivateResult(const std::string &remoteNetworkId, bool isSuccess)
{
    return WriteInputFunc(CoorType::ACTIVATE_RESULT, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN), "IsSuccess",
        isSuccess);
}

int32_t CoordinationDFX::WriteDeactivate(const std::string &remoteNetworkId,
    std::map<std::string, int32_t> sessionDevMap_, OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (sessionDevMap_.find(remoteNetworkId) == sessionDevMap_.end()) {
        FI_HILOGE("sessionDevMap_ can't find the remoteNetworkId");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevMap_[remoteNetworkId];
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CoorType::DEACTIVATE_SUCC, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
            "sessionId", sessionId);
    }
    return WriteInputFunc(CoorType::DEACTIVATE_FAIL, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
        "sessionId", sessionId);
}

int32_t CoordinationDFX::WriteDeactivateResult(const std::string &remoteNetworkId,
    std::map<std::string, int32_t> sessionDevMap_)
{
    if (sessionDevMap_.find(remoteNetworkId) == sessionDevMap_.end()) {
        FI_HILOGE("sessionDevMap_ can't find the remoteNetworkId");
        return RET_ERR;
    }
    int32_t sessionId = sessionDevMap_[remoteNetworkId];
    return WriteInputFunc(CoorType::DEACTIVATE_RESULT, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
        "sessionId", sessionId);
}

int32_t CoordinationDFX::WriteOpenSoftbusResult(const std::string &remoteNetworkId, int32_t para, int32_t type)
{
    if (type == INIT_SIGN) {
        return WriteInputFunc(CoorType::OPEN_SOFUBUS_RESULT_ONE, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
            "Initsign", para);
    } else if (type == SESS_SIGN) {
        return WriteInputFunc(CoorType::OPEN_SOFUBUS_RESULT_TWO, "remoteNetworkId", remoteNetworkId.substr(0, SUB_LEN),
            "sessionId", para);
    } else {
        return WriteInputFunc(CoorType::OPEN_SOFUBUS_RESULT_THREE, "remoteNetworkId",
            remoteNetworkId.substr(0, SUB_LEN), "status", para);
    }
}

int32_t CoordinationDFX::WriteCooperateDrag(const std::string &remoteNetworkId, CoordinationState previousSta,
    CoordinationState updateSta)
{
    if (coordinationState_.find(previousSta) == coordinationState_.end()) {
        FI_HILOGE("coordinationState_ can't find the previous coordination state");
        return RET_ERR;
    }
    if (coordinationState_.find(updateSta) == coordinationState_.end()) {
        FI_HILOGE("coordinationState_ can't find the updated coordination state");
        return RET_ERR;
    }
    std::string preState = coordinationState_[previousSta];
    std::string upState = coordinationState_[updateSta];
    return WriteInputFunc(CoorType::COOP_DRAG_SUCC, "PreviousState", preState, "UpdateState", upState);
}

int32_t CoordinationDFX::WriteCooperateDrag(const std::string &remoteNetworkId, CoordinationState currentSta)
{
    if (currentSta != CoordinationState::STATE_OUT) {
        return RET_ERR;
    }
    if (coordinationState_.find(currentSta) == coordinationState_.end()) {
        FI_HILOGE("coordinationState_ can't find the current coordination state");
        return RET_ERR;
    }
    std::string curState = coordinationState_[currentSta];
    return WriteInputFunc(CoorType::COOP_DRAG_FAIL, "CurrentState", curState);
}

int32_t CoordinationDFX::WriteCooperateDragResult(const std::string &remoteNetworkId,
    const CoordinationState &currentSta, OHOS::HiviewDFX::HiSysEvent::EventType type)
{
    if (coordinationState_.find(currentSta) == coordinationState_.end()) {
        FI_HILOGE("coordinationState_ can't find the current coordination state");
        return RET_ERR;
    }
    std::string curState = coordinationState_[currentSta];
    if (type == OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR) {
        return WriteInputFunc(CoorType::COOP_DRAG_RESULT_SUCC, "remotedeviceId", remoteNetworkId.substr(0, SUB_LEN),
            "CurCoordinationState", curState);
    }
    return WriteInputFunc(CoorType::COOP_DRAG_RESULT_FAIL, "remotedeviceId", remoteNetworkId.substr(0, SUB_LEN),
        "CurCoordinationState", curState);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
