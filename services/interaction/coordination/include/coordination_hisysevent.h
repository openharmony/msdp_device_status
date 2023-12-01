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

#ifndef COORDINATION_HISYSEVENT_H
#define COORDINATION_HISYSEVENT_H

#include <map>
#include <string>

#include "coordination_sm.h"
#include "devicestatus_define.h"
#include "hisysevent.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t SUB_LEN { 6 };
constexpr int32_t INIT_SIGN { 0 };
constexpr int32_t SESS_SIGN { 1 };
constexpr int32_t STATUS_SIGN { 2 };
} //namespace
enum CoorType : int32_t {
    PREPARE_SUCC = 0,
    PREPARE_FAIL = 1,
    UNPREPARE_SUCC = 2,
    UNPREPARE_FAIL = 3,
    ACTIVATE_SUCC = 4,
    ACTIVATE_FAIL = 5,
    ACTIVATE_RESULT = 7,
    DEACTIVATE_SUCC = 8,
    DEACTIVATE_FAIL = 9,
    DEACTIVATE_RESULT = 11,
    OPEN_SOFUBUS_RESULT_ONE = 13,
    OPEN_SOFUBUS_RESULT_TWO = 15,
    OPEN_SOFUBUS_RESULT_THREE = 17,
    INPUT_START_REMOTE_INPUT = 19,
    INPUT_STOP_REMOTE_O = 21,
    INPUT_STOP_REMOTE_T = 23,
    INPUT_PRE_REMOTE_O = 25,
    INPUT_PRE_REMOTE_T = 27,
    INPUT_UNPRE_REMOTE_O = 29,
    INPUT_UNPRE_REMOTE_T = 31,
    COOP_DRAG_SUCC = 32,
    COOP_DRAG_FAIL = 33,
    COOP_DRAG_RESULT_SUCC = 34,
    COOP_DRAG_RESULT_FAIL = 35,
};

class CoordinationDFX {
public:
    static int32_t WritePrepare(int32_t monitorId, OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteUnprepare(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteActivate(const std::string &localNetworkId, const std::string &remoteNetworkId,
        std::map<std::string, int32_t> sessionDevMap_, OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteActivateResult(const std::string &remoteNetworkId, bool isSuccess);
    static int32_t WriteDeactivate(const std::string &remoteNetworkId, std::map<std::string, int32_t> sessionDevMap_,
        OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteDeactivateResult(const std::string &remoteNetworkId,
        std::map<std::string, int32_t> sessionDevMap_);
    static int32_t WriteOpenSoftbusResult(const std::string &remoteNetworkId, int32_t para, int32_t type);
    static int32_t WriteCooperateDrag(const std::string &remoteNetworkId, CoordinationState currentSta);
    static int32_t WriteCooperateDrag(const std::string &remoteNetworkId, CoordinationState previousSta,
        CoordinationState updateSta);
    static int32_t WriteCooperateDragResult(const std::string &remoteNetworkId, const CoordinationState &currentSta,
        OHOS::HiviewDFX::HiSysEvent::EventType type);
    template<typename... Types>
    static int32_t WriteInputFunc(const CoorType &coorType, Types... paras);

private:
    static std::map<CoordinationState, std::string> coordinationState_;
    static std::map<CoorType, std::pair<std::string, std::string>> serialStr_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DEVICESTATUS_HISYSEVENT_H
