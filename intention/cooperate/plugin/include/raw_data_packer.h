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

#ifndef RAW_DATA_PACKER_H
#define RAW_DATA_PACKER_H

#include "nocopyable.h"
#include "parcel.h"

#include "softbus_message_id.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

class RawDataPacker {
public:
    static int32_t GetSoftbusMessageId(const void *data, uint32_t dataLen, SoftbusMessageId &messageId);
    static int32_t WriteRawDataToParcel(const void *data, uint32_t dataLen, Parcel &parcel);
};
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // RAW_DATA_PACKER_H
