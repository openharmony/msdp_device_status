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

#ifndef COLLABORATION_SERVICE_STATUS_CHANGE_H
#define COLLABORATION_SERVICE_STATUS_CHANGE_H

#include "system_ability_definition.h"
#include "system_ability_status_change_stub.h"

#include "i_context.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CollaborationServiceStatusChange : public SystemAbilityStatusChangeStub {
inline static constexpr int DEVICE_COLLABORATION_SA_ID { 65588 };
public:
    explicit CollaborationServiceStatusChange(IContext *context);
    ~CollaborationServiceStatusChange() = default;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;

private:
    IContext *context_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COLLABORATION_SERVICE_STATUS_CHANGE_H