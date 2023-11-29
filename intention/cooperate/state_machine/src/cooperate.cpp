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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Cooperate" };
} // namespace
int32_t Cooperate::Init()
{
    CALL_DEBUG_ENTER;
    auto [sender, receiver] = Channel<Cooperate>::OpenChannel();
    sender_ = sender;
    receiver_ = receiver;
    return sm_.Init(sender);
}

int32_t Cooperate::Enable()
{
    running_.store(true);
    worker_ = std::thread(std::bind(&Cooperate::Loop, this));
    FI_HILOGD("enable cooperate");
    sender_.Send(CooperateEvent(CooperateEventType::ENABLE));
    return RET_OK;
}

void Cooperate::Disable()
{
    sender_.Send(CooperateEvent(CooperateEventType::DISABLE));
    sender_.Send(CooperateEvent(CooperateEventType::QUIT));
    if (worker_.joinable()) {
        worker_.join();
    }
}

int32_t Cooperate::StartCooperate(int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId)
{
    StartCooperateEvent event {
        .userData = userData,
        .remoteNetworkId = remoteNetworkId,
        .startDeviceId = startDeviceId
    };
    sender_.Send(CooperateEvent(CooperateEventType::START, event));

    std::thread([this, &remoteNetworkId, startDeviceId] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sender_.Send(CooperateEvent(CooperateEventType::PREPARE_DINPUT_RESULT,
            StartRemoteInputResult {
                .source = "local",
                .sink = remoteNetworkId,
                .startDeviceId = startDeviceId,
                .success = true }));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sender_.Send(CooperateEvent(CooperateEventType::START_DINPUT_RESULT,
            StartRemoteInputResult {
                .source = "local",
                .sink = remoteNetworkId,
                .startDeviceId = startDeviceId,
                .success = true }));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        sender_.Send(CooperateEvent(CooperateEventType::POINTER_MOVE,
            PointerMoveEvent {
                .deviceId = 13,
            }
        ));
    }).detach();
    return RET_OK;
}

int32_t Cooperate::StopCooperate(int32_t userData, bool isUnchained)
{
    return RET_ERR;
}

void Cooperate::Loop()
{
    while (running_.load()) {
        CooperateEvent event = receiver_.Receive();
        switch (event.type) {
            case CooperateEventType::NOOP : {
                FI_HILOGD("noop");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                break;
            }
            case CooperateEventType::QUIT : {
                FI_HILOGD("quit");
                running_.store(false);
                break;
            }
            default : {
                sm_.OnEvent(event);
                break;
            }
        }
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS