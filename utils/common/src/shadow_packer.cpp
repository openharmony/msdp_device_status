/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "shadow_packer.h"
#include "utility.h"

#include "parcel.h"
#include "securec.h"

#include "devicestatus_common.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "ShadowPacker" };
} // namespace


int32_t ShadowPacker::Marshalling(const std::vector<ShadowInfo> &shadowInfos, Parcel &data)
{
    CALL_DEBUG_ENTER;
    if (shadowInfos.empty()) {
        FI_HILOGE("Invalid parameter shadowInfos");
        return ERR_INVALID_VALUE;
    }
    int32_t shadowNum = static_cast<int32_t>(shadowInfos.size());
    if (shadowNum > SHADOW_NUM_LIMIT) {
        FI_HILOGW("Only %{public}d shadowInfos are allowed to be packaged at most, now %{public}d exceeding the limit",
            SHADOW_NUM_LIMIT, shadowNum);
        shadowNum  = SHADOW_NUM_LIMIT;
    }
    WRITEINT32(data, shadowNum, ERR_INVALID_VALUE);
    for (int32_t i = 0; i < shadowNum; i++) {
        CHKPR(shadowInfos[i].pixelMap, RET_ERR);
        if (!shadowInfos[i].pixelMap->Marshalling(data)) {
            FI_HILOGE("Failed to marshalling pixelMap");
            return ERR_INVALID_VALUE;
        }
        WRITEINT32(data, shadowInfos[i].x, ERR_INVALID_VALUE);
        WRITEINT32(data, shadowInfos[i].y, ERR_INVALID_VALUE);
    }
    return RET_OK;
}

int32_t ShadowPacker::UnMarshalling(Parcel &data, std::vector<ShadowInfo> &shadowInfos)
{
    CALL_DEBUG_ENTER;
    int32_t shadowNum { 0 };
    READINT32(data, shadowNum, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (shadowNum <= 0 || shadowNum > SHADOW_NUM_LIMIT) {
        FI_HILOGE("Invalid shadowNum:%{public}d", shadowNum);
        return RET_ERR;
    }
    for (int32_t i = 0; i < shadowNum; i++) {
        ShadowInfo shadowInfo;
        auto pixelMap = OHOS::Media::PixelMap::Unmarshalling(data);
        CHKPR(pixelMap, RET_ERR);
        shadowInfo.pixelMap = std::shared_ptr<OHOS::Media::PixelMap>(pixelMap);
        READINT32(data, shadowInfo.x, E_DEVICESTATUS_READ_PARCEL_ERROR);
        READINT32(data, shadowInfo.y, E_DEVICESTATUS_READ_PARCEL_ERROR);
        shadowInfos.push_back(shadowInfo);
    }
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
