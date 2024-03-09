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

#include "fusion_data_binding_internal.h"

#include "devicestatus_define.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "fusion_data_binding"

struct CDragData* CDragDataFree(struct CDragData *cdrag)
{
    CHKPP(cdrag);
    return nullptr;
}

int32_t CDragDataFrom(const ::OHOS::Msdp::DeviceStatus::DragData *drag, CDragData *cdrag)
{
    CALL_DEBUG_ENTER;
    CHKPR(drag, RET_ERR);
    CHKPR(cdrag, RET_ERR);
    cdrag->shadowInfo.x = drag->shadowInfo.x;
    cdrag->shadowInfo.y = drag->shadowInfo.y;
    cdrag->buffer = drag->buffer.data();
    cdrag->bufSize = drag->buffer.size();
    cdrag->sourceType = drag->sourceType;
    cdrag->dragNum = drag->dragNum;
    cdrag->pointerId = drag->pointerId;
    cdrag->displayX = drag->displayX;
    cdrag->displayY = drag->displayY;
    cdrag->displayId = drag->displayId;
    cdrag->hasCanceledAnimation = drag->hasCanceledAnimation;
    return RET_OK;
}
