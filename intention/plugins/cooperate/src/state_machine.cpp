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

#include "state_machine.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "StateMachine" };
} // namespace

void StateMachine::EnableCooperate()
{
    CALL_INFO_TRACE;
}

void StateMachine::DisableCooperate()
{
    CALL_INFO_TRACE;
}

int32_t StateMachine::StartCooperate(const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_INFO_TRACE;
    return RET_ERR;
}

int32_t StateMachine::StopCooperate(bool isUnchained)
{
    CALL_INFO_TRACE;
    return RET_ERR;
}

int32_t StateMachine::GetCooperateState(const std::string &networkId)
{
    CALL_INFO_TRACE;
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
