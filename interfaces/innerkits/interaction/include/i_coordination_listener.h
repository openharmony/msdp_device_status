/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef I_COORDINATION_LISTENER_H
#define I_COORDINATION_LISTENER_H

#include <string>

#include "coordination_message.h"

namespace OHOS {
namespace Msdp {
class ICoordinationListener {
public:
    ICoordinationListener() = default;
    virtual ~ICoordinationListener() = default;
    virtual void OnCoordinationMessage(const std::string &networkId, CoordinationMessage msg) = 0;
};
} // namespace Msdp
} // namespace OHOS
#endif // I_COORDINATION_LISTENER_H