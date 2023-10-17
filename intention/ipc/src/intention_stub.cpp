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

#include "intention_stub.h"

#include "message_parcel.h"
#include "pixel_map.h"

#include "fi_log.h"
#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "i_plugin.h"
// #include "intention_proxy.h"
// #include "intention_service.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionStub" };
} // namespace

int32_t IntentionStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    std::u16string descriptor = IntentionStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        FI_HILOGE("IntentionStub::OnRemoteRequest failed, descriptor is not matched");
        return E_DEVICESTATUS_GET_SERVICE_FAILED;
    }

    switch (GACTION(code)) {
        case CommonAction::ENABLE: {
            return Enable(GINTENTION(code), data, reply);
        }
        case CommonAction::DISABLE: {
            return Disable(GINTENTION(code), data, reply);
        }
        case CommonAction::START: {
            return Start(GINTENTION(code), data, reply);
        }
        case CommonAction::STOP: {
            return Stop(GINTENTION(code), data, reply);
        }
        case CommonAction::ADD_WATCH: {
            return AddWatch(GINTENTION(code), GPARAM(code), data, reply);
        }
        case CommonAction::REMOVE_WATCH: {
            return RemoveWatch(GINTENTION(code), GPARAM(code), data, reply);
        }
        case CommonAction::SET_PARAM: {
            return SetParam(GINTENTION(code), GPARAM(code), data, reply);
        }
        case CommonAction::GET_PARAM: {
            return GetParam(GINTENTION(code), GPARAM(code), data, reply);
        }
        case CommonAction::CONTROL: {
            return Control(GINTENTION(code), GPARAM(code), data, reply);
        }
    }
    return RET_ERR;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS