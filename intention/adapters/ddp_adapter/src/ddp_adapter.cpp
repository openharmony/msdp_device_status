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

#include "ddp_adapter.h"

#include "ddp_adapter_impl.h"
#include "devicestatus_define.h"

#undef LOG_TAG
#define LOG_TAG "DDPAdapter"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DDPAdapter::DDPAdapter()
{
    ddp_ = std::make_shared<DDPAdapterImpl>();
}

void DDPAdapter::AddObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_DEBUG_ENTER;
    ddp_->AddObserver(observer);
}

void DDPAdapter::RemoveObserver(std::shared_ptr<IDeviceProfileObserver> observer)
{
    CALL_DEBUG_ENTER;
    ddp_->RemoveObserver(observer);
}

void DDPAdapter::AddWatch(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    ddp_->AddWatch(networkId);
}

void DDPAdapter::RemoveWatch(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    ddp_->RemoveWatch(networkId);
}

void DDPAdapter::OnProfileChanged(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    ddp_->OnProfileChanged(networkId);
}

std::string DDPAdapter::GetNetworkIdByUdId(const std::string &udId)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetNetworkIdByUdId(udId);
}

std::string DDPAdapter::GetUdIdByNetworkId(const std::string &networkId)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetUdIdByNetworkId(networkId);
}

std::string DDPAdapter::GetLocalNetworkId()
{
    CALL_DEBUG_ENTER;
    return ddp_->GetLocalNetworkId();
}

int32_t DDPAdapter::UpdateCrossingSwitchState(bool state)
{
    CALL_DEBUG_ENTER;
    return ddp_->UpdateCrossingSwitchState(state);
}

int32_t DDPAdapter::GetCrossingSwitchState(const std::string &udId, bool &state)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetCrossingSwitchState(udId, state);
}

int32_t DDPAdapter::GetProperty(const std::string &udId, const std::string &name, bool &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(udId, name, value);
}

int32_t DDPAdapter::GetProperty(const std::string &udId, const std::string &name, int32_t &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(udId, name, value);
}

int32_t DDPAdapter::GetProperty(const std::string &udId, const std::string &name, std::string &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(udId, name, value);
}

int32_t DDPAdapter::SetProperty(const std::string &name, bool value)
{
    CALL_DEBUG_ENTER;
    return ddp_->SetProperty(name, value);
}

int32_t DDPAdapter::SetProperty(const std::string &name, int32_t value)
{
    CALL_DEBUG_ENTER;
    return ddp_->SetProperty(name, value);
}

int32_t DDPAdapter::SetProperty(const std::string &name, const std::string &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->SetProperty(name, value);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
