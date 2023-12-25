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
namespace Cooperate {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Cooperate" };
} // namespace

Cooperate::Cooperate(IContext *env)
    : env_(env), context_(env), sm_(env)
{
    auto [sender, receiver] = Channel<CooperateEvent>::OpenChannel();
    receiver_ = receiver;
    context_.sender_ = sender;
    context_.Enable();
}

int32_t Cooperate::RegisterListener(int32_t pid)
{
    CALL_DEBUG_ENTER;
    CHKPR(env_, RET_ERR);
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
    running_.store(true);
    worker_ = std::thread(std::bind(&Cooperate::Loop, this));
    sender_.Send(CooperateEvent(CooperateEventType::ENABLE));
    return RET_OK;
}

int32_t Cooperate::Disable(int32_t pid)
{
    CALL_DEBUG_ENTER;
    sender_.Send(CooperateEvent(CooperateEventType::DISABLE));
    sender_.Send(CooperateEvent(CooperateEventType::QUIT));
    if (worker_.joinable()) {
        worker_.join();
    }
    return RET_OK;
}

int32_t Cooperate::Start(int32_t pid, int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    CALL_DEBUG_ENTER;
    StartCooperateEvent event {
        .userData = userData,
        .remoteNetworkId = remoteNetworkId,
        .startDeviceId = startDeviceId
    };
    sender_.Send(CooperateEvent(CooperateEventType::START, event));
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

void Cooperate::Loop()
{
    while (running_.load()) {
        CooperateEvent event = receiver_.Receive();
        switch (event.type) {
            case CooperateEventType::NOOP : {
                break;
            }
            case CooperateEventType::QUIT : {
                running_.store(false);
                break;
            }
            default : {
                sm_.OnEvent(context_, event);
                break;
            }
        }
    }
}

extern "C" ICooperate* CreateInstance(IContext *env)
{
    CHKPP(env);
    return new Cooperate(env);
}

extern "C" void DestroyInstance(ICooperate *instance)
{
    if (instance != nullptr) {
        delete instance;
    }
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS