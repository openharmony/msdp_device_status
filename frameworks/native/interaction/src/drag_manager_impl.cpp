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

#include "drag_manager_impl.h"

#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "drag_data.h"
namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragManagerImpl" };
} // namespace

DragManagerImpl::DragManagerImpl() {}
DragManagerImpl::~DragManagerImpl() {}

int32_t DragManagerImpl::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragStyle(style);
}

int32_t DragManagerImpl::UpdateDragMessage(const std::u16string &message)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().UpdateDragMessage(message);
}

int32_t DragManagerImpl::StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback)
{
    CALL_DEBUG_ENTER;
    SetCallback(callback);
    return DeviceStatusClient::GetInstance().StartDrag(dragData);
}

int32_t DragManagerImpl::StopDrag(int32_t &dragResult)
{
    CALL_DEBUG_ENTER;
    return DeviceStatusClient::GetInstance().StopDrag(dragResult);
}

void DragManagerImpl::SetCallback(std::function<void(int32_t&)> callback)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> guard(mtx_);
    CHKPV(callback);
    stopCallback_ = callback;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
