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

// Implementation of client side of IPC.

#ifndef INTENTION_CLIENT_H
#define INTENTION_CLIENT_H

#include <singleton.h>

#include "i_plugin.h"
#include "i_intention.h"
#include "intention_proxy.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IntentionClient final : public DelayedRefSingleton<IntentionClient> {
    DECLARE_DELAYED_REF_SINGLETON(IntentionClient)

public:
    DISALLOW_COPY_AND_MOVE(IntentionClient);

    // Request to enable the service identified by [`intention`].
    int32_t Enable(uint32_t intention, ParamBase &data, ParamBase &reply);
    // Request to disable the service identified by [`intention`].
    int32_t Disable(uint32_t intention, ParamBase &data, ParamBase &reply);
    // Request to start the service identified by [`intention`].
    int32_t Start(uint32_t intention, ParamBase &data, ParamBase &reply);
    // Request to stop the service identified by [`intention`].
    int32_t Stop(uint32_t intention, ParamBase &data, ParamBase &reply);
    // Request to add a watch of state of service, with the service identified by
    // [`intention`], the state to watch identified by [`id`], parameters packed in
    // [`data`] parcel.
    int32_t AddWatch(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply);
    // Request to remove a watch of state of service.
    int32_t RemoveWatch(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply);
    // Request to set a parameter of service, with the service identified by
    // [`intention`], the parameter identified by [`id`], and values packed in
    // [`data`] parcel.
    int32_t SetParam(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply);
    // Request to get a parameter of service, with the service identified by
    // [`intention`], the parameter identified by [`id`].
    int32_t GetParam(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply);
    // Request to interact with service identified by [`intention`] for general purpose.
    // This interface supplements functions of previous intefaces. Functionalities of
    // this interface is service spicific.
    int32_t Control(uint32_t intention, uint32_t id, ParamBase &data, ParamBase &reply);

private:
    class IntentionDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        IntentionDeathRecipient() = default;
        ~IntentionDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote);

    private:
        DISALLOW_COPY_AND_MOVE(IntentionDeathRecipient);
    };
    ErrCode Connect();
    void ResetProxy(const wptr<IRemoteObject>& remote);

private:
    std::mutex mutex_;
    sptr<IIntention> devicestatusProxy_ { nullptr };
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ { nullptr };
    std::function<void()> deathListener_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // INTENTION_CLIENT_H
