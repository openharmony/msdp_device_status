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

#include "drag_server.h"

#include "drag_params.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragServer" };
} // namespace

DragServer::DragServer(IContext *context)
    : context_(context)
{}

int32_t DragServer::Enable(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragServer::Disable(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t DragServer::Start(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    DragData dragData {};
    StartDragParam param { dragData };

    if (!param.Unmarshalling(data)) {
        FI_HILOGE("Failed to unmarshalling param");
        return RET_ERR;
    }
    CHKPR(context_, RET_ERR);
    FI_HILOGD("Start drag");
    return RET_OK;
}

int32_t DragServer::Stop(CallingContext &context, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    StopDragParam param {};

    if (!param.Unmarshalling(data)) {
        FI_HILOGE("Failed to unmarshalling param");
        return RET_ERR;
    }
    CHKPR(context_, RET_ERR);
    FI_HILOGD("Stop drag");
    return RET_OK;
}

int32_t DragServer::AddWatch(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    switch (id) {
        case DragRequestID::ADD_DRAG_LISTENER: {
            FI_HILOGD("Add drag listener");
            return RET_OK;
        }
        case DragRequestID::ADD_SUBSCRIPT_LISTENER: {
            FI_HILOGD("Add subscript listener");
            return RET_OK;
        }
        default: {
            FI_HILOGE("Unexpected request ID (%{public}u)", id);
            return RET_ERR;
        }
    }
}

int32_t DragServer::RemoveWatch(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    switch (id) {
        case DragRequestID::ADD_DRAG_LISTENER: {
            FI_HILOGD("Remove drag listener");
            return RET_OK;
        }
        case DragRequestID::ADD_SUBSCRIPT_LISTENER: {
            FI_HILOGD("Remove subscript listener");
            return RET_OK;
        }
        default: {
            FI_HILOGE("Unexpected request ID (%{public}u)", id);
            return RET_ERR;
        }
    }
}

int32_t DragServer::SetParam(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    switch (id) {
        case DragRequestID::UPDATE_SHADOW_PIC: {
            UpdateShadowPicParam param {};

            if (!param.Unmarshalling(data)) {
                FI_HILOGE("UpdateShadowPicParam::Unmarshalling fail");
                return RET_ERR;
            }
            FI_HILOGD("Updata shadow pic");
            return RET_OK;
        }
        default: {
            FI_HILOGE("Unexpected request ID (%{public}u)", id);
            return RET_ERR;
        }
    }
}

int32_t DragServer::GetParam(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    switch (id) {
        case DragRequestID::GET_DRAG_DATA: {
            DragData dragData {};
            FI_HILOGD("Get drag data");
            GetDragDataReply dragDataReply { dragData };

            if (!dragDataReply.Marshalling(reply)) {
                FI_HILOGE("GetDragDataReply::Marshalling fail");
                return RET_ERR;
            }
            return RET_OK;
        }
        default: {
            FI_HILOGE("Unexpected request ID (%{public}u)", id);
            return RET_ERR;
        }
    }
}

int32_t DragServer::Control(CallingContext &context, uint32_t id, MessageParcel &data, MessageParcel &reply)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS