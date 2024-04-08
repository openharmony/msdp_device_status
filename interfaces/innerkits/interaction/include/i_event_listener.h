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

#ifndef I_LOCATION_LISTENER_H
#define I_LOCATION_LISTENER_H

namespace OHOS {
namespace Msdp {

struct Event {
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t displayWidth { -1 };
    int32_t displayHeight { -1 };
};

class IEventListener {
public:
    IEventListener() = default;
    virtual ~IEventListener() = default;
    virtual void OnMouseLocationEvent(const std::string &networkId, const Event &event) = 0;
};
} // namespace Msdp
} // namespace OHOS
#endif // I_LOCATION_LISTENER_H