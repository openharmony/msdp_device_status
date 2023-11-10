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

#include "summary_packer.h"

#include <message_parcel.h>

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "SummaryPacker" };
} // namespace

int32_t SummaryPacker::Marshalling(const SummaryMap &val, Parcel &parcel)
{
    WRITEINT32(parcel, static_cast<int32_t>(val.size()), ERR_INVALID_VALUE);
    for (auto const &[k, v] : val) {
        WRITESTRING(parcel, k, ERR_INVALID_VALUE);
        WRITEINT64(parcel, v, ERR_INVALID_VALUE);
    }
    return RET_OK;
}

int32_t SummaryPacker::UnMarshalling(Parcel &parcel, SummaryMap &val)
{
    int32_t size = 0;
    READINT32(parcel, size, E_DEVICESTATUS_READ_PARCEL_ERROR);
    if (size < 0) {
        FI_HILOGE("Invalid size:%{public}d", size);
        return RET_ERR;
    }
    size_t readAbleSize = parcel.GetReadableBytes();
    if ((static_cast<size_t>(size) > readAbleSize) || static_cast<size_t>(size) > val.max_size()) {
        return RET_ERR;
    }

    for (int32_t i = 0; i < size; ++i) {
        std::string key;
        READSTRING(parcel, key, E_DEVICESTATUS_READ_PARCEL_ERROR);
        READINT64(parcel, val[key], E_DEVICESTATUS_READ_PARCEL_ERROR);
    }
    return RET_OK;
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
