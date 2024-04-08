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

#ifndef COOPERATE_MOUSE_LOCATION_H
#define COOPERATE_MOUSE_LOCATION_H

#include <set>
#include <unordered_map>

#include "nocopyable.h"
#include "pointer_event.h"

#include "cooperate_events.h"
#include "i_context.h"
#include "i_event_listener.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {
class MouseLocation {

struct LocationInfo {
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t displayWidth { -1 };
    int32_t displayHeight { -1 };
};

public:
    MouseLocation(IContext *env) : env_(env) {}
    ~MouseLocation() = default;
    DISALLOW_COPY_AND_MOVE(MouseLocation);
    void AddListener(const RegisterEventListenerEvent &event);
    void RemoveListener(const UnregisterEventListenerEvent &event);

    void OnSubscribeMouseLocation(const DSoftbusSubscribeMouseLocation &notice);
    void OnUnSubscribeMouseLocation(const DSoftbusUnSubscribeMouseLocation &notice);
    void OnRelaySubscribeMouseLocation(const DSoftbusRelaySubscribeMouseLocation &notice);
    void OnRelayUnSubscribeMouseLocation(const DSoftbusRelayUnSubscribeMouseLocation &notice);
    void OnRemoteMouseLocation(const DSoftbusSyncMouseLocation &notice);

    void ProcessData(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void SyncLocationToRemote(const std::string &networkId, const LocationInfo &locationInfo);

private:
    int32_t SubscribeMouseLocation(const std::string &networkId, const DSoftbusSubscribeMouseLocation &event);
    int32_t UnSubscribeMouseLocation(const std::string &networkId, const DSoftbusUnSubscribeMouseLocation &event);
    int32_t SyncMouseLocation(const std::string &networkId, const DSoftbusSyncMouseLocation &event);
    int32_t RelaySubscribeMouseLocation(const std::string &networkId, const DSoftbusRelaySubscribeMouseLocation &event);
    int32_t RelayUnSubscribeMouseLocation(const std::string &networkId,
        const DSoftbusRelayUnSubscribeMouseLocation &event);
    void ReportMouseLocationToListener(const std::string &networkId, const LocationInfo &locationInfo, int32_t pid);
    void TransferToLocationInfo(std::shared_ptr<MMI::PointerEvent> pointerEvent, LocationInfo &locationInfo);
    bool HasRemoteSubscriber();
    bool HasLocalListener();

private:
    IContext *env_ { nullptr };
    std::string localNetworkId_;
    std::set<int32_t> localListeners_;
    std::set<std::string> remoteSubscribers_;
    std::unordered_map<std::string, std::set<int32_t>> listeners_;
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_MOUSE_LOCATION_H
