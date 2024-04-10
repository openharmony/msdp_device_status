/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dinput_adapter.h"
#include "dinput_adapter_impl.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {

DInputAdapter::DInputAdapter(IContext *env)
{
    dinput_ = std::make_shared<DInputAdapterImpl>(env);
}

bool DInputAdapter::IsNeedFilterOut(const std::string &networkId, BusinessEvent &&event)
{
    return dinput_->IsNeedFilterOut(networkId, std::move(event));
}

int32_t DInputAdapter::StartRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    return dinput_->StartRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, callback);
}

int32_t DInputAdapter::StopRemoteInput(const std::string &remoteNetworkId, const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    return dinput_->StopRemoteInput(remoteNetworkId, originNetworkId, inputDeviceDhids, callback);
}

int32_t DInputAdapter::StopRemoteInput(const std::string &originNetworkId,
    const std::vector<std::string> &inputDeviceDhids, DInputCallback callback)
{
    return dinput_->StopRemoteInput(originNetworkId, inputDeviceDhids, callback);
}

int32_t DInputAdapter::PrepareRemoteInput(const std::string &remoteNetworkId,
    const std::string &originNetworkId, DInputCallback callback)
{
    return dinput_->PrepareRemoteInput(remoteNetworkId, originNetworkId, callback);
}

int32_t DInputAdapter::UnPrepareRemoteInput(const std::string &remoteNetworkId,
    const std::string &originNetworkId, DInputCallback callback)
{
    return dinput_->UnPrepareRemoteInput(remoteNetworkId, originNetworkId, callback);
}

int32_t DInputAdapter::PrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    return dinput_->PrepareRemoteInput(networkId, callback);
}

int32_t DInputAdapter::UnPrepareRemoteInput(const std::string &networkId, DInputCallback callback)
{
    return dinput_->UnPrepareRemoteInput(networkId, callback);
}

int32_t DInputAdapter::RegisterSessionStateCb(std::function<void(uint32_t)> callback)
{
    return dinput_->RegisterSessionStateCb(callback);
}

int32_t DInputAdapter::UnregisterSessionStateCb()
{
    return dinput_->UnregisterSessionStateCb();
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
