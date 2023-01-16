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

#include "drag_data_adapter.h"

#include "drag_data.h"
#include "devicestatus_define.h"
#include "fi_log.h"
#include "hitrace_meter.h"
#include "pointer_style.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDataAdapter" };
} // namespace

DragDataAdapter::DragDataAdapter() {}
DragDataAdapter::~DragDataAdapter() {}

void DragDataAdapter::Init(const DragData &dragData, const MMI::PointerStyle &pointerStyle)
{
    FI_HILOGD("Init dragDataAdapter");
}

int32_t DragDataAdapter::UpdateDragStyle(int32_t style)
{
    return RET_ERR;
}

int32_t DragDataAdapter::UpdateDragMessage(const std::u16string &message)
{
    return RET_ERR;
}

const DragData& DragDataAdapter::GetDragData()
{
    return dragData_;
}

int32_t DragDataAdapter::GetDragStyle() const
{
    return RET_ERR;
}

const std::u16string & DragDataAdapter::GetDragMessage() const
{
    return dragMessage_;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS