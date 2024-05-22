/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "raw_data_packer.h"

#include "devicestatus_define.h"
#include "softbus_message_id.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace Cooperate {

constexpr uint32_t SOFTBUS_MESSAGE_ID_SIZE { sizeof(SoftbusMessageId) };
constexpr uint32_t MAX_CAPACITY_OF_RAW_DATA { 1048567 }; // 1MB

int32_t RawDataPacker::GetSoftbusMessageId(const void *data, uint32_t dataLen, SoftbusMessageId &messageId)
{
    CALL_INFO_TRACE;
    if (dataLen < SOFTBUS_MESSAGE_ID_SIZE) {
        FI_HILOGE("GetSoftbusMessageId failed, dataLen is too short");
        return RET_ERR;
    }
    if (memcpy_s(&messageId, SOFTBUS_MESSAGE_ID_SIZE, data, SOFTBUS_MESSAGE_ID_SIZE) != EOK) {
        FI_HILOGE("GetSoftbusMessageId failed, memcpy_s failed");
        return RET_ERR;
    }
    if (messageId <= SoftbusMessageId::MIN_ID || messageId >= SoftbusMessageId::MAX_ID) {
        FI_HILOGE("Unsupported softbusMessageId, skip");
        return RET_ERR;
    }
    return RET_OK;  

}

int32_t RawDataPacker::WriteRawDataToParcel(const void *data, uint32_t dataLen, Parcel &parcel)
{
    CALL_INFO_TRACE;
    if (dataLen > MAX_CAPACITY_OF_RAW_DATA) {
        FI_HILOGE("Data size:%{public}u exceeds limits:%{public}u", dataLen, MAX_CAPACITY_OF_RAW_DATA);
        return RET_ERR;
    }
    if (!parcel.SetMaxCapacity(MAX_CAPACITY_OF_RAW_DATA)) {
        FI_HILOGE("SetMaxCapacity failed");
        return RET_ERR;
    }
    if (!parcel.WriteBuffer(data, dataLen)) {
        FI_HILOGE("WriteBuffer failed");
        return RET_ERR;
    }
    return RET_OK;
}
} // namespace Cooperate
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
