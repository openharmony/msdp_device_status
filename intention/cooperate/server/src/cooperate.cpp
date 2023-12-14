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

#include "cooperate.h"

#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Cooperate" };
} // namespace

Cooperate::Cooperate(IContext *context)
    : context_(context)
{}

int32_t Cooperate::RegisterListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::UnregisterListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::RegisterHotAreaListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::UnregisterHotAreaListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::Enable(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::Disable(int32_t pid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::Start(int32_t pid, int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    CHKPR(context_, RET_ERR);
    return RET_ERR;
}

int32_t Cooperate::Stop(int32_t pid, int32_t userData, bool isUnchained)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t Cooperate::GetCooperateState(int32_t pid, int32_t userData, const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

void Cooperate::Dump(int32_t fd)
{
    CALL_DEBUG_ENTER;
}

extern "C" ICooperate* CreateInstance(IContext *context)
{
    return new (std::nothrow) Cooperate(context);
}

extern "C" void DestroyInstance(ICooperate *instance)
{
    if (instance != nullptr) {
        delete instance;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS