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

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DDPAdapter" };
} // namespace

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

int32_t DDPAdapter::GetProperty(const std::string &networkId, const std::string &name, bool &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(networkId, name, value);
}

int32_t DDPAdapter::GetProperty(const std::string &networkId, const std::string &name, int32_t &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(networkId, name, value);
}

int32_t DDPAdapter::GetProperty(const std::string &networkId, const std::string &name, std::string &value)
{
    CALL_DEBUG_ENTER;
    return ddp_->GetProperty(networkId, name, value);
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
