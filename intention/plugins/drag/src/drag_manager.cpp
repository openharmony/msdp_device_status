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

#include "drag_manager.h"

#include "devicestatus_define.h"
#include "fi_log.h"
#include "drag_params.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragManager" };
} // namespace

int32_t DragManager::AddListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::RemoveListener(SessionPtr session)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::StartDrag(const DragData &dragData, SessionPtr sess)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::StopDrag(DragResult result, bool hasCustomAnimation)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::GetDragTargetPid() const
{
    return RET_ERR;
}

int32_t DragManager::GetUdKey(std::string &udKey) const
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::UpdateDragStyle(DragCursorStyle style, int32_t targetPid, int32_t targetTid)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragManager::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
