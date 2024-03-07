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
#include <vector>

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "preview_style_packer.h"

#undef LOG_TAG
#define LOG_TAG "PreviewStylePacker"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

int32_t PreviewStylePacker::Marshalling(const PreviewStyle &previewStyle, Parcel &data)
{
    std::vector<int32_t> types;
    for (const auto &elem : previewStyle.types) {
        types.push_back(static_cast<int32_t>(elem));
    }
    WRITEINT32VECTOR(data, types, ERR_INVALID_VALUE);
    WRITEUINT32(data, previewStyle.foregroundColor, ERR_INVALID_VALUE);
    WRITEINT32(data, previewStyle.opacity, ERR_INVALID_VALUE);
    WRITEFLOAT(data, previewStyle.radius, ERR_INVALID_VALUE);
    WRITEFLOAT(data, previewStyle.scale, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t PreviewStylePacker::UnMarshalling(Parcel &data, PreviewStyle &previewStyle)
{
    std::vector<int32_t> types;
    READINT32VECTOR(data, types, ERR_INVALID_VALUE);
    for (const auto &elem : types) {
        previewStyle.types.push_back(static_cast<PreviewType>(elem));
    }
    READUINT32(data, previewStyle.foregroundColor, ERR_INVALID_VALUE);
    READINT32(data, previewStyle.opacity, ERR_INVALID_VALUE);
    READFLOAT(data, previewStyle.radius, ERR_INVALID_VALUE);
    READFLOAT(data, previewStyle.scale, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t PreviewAnimationPacker::Marshalling(const PreviewAnimation &previewAnimation, Parcel &data)
{
    WRITEINT32(data, previewAnimation.duration, ERR_INVALID_VALUE);
    WRITESTRING(data, previewAnimation.curveName, ERR_INVALID_VALUE);
    WRITEFLOATVECTOR(data, previewAnimation.curve, ERR_INVALID_VALUE);
    return RET_OK;
}

int32_t PreviewAnimationPacker::UnMarshalling(Parcel &data, PreviewAnimation &previewAnimation)
{
    READINT32(data, previewAnimation.duration, ERR_INVALID_VALUE);
    if (previewAnimation.duration <= 0) {
        FI_HILOGE("Invalid paramater duration:%{public}d", previewAnimation.duration);
        return ERR_INVALID_VALUE;
    }
    if (previewAnimation.duration > MAX_ANIMATION_DURATION_MS) {
        FI_HILOGW("Duration:%{public}d too long, use default value", previewAnimation.duration);
        previewAnimation.duration = MAX_ANIMATION_DURATION_MS;
    }
    READSTRING(data, previewAnimation.curveName, ERR_INVALID_VALUE);
    READFLOATVECTOR(data, previewAnimation.curve, ERR_INVALID_VALUE);
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS