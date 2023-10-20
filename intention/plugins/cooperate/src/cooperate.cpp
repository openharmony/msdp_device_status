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

#include "cooperate.h"

#include "cooperate_params.h"
#include "devicestatus_define.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "Cooperate" };
} // namespace

Cooperate::Cooperate(IContext *context)
    : context_(context)
{}

int32_t Cooperate::Enable(CallingContext &context, Parcel &data, Parcel &reply)
{
    sm_.EnableCooperate();
    return RET_OK;
}

int32_t Cooperate::Disable(CallingContext &context, Parcel &data, Parcel &reply)
{
    sm_.DisableCooperate();
    return RET_OK;
}

int32_t Cooperate::Start(CallingContext &context, Parcel &data, Parcel &reply)
{
    CHKPR(context_, RET_ERR);
    StartCooperateParam param;
    if (!param.Unmarshalling(data)) {
        FI_HILOGE("Failed to unmarshalling param");
        return RET_ERR;
    }

    int32_t ret = sm_.StartCooperate(param.remoteNetworkId, param.startDeviceId);
    if (ret != RET_OK) {
        FI_HILOGE("StartCooperate failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t Cooperate::Stop(CallingContext &context, Parcel &data, Parcel &reply)
{
    StopCooperateParam param;
    if (!param.Unmarshalling(data)) {
        FI_HILOGE("Failed to unmarshalling param");
        return RET_ERR;
    }

    int32_t ret = sm_.StopCooperate(param.isUnchained);
    if (ret != RET_OK) {
        FI_HILOGE("Deactivate coordination failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t Cooperate::AddWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_OK;
}

int32_t Cooperate::RemoveWatch(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_OK;
}

int32_t Cooperate::SetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

int32_t Cooperate::GetParam(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    GetCooperateStateParam param;
    if (!param.Unmarshalling(data)) {
        FI_HILOGE("Failed to unmarshalling param");
        return RET_ERR;
    }

    int32_t ret = sm_.GetCooperateState(param.deviceId);
    if (ret != RET_OK) {
        FI_HILOGE("Get coordination state failed");
    }
    return ret;
}

int32_t Cooperate::Control(CallingContext &context, uint32_t id, Parcel &data, Parcel &reply)
{
    return RET_ERR;
}

extern "C" IPlugin* CreateInstance(IContext *context)
{
    CHKPP(context);
    return new (std::nothrow) Cooperate(context);
}

extern "C" void DestroyInstance(IPlugin *instance)
{
    if (instance != nullptr) {
        delete instance;
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS