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

#ifndef COOPERATE_PARAMS_H
#define COOPERATE_PARAMS_H

#include "i_plugin.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum CooperateParam : uint32_t {
    PREPARE,
    STATE,
    REGISTER
};

struct DefaultCooperateParam final : public ParamBase {
    DefaultCooperateParam() = default;
    DefaultCooperateParam(int32_t userData);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;

    int32_t userData { -1 };
};

struct DefaultCooperateReply final : public ParamBase {
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;
};

struct StartCooperateParam final : public ParamBase {
    StartCooperateParam();
    StartCooperateParam(int32_t userData, const std::string &remoteNetworkId, int32_t startDeviceId);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;

    std::string remoteNetworkId;
    int32_t startDeviceId { -1 };
    int32_t userData { -1 };
};

struct StopCooperateParam final : public ParamBase {
    StopCooperateParam();
    StopCooperateParam(int32_t userData, bool isUnchained);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;

    int32_t userData { -1 };
    bool isUnchained { false };
};

struct GetCooperateStateParam final : public ParamBase {
    GetCooperateStateParam();
    GetCooperateStateParam(std::string deviceId, int32_t userData);
    bool Marshalling(Parcel &data) const override;
    bool Unmarshalling(Parcel &data) override;

    std::string deviceId;
    int32_t userData { -1 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // COOPERATE_PARAMS_H
