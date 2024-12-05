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

#include "collaboration_service_status_change.h"

#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "CollaborationServiceStatusChange"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CollaborationServiceStatusChange::CollaborationServiceStatusChange(IContext *context)
    : context_(context)
{}

void CollaborationServiceStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    FI_HILOGI("systemAbilityId:%{public}d", systemAbilityId);
}

void CollaborationServiceStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    FI_HILOGI("systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != DEVICE_COLLABORATION_SA_ID) {
        FI_HILOGE("systemAbilityId is not DEVICE_COLLABORATION_SA_ID");
        return;
    }
    CHKPV(context_);
    context_->GetSocketSessionManager().DeleteCollaborationServiceByName();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS