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

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "cooperate_manager_impl.h"
#include "devicestatus_define.h"
#include "drag_manager_impl.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr ::OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "IntentionClient" };
} // namespace

IntentionClient::IntentionClient() {}

IntentionClient::~IntentionClient()
{
    if (devicestatusProxy_ != nullptr) {
        auto remoteObject = devicestatusProxy_->AsObject();
        if (remoteObject != nullptr) {
            remoteObject->RemoveDeathRecipient(deathRecipient_);
        }
    }
}

int32_t IntentionClient::Enable(uint32_t intention, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Enable, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Enable, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->Enable(intention, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::Disable(uint32_t intention, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Disable, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Disable, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->Disable(intention, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::Start(uint32_t intention, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Start, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Start, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->Start(intention, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::Stop(uint32_t intention, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Stop, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Stop, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->Stop(intention, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::AddWatch(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Add watch, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Add watch, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->AddWatch(intention, id, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::RemoveWatch(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Remove watch, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Remove watch, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->RemoveWatch(intention, id, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::SetParam(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Set param, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Set param, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->SetParam(intention, id, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::GetParam(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Get param, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Get param, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->GetParam(intention, id, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

int32_t IntentionClient::Control(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply)
{
    CALL_DEBUG_ENTER;
    MessageParcel dataParcel;
    if (!dataParcel.WriteInterfaceToken(IntentionProxy::GetDescriptor())) {
        FI_HILOGE("Control, write descriptor failed");
        return RET_ERR;
    }
    data.Marshalling(dataParcel);

    if (Connect() != RET_OK) {
        FI_HILOGE("Control, failed to connect IntentionService");
        return RET_ERR;
    }
    MessageParcel replyParcel;
    int32_t ret = devicestatusProxy_->Control(intention, id, dataParcel, replyParcel);
    if (ret == RET_OK) {
        reply.Unmarshalling(replyParcel);
    }
    return ret;
}

ErrCode IntentionClient::Connect()
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> lock(mutex_);
    if (devicestatusProxy_ != nullptr) {
        return RET_OK;
    }

    sptr<ISystemAbilityManager> sa = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHKPR(sa, E_DEVICESTATUS_GET_SYSTEM_ABILITY_MANAGER_FAILED);

    sptr<IRemoteObject> remoteObject = sa->CheckSystemAbility(MSDP_DEVICESTATUS_SERVICE_ID);
    CHKPR(remoteObject, E_DEVICESTATUS_GET_SERVICE_FAILED);

    deathRecipient_ = sptr<IRemoteObject::DeathRecipient>(new (std::nothrow) IntentionDeathRecipient());
    CHKPR(deathRecipient_, ERR_NO_MEMORY);

    if (remoteObject->IsProxyObject()) {
        if (!remoteObject->AddDeathRecipient(deathRecipient_)) {
            FI_HILOGE("Add death recipient to DeviceStatus service failed");
            return E_DEVICESTATUS_ADD_DEATH_RECIPIENT_FAILED;
        }
    }

    devicestatusProxy_ = iface_cast<IIntention>(remoteObject);
    FI_HILOGD("Connecting IntentionService success");
    return RET_OK;
}

void IntentionClient::ResetProxy(const wptr<IRemoteObject>& remote)
{
    CALL_DEBUG_ENTER;
    std::lock_guard<std::mutex> lock(mutex_);
    CHKPV(devicestatusProxy_);
    auto serviceRemote = devicestatusProxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        devicestatusProxy_ = nullptr;
    }
    if (deathListener_ != nullptr) {
        FI_HILOGI("Notify death listener");
        deathListener_();
    }
}

void IntentionClient::IntentionDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    CALL_DEBUG_ENTER;
    CHKPV(remote);
    IntentionClient::GetInstance().ResetProxy(remote);
    FI_HILOGD("Recv death notice");
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
