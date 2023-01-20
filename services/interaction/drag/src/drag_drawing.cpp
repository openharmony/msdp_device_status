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

#include "drag_drawing.h"

#include "hitrace_meter.h"

#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawing" };
} // namespace


void DragDrawing::InitPicture(OHOS::Media::PixelMap pixelMap, int32_t x, int32_t y)
{
    FI_HILOGD("Init DragDrawing");
}

void DragDrawing::Draw(int32_t x, int32_t y)
{
}

void DragDrawing::UpdateDragMessage(uint8_t message[])
{
}

void DragDrawing::UpdateDragStyle(int32_t dragStyle)
{
}

void DragDrawing::DrawMessage()
{
}

void DragDrawing::DrawStyle()
{
}

void DragDrawing::DrawPicture()
{
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS