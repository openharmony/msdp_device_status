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

#include "devicestatus_callback_proxy.h"

#include <message_parcel.h>

#include "devicestatus_common.h"

#include "iremote_object.h"
#include "message_option.h"

namespace OHOS {
namespace Msdp {
void DevicestatusCallbackProxy::OnDevicestatusChanged(const DevicestatusDataUtils::DevicestatusData& devicestatusData)
{
    sptr<IRemoteObject> remote = Remote();
    DEVICESTATUS_RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DevicestatusCallbackProxy::GetDescriptor())) {
        DEV_HILOGE(INNERKIT, "Write descriptor failed");
        return;
    }

    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, static_cast<int32_t>(devicestatusData.type));
    DEVICESTATUS_WRITE_PARCEL_NO_RET(data, Int32, static_cast<int32_t>(devicestatusData.value));

    int32_t ret = remote->SendRequest(static_cast<int32_t>(IdevicestatusCallback::DEVICESTATUS_CHANGE),
        data, reply, option);
    if (ret != ERR_OK) {
        DEV_HILOGE(INNERKIT, "SendRequest is failed, error code: %{public}d", ret);
    }
}
} // Msdp
} // OHOS