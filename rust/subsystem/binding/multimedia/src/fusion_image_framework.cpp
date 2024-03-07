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

#include "fusion_image_framework_internal.h"

#include "c_parcel_internal.h"

#include "devicestatus_define.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "fusion_image_framework"

namespace {
static const int32_t MAX_PIXEL_MAP_WIDTH { 600 };
static const int32_t MAX_PIXEL_MAP_HEIGHT { 600 };

CPixelMap* CPixelMapFrom(std::shared_ptr<::OHOS::Media::PixelMap> pixelMap)
{
    auto cImg = new (std::nothrow) CPixelMap;
    CHKPP(cImg);
    cImg->refCnt = 1;
    cImg->pixelMap = pixelMap;
    return cImg;
}

struct CPixelMap* CPixelMapRef(struct CPixelMap *pixelMap)
{
    CHKPP(pixelMap);
    pixelMap->refCnt++;
    return pixelMap;
}

struct CPixelMap* CPixelMapUnref(struct CPixelMap *pixelMap)
{
    CHKPP(pixelMap);
    if (pixelMap->refCnt > 0) {
        pixelMap->refCnt--;
    }
    if (pixelMap->refCnt > 0) {
        return pixelMap;
    }
    delete pixelMap;
    return nullptr;
}

int32_t CPixelMapSerialize(const struct CPixelMap *pixelMap, CParcel *parcel)
{
    CALL_DEBUG_ENTER;
    CHKPR(parcel, RET_ERR);
    CHKPR(parcel->parcel_, RET_ERR);
    CHKPR(pixelMap, RET_ERR);
    CHKPR(pixelMap->pixelMap, RET_ERR);
    if (!pixelMap->pixelMap->Marshalling(*parcel->parcel_)) {
        return RET_ERR;
    }
    return RET_OK;
}

struct CPixelMap* CPixelMapDeserialize(const CParcel *parcel)
{
    CALL_DEBUG_ENTER;
    CHKPP(parcel);
    CHKPP(parcel->parcel_);

    auto pixelMap = ::OHOS::Media::PixelMap::Unmarshalling(*parcel->parcel_);
    CHKPP(pixelMap);
    std::shared_ptr<::OHOS::Media::PixelMap> sPixelMap(pixelMap);

    if (sPixelMap->GetWidth() > MAX_PIXEL_MAP_WIDTH || sPixelMap->GetHeight() > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Pixelmap is oversize, width:%{public}d, height:%{public}d",
            sPixelMap->GetWidth(), sPixelMap->GetHeight());
        return nullptr;
    }
    return CPixelMapFrom(sPixelMap);
}
} // namespace