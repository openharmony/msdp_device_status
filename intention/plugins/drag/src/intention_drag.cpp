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

#include "intention_drag.h"

#include "drag_params.h"
#include "fi_log.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionDrag" };
} // namespace

IntentionDrag::IntentionDrag(IContext *context)
    : context_(context)
{}

int32_t IntentionDrag::Enable(CallingContext &context, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t IntentionDrag::Disable(CallingContext &context, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t IntentionDrag::Start(CallingContext &context, Parcel &data, Parcel &reply)
{
    StartDragParam param;
    if (!param.Unmarshalling(data)) {
        return RET_ERR;
    }
    return dragMgr_.StartDrag(param, context.session);
}

int32_t IntentionDrag::Stop(CallingContext &context, Parcel &data, Parcel &reply)
{
    StopDragParam param;
    if (!param.Unmarshalling(data)) {
        return RET_ERR;
    }
    return dragMgr_.StopDrag(param.result, param.hasCustomAnimation);
}

int32_t IntentionDrag::AddWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t IntentionDrag::RemoveWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t IntentionDrag::SetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    switch (id) {
        case DragParam::WINDOW_VISIBLE: {
            SetDragWindowVisibleParam param;
            if (param.Unmarshalling(data)) {
                return dragMgr_.SetDragWindowVisible(param.visible);
            }
            break;
        }
        case DragParam::SHADOW_PIC: {
            UpdateShadowPicParam param;
            if (param.Unmarshalling(data)) {
                return dragMgr_.UpdateShadowPic(param.shadowInfo);
            }
            break;
        }
        default: {
            FI_HILOGE("The \'DragParam\' parameter is invalid");
            break;
        }
    }
    return RET_ERR;
}

int32_t IntentionDrag::GetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t IntentionDrag::Control(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

extern "C" IPlugin* CreateInstance(IContext *context)
{
    CHKPP(context);
    return new (std::nothrow) IntentionDrag(context);
}

extern "C" void DestroyInstance(IPlugin *instance)
{
    if (instance != nullptr) {
        delete instance;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
