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

#include "nocopyable.h"
#include "i_event_listener.h"
#include "pointer_event.h"

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

    bool Marshalling();
    bool Unmarshalling();
};

public:
    DISALLOW_COPY_AND_MOVE(MouseLocation);
    void AddListener(const RegisterEventListenerEvent &event);
    void RemoveListener(const UnregisterEventListenerEvent &event);
    void OnPointerEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    int32_t SyncLocationToRemote(const std::string &networkId, const LocationInfo &locationInfo);
private:
    void OnMouseLocationMessage();
    void NotifySubscriber();
private:
    IContext *env_ { nullptr };
};
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_MOUSE_LOCATION_H
