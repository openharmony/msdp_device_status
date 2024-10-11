/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "cooperate_client_test_mock.h"

#include "tunnel_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

CooperateClientMock *CooperateClientMock::g_mock;

CooperateClientMock::CooperateClientMock()
{
    g_mock = this;
}

CooperateClientMock::~CooperateClientMock()
{
    g_mock = nullptr;
}

TunnelClient::~TunnelClient()
{
    if (devicestatusProxy_ != nullptr) {
        auto remoteObject = devicestatusProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

int32_t TunnelClient::Enable(Intention intention, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().Enable(intention, data, reply);
}

int32_t TunnelClient::Disable(Intention intention, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().Disable(intention, data, reply);
}

int32_t TunnelClient::Start(Intention intention, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().Start(intention, data, reply);
}

int32_t TunnelClient::Stop(Intention intention, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().Stop(intention, data, reply);
}

int32_t TunnelClient::AddWatch(Intention intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().AddWatch(intention, id, data, reply);
}

int32_t TunnelClient::RemoveWatch(Intention intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().RemoveWatch(intention, id, data, reply);
}

int32_t TunnelClient::SetParam(Intention intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().SetParam(intention, id, data, reply);
}

int32_t TunnelClient::GetParam(Intention intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().GetParam(intention, id, data, reply);
}

int32_t TunnelClient::Control(Intention intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    return CooperateClientMock::GetMock().Control(intention, id, data, reply);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS