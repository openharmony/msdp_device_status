/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <message_parcel.h>

#include "iremote_object.h"
#include "message_option.h"

#include "devicestatus_callback_proxy.h"
#include "devicestatus_client.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
void DeviceStatusCallbackProxy::OnDeviceStatusChanged(const Data& devicestatusData)
{
    sptr<IRemoteObject> remote = Remote();
    DEV_RET_IF_NULL(remote == nullptr);

    std::map<Type, int32_t> typeMap= DeviceStatusClient::GetInstance().GetTypeMap();
    auto it = typeMap.find(devicestatusData.type);
    if (it == typeMap.end()) {
        DEV_HILOGE(INNERKIT, "type not exist report failed");
        return;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DeviceStatusCallbackProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    WRITEINT32(data, static_cast<int32_t>(devicestatusData.type));
    WRITEINT32(data, static_cast<int32_t>(devicestatusData.value));

    int32_t ret = remote->SendRequest(static_cast<int32_t>(IRemoteDevStaCallback::DEVICESTATUS_CHANGE),
        data, reply, option);
    if (ret != RET_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code:%{public}d", ret);
        return;
    }
}
} // namespace DeviceStatus
} // Msdp
} // OHOS
