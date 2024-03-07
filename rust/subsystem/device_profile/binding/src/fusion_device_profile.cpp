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

#include "fusion_device_profile.h"

#include <sstream>

#include "distributed_device_profile_client.h"

#include "devicestatus_define.h"

using namespace OHOS::DeviceProfile;

#undef LOG_TAG
#define LOG_TAG "FusionDeviceProfile"

namespace {
const std::string SERVICE_ID { "deviceStatus" };
} // namespace

class ProfileEventCallbackImpl final : public IProfileEventCallback {
public:
    explicit ProfileEventCallbackImpl(CIProfileEventCb* eventCb);
    ~ProfileEventCallbackImpl();

    void OnProfileChanged(const ProfileChangeNotification &changeNotification) override;

private:
    CIProfileEventCb* eventCb_ { nullptr };
};

ProfileEventCallbackImpl::ProfileEventCallbackImpl(CIProfileEventCb* eventCb)
{
    if ((eventCb != nullptr) && (eventCb->clone != nullptr)) {
        eventCb_ = eventCb->clone(eventCb);
    }
}

ProfileEventCallbackImpl::~ProfileEventCallbackImpl()
{
    if (eventCb_ != nullptr && eventCb_->destruct != nullptr) {
        eventCb_->destruct(eventCb_);
    }
}

void ProfileEventCallbackImpl::OnProfileChanged(const ProfileChangeNotification &changeNotification)
{
    CALL_INFO_TRACE;
}

static void Destruct(CIProfileEvents* target)
{
    CHKPV(target);
    if (target->profileEvents == nullptr) {
        delete target;
    } else {
        delete [] target->profileEvents;
        delete target;
    }
}

int32_t PutDeviceProfile(const CServiceCharacteristicProfile* profile)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t GetDeviceProfile(const char* udId, const char* serviceId, CServiceCharacteristicProfile* profile)
{
    CALL_DEBUG_ENTER;
    return RET_ERR;
}

int32_t SubscribeProfileEvents(const CISubscribeInfos* subscribeInfos,
                               CIProfileEventCb* eventCb,
                               CIProfileEvents** failedEvents)
{
    CALL_DEBUG_ENTER;
    CHKPR(subscribeInfos, RET_ERR);
    CHKPR(subscribeInfos->subscribeInfos, RET_ERR);
    std::list<SubscribeInfo> subscriptions;

    for (size_t index = 0; index < subscribeInfos->nSubscribeInfos; ++index) {
        const CSubscribeInfo &cSub = subscribeInfos->subscribeInfos[index];

        if ((cSub.profileEvent >= ProfileEvent::EVENT_UNKNOWN) &&
            (cSub.profileEvent < ProfileEvent::EVENT_PROFILE_END)) {
            CHKPC(cSub.extraInfo);
            SubscribeInfo subscription;
            subscription.profileEvent = static_cast<ProfileEvent>(cSub.profileEvent);
            subscription.extraInfo = nlohmann::json::parse(cSub.extraInfo, nullptr, false);
            subscriptions.push_back(subscription);
        }
    }

    auto callback = std::make_shared<ProfileEventCallbackImpl>(eventCb);
    std::list<ProfileEvent> fails;

    int32_t ret = DistributedDeviceProfileClient::GetInstance().SubscribeProfileEvents(
        subscriptions, callback, fails);

    if (!fails.empty()) {
        CIProfileEvents* events = new (std::nothrow) CIProfileEvents;
        CHKPR(events, RET_ERR);
        events->numOfProfileEvents = fails.size();
        events->profileEvents = new (std::nothrow) uint32_t[fails.size()];
        if (events->profileEvents == nullptr) {
            delete events;
            FI_HILOGE("Failed to allocate memory for profileEvents");
            return RET_ERR;
        }
        events->clone = nullptr;
        events->destruct = &Destruct;

        size_t index = 0;
        for (const auto &profile_event: fails) {
            events->profileEvents[index++] = profile_event;
        }
        *failedEvents = events;
        events->destruct(events);
    } else {
        *failedEvents = nullptr;
    }
    return ret;
}

int32_t UnsubscribeProfileEvents(const CIProfileEvents* profileEvents,
                                 CIProfileEventCb* eventCb,
                                 CIProfileEvents** failedEvents)
{
    CALL_DEBUG_ENTER;
    CHKPR(profileEvents, RET_ERR);
    std::list<ProfileEvent> profiles;

    for (size_t index = 0; index < profileEvents->numOfProfileEvents; ++index) {
        uint32_t cPro = profileEvents->profileEvents[index];
        if ((cPro >= static_cast<uint32_t>(ProfileEvent::EVENT_UNKNOWN)) &&
            (cPro < static_cast<uint32_t>(ProfileEvent::EVENT_PROFILE_END))) {
            ProfileEvent profile = static_cast<ProfileEvent>(cPro);
            profiles.push_back(profile);
        }
    }

    auto callback = std::make_shared<ProfileEventCallbackImpl>(eventCb);
    std::list<ProfileEvent> fails;

    int32_t ret = DistributedDeviceProfileClient::GetInstance().UnsubscribeProfileEvents(
        profiles, callback, fails);

    if (!fails.empty()) {
        CIProfileEvents* events = new (std::nothrow) CIProfileEvents;
        CHKPR(events, RET_ERR);
        events->numOfProfileEvents = fails.size();
        events->profileEvents = new (std::nothrow) uint32_t[fails.size()];
        if (events->profileEvents == nullptr) {
            delete events;
            FI_HILOGE("Failed to allocate memory for profileEvents");
            return RET_ERR;
        }
        events->clone = nullptr;
        events->destruct = &Destruct;

        size_t index = 0;
        for (const auto &profile_event: fails) {
            events->profileEvents[index++] = profile_event;
        }
        *failedEvents = events;
        events->destruct(events);
    } else {
        *failedEvents = nullptr;
    }
    return ret;
}

