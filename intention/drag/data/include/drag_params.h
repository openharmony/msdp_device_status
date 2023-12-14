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

#include "drag_data.h"
#include "intention_identity.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum DragRequestID : uint32_t {
    UNKNOWN_DRAG_ACTION,
    ADD_DRAG_LISTENER,
    REMOVE_DRAG_LISTENER,
    ADD_SUBSCRIPT_LISTENER,
    REMOVE_SUBSCRIPT_LISTENER,
    SET_DRAG_WINDOW_VISIBLE,
    UPDATE_DRAG_STYLE,
    UPDATE_SHADOW_PIC,
    GET_DRAG_TARGET_PID,
    GET_UDKEY,
    GET_SHADOW_OFFSET,
    GET_DRAG_DATA,
    UPDATE_PREVIEW_STYLE,
    UPDATE_PREVIEW_STYLE_WITH_ANIMATION,
    GET_DRAG_SUMMARY,
    GET_DRAG_STATE,
    ENTER_TEXT_EDITOR_AREA,
    GET_DRAG_ACTION,
    GET_EXTRA_INFO,
};

struct StartDragParam final : public ParamBase {
    StartDragParam(DragData &dragData);
    StartDragParam(const DragData &dragData);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    // For efficiency, we want to avoid copying 'DragData' whenever possible.
    // Considering the 'Start drag' scenario, we use 'StartDragParam' simply
    // as wrapper of 'DragData' and serialize 'DragData' in the right same
    // call. We do not dereference 'DragData' or keep a reference to it for
    // later use. In this case, we can safely keep a pointer to the input 'DragData'.
    DragData *dragDataPtr_ { nullptr };
    const DragData *cDragDataPtr_ { nullptr };
};

struct StopDragParam final : public ParamBase {
    StopDragParam() = default;
    StopDragParam(const DragDropResult &dropResult);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    DragDropResult dropResult_ {};
};

struct SetDragWindowVisibleParam final : public ParamBase {
    SetDragWindowVisibleParam() = default;
    SetDragWindowVisibleParam(bool visible);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    bool visible_ { false };
};

struct UpdateDragStyleParam final : public ParamBase {
    UpdateDragStyleParam() = default;
    UpdateDragStyleParam(DragCursorStyle style);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    DragCursorStyle cursorStyle_ { DragCursorStyle::DEFAULT };
};

struct UpdateShadowPicParam final : public ParamBase {
    UpdateShadowPicParam() = default;
    UpdateShadowPicParam(const ShadowInfo &shadowInfo);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    ShadowInfo shadowInfo_ {};
};

struct GetDragTargetPidReply : public ParamBase {
    GetDragTargetPidReply() = default;
    GetDragTargetPidReply(int32_t pid);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    int32_t targetPid_ { -1 };
};

struct GetUdKeyReply final : public ParamBase {
    GetUdKeyReply() = default;
    GetUdKeyReply(std::string &&udKey);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    std::string udKey_;
};

struct GetShadowOffsetReply final : public ParamBase {
    GetShadowOffsetReply() = default;
    GetShadowOffsetReply(int32_t offsetX, int32_t offsetY, int32_t width, int32_t height);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    int32_t offsetX_ { -1 };
    int32_t offsetY_ { -1 };
    int32_t width_ { -1 };
    int32_t height_ { -1 };
};

using GetDragDataReply = StartDragParam;

struct UpdatePreviewStyleParam final : public ParamBase {
    UpdatePreviewStyleParam() = default;
    UpdatePreviewStyleParam(const PreviewStyle &previewStyle);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    PreviewStyle previewStyle_;
};

struct UpdatePreviewAnimationParam final : public ParamBase {
    UpdatePreviewAnimationParam() = default;
    UpdatePreviewAnimationParam(const PreviewStyle &previewStyle, const PreviewAnimation &animation);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    PreviewStyle previewStyle_ {};
    PreviewAnimation previewAnimation_ {};
};

struct GetDragSummaryReply final : public ParamBase {
    GetDragSummaryReply() = default;
    GetDragSummaryReply(std::map<std::string, int64_t> &&summaries);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    std::map<std::string, int64_t> summaries_;
};

struct GetDragStateReply final : public ParamBase {
    GetDragStateReply() = default;
    GetDragStateReply(DragState dragState);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    DragState dragState_ { DragState::ERROR };
};

struct EnterTextEditorAreaParam final : public ParamBase {
    EnterTextEditorAreaParam() = default;
    EnterTextEditorAreaParam(bool enable);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    bool enable_ { false };
};

struct GetDragActionReply final : public ParamBase {
    GetDragActionReply() = default;
    GetDragActionReply(DragAction dragAction);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    DragAction dragAction_ { DragAction::INVALID };
};

struct GetExtraInfoReply final : public ParamBase {
    GetExtraInfoReply() = default;
    GetExtraInfoReply(std::string &&extraInfo);

    bool Marshalling(MessageParcel &parcel) const override;
    bool Unmarshalling(MessageParcel &parcel) override;

    std::string extraInfo_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_PARAMS_H
