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

#include "cooperate_params.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DefaultCooperateParam::DefaultCooperateParam(int32_t userData)
    : userData(userData)
{}

bool DefaultCooperateParam::Marshalling(Parcel &data) const
{
    return data.WriteInt32(userData);
}

bool DefaultCooperateParam::Unmarshalling(Parcel &data)
{
    return data.ReadInt32(userData);
}

bool DefaultCooperateReply::Marshalling(Parcel &data) const
{
    return true;
}

bool DefaultCooperateReply::Unmarshalling(Parcel &data)
{
    return true;
}

StartCooperateParam::StartCooperateParam()
{
}

StartCooperateParam::StartCooperateParam(int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId)
    : remoteNetworkId(remoteNetworkId),
      startDeviceId(startDeviceId),
      userData(userData)
{}

bool StartCooperateParam::Marshalling(Parcel &data) const
{
    return (
        data.WriteInt32(userData) &&
        data.WriteString(remoteNetworkId) &&
        data.WriteInt32(startDeviceId)
    );
}

bool StartCooperateParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(userData) &&
        data.ReadString(remoteNetworkId) &&
        data.ReadInt32(startDeviceId)
    );
}

GetCooperateStateParam::GetCooperateStateParam()
{
}

GetCooperateStateParam::GetCooperateStateParam(std::string deviceId, int32_t userData)
    : deviceId(deviceId),
      userData(userData)
{}

bool GetCooperateStateParam::Marshalling(Parcel &data) const
{
    return (
        data.WriteInt32(userData) &&
        data.WriteString(deviceId)
    );
}

bool GetCooperateStateParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(userData) &&
        data.ReadString(deviceId)
    );
}

StopCooperateParam::StopCooperateParam()
{
}

StopCooperateParam::StopCooperateParam(int32_t userData, bool isUnchained)
    : userData(userData), isUnchained(isUnchained)
{}

bool StopCooperateParam::Marshalling(Parcel &data) const
{
    return (
        data.WriteInt32(userData) &&
        data.WriteBool(isUnchained)
    );
}

bool StopCooperateParam::Unmarshalling(Parcel &data)
{
    return (
        data.ReadInt32(userData) &&
        data.ReadBool(isUnchained)
    );
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS