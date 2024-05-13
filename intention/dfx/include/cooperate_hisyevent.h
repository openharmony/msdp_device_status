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

#ifndef COOPERATE_HISYSEVENT_H
#define COOPERATE_HISYSEVENT_H

#include <map>
#include <string>

#include "devicestatus_define.h"
#include "hisysevent.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum CooperateType : int32_t {
    ENABLE_SUCC = 0,
    ENABLE_FAIL = 1,
    DISENABLE_SUCC = 2,
    DISENABLE_FAIL = 3,
    LOCAL_ACTIVATE_SUCC = 4,
    LOCAL_ACTIVATE_FAIL = 5,
    REMOTE_ACTIVATE_SUCC = 6,
    REMOTE_ACTIVATE_FAIL = 7,
    LOCAL_DISACTIVATE_SUCC = 8,
    LOCAL_DISACTIVATE_FAIL = 9,
    REMOTE_DISACTIVATE_SUCC = 10,
    REMOTE_DISACTIVATE_FAIL = 11,
    OPENSESSION_SUCC = 12,
    OPENSESSION_FAIL = 13,
    DEACTIVATE_SUCC = 14,
    DEACTIVATE_FAIL = 15,
    DEACTIVATE_RESULT = 16,
    UPDATESTATE_SUCC = 17,
    START_SUCC = 18,
    START_FAIL = 19,
    STOP_SUCC = 20,
    STOP_FAIL = 21,
};
enum CooperateState : size_t {
    COOPRERATE_STATE_FREE = 0,
    COOPRERATE_STATE_OUT,
    COOPRERATE_STATE_IN,
    N_COOPERATE_STATUS,
};

class CooperateDFX {
public:

    static int32_t WriteEnable(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteDisable(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteLocalStart(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteLocalStop(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteRemoteStart(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteRemoteStop(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteOpenSession(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteStart(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteStop(OHOS::HiviewDFX::HiSysEvent::EventType type);
    static int32_t WriteCooperateState(CooperateState currentSta);
    template<typename... Types>
    static int32_t WriteInputFunc(const CooperateType &cooperateType, Types... paras);

private:
    static std::map<CooperateState, std::string> CooperateState_;
    static std::map<CooperaeType, std::pair<std::string, std::string>> serialStr_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_HISYSEVENT_H
