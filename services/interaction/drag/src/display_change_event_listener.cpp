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

#include "display_change_event_listener.h"

#undef LOG_TAG
#define LOG_TAG "DisplayChangeEventListener"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DisplayChangeEventListener::DisplayChangeEventListener(IContext *context)
    : context_(context)
{
}

void DisplayChangeEventListener::OnCreate(Rosen::DisplayId displayId)
{
    FI_HILOGI("display:%{public}" PRIu64"", displayId);
}

void DisplayChangeEventListener::OnDestroy(Rosen::DisplayId displayId)
{
    FI_HILOGI("display:%{public}" PRIu64"", displayId);
}

void DisplayChangeEventListener::OnChange(Rosen::DisplayId displayId)
{
    if (Rosen::DisplayManager::GetInstance().IsFoldable()) {
        Rosen::FoldDisplayMode foldMode = Rosen::DisplayManager::GetInstance().GetFoldDisplayMode();
        if (foldMode == Rosen::FoldDisplayMode::FULL) {
            if (lastRotation_ == Rosen::Rotation::ROTATION_0) {
                FI_HILOGD("FoldDisplayMode is full");
                return;
            }
            lastRotation_ = Rosen::Rotation::ROTATION_0;
            CHKPV(context_);
            int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
                std::bind(&IDragManager::RotateDragWindow, &context_->GetDragManager(), Rosen::Rotation::ROTATION_0));
            if (ret != RET_OK) {
                FI_HILOGE("Post async task failed");
            }
            return;
        }
    }
    sptr<Rosen::Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(displayId);
    if (display == nullptr) {
        FI_HILOGD("Get display info failed, display:%{public}" PRIu64"", displayId);
        display = Rosen::DisplayManager::GetInstance().GetDisplayById(0);
        if (display == nullptr) {
            FI_HILOGE("Get display info failed, display is nullptr");
            return;
        }
    }
    Rosen::Rotation currentRotation = display->GetRotation();
    if (currentRotation == lastRotation_) {
        return;
    }
    lastRotation_ = currentRotation;
    CHKPV(context_);
    int32_t ret = context_->GetDelegateTasks().PostAsyncTask(
        std::bind(&IDragManager::RotateDragWindow, &context_->GetDragManager(), currentRotation));
    if (ret != RET_OK) {
        FI_HILOGE("Post async task failed");
    }
}

DisplayAbilityStatusChange::DisplayAbilityStatusChange(IContext *context)
    : context_(context)
{}

void DisplayAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    FI_HILOGI("systemAbilityId:%{public}d", systemAbilityId);
    if (systemAbilityId != DISPLAY_MANAGER_SERVICE_SA_ID) {
        FI_HILOGE("systemAbilityId is not DISPLAY_MANAGER_SERVICE_SA_ID");
        return;
    }
    CHKPV(context_);
    displayChangeEventListener_ = sptr<DisplayChangeEventListener>::MakeSptr(context_);
    CHKPV(displayChangeEventListener_);
    Rosen::DisplayManager::GetInstance().RegisterDisplayListener(displayChangeEventListener_);
}

void DisplayAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    FI_HILOGI("systemAbilityId:%{public}d", systemAbilityId);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS