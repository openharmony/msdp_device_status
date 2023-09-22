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

#ifndef DRAG_PARAMS_H
#define DRAG_PARAMS_H
#include <memory>

#include "i_plugin.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum DragParam : uint32_t {
    DRAG_STYLE,
    DRAG_MESSAGE,
    PID,
    UDKEY,
    WINDOW_VISIBLE,
    SHADOW_OFFSET,
    SHADOW_PIC,
    LISTENER
};

struct DefaultDragReply final : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;
};

struct StartDragParam final : public ParamBase {
    StartDragParam(const DragData &dragData);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    DragData dragData;
};

struct StopDragParam final : public ParamBase {
    StopDragParam(int32_t result, bool hasCustomAnimation);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    int32_t result;
    bool hasCustomAnimation { false };
};

struct DragStyleParam final : public ParamBase {
    DragStyleParam(int32_t style);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    int32_t mouseStyle {};
};

struct DragTargetPidParam : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    int32_t pid;
};

struct GetUdKeyParam final : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    std::string udKey;
};

struct AddDragListenerParam : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);
};

struct RemoveDragListenerParam : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);
};

struct SetDragWindowVisibleParam final : public ParamBase {
    SetDragWindowVisibleParam(bool visible);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    bool visible;
};

struct GetShadowOffsetParam final : public ParamBase {
    GetShadowOffsetParam(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    int32_t offsetX { -1 };
    int32_t offsetY { -1 };
    int32_t width { -1 };
    int32_t height { -1 };
};

struct UpdateShadowPicParam final : public ParamBase {
    UpdateShadowPicParam(ShadowInfo shadowInfo);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data);

    ShadowInfo shadowInfo;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_PARAMS_H
