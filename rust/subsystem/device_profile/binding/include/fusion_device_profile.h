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

#ifndef FUSION_DEVICE_PROFILE_H
#define FUSION_DEVICE_PROFILE_H

#include <cstddef>
#include <cstdint>

struct CServiceCharacteristicProfile {
    const char* serviceId;
    const char* serviceType;
    const char* serviceProfileJson;
    const char* characteristicProfileJson;
};

struct CIProfileEvents {
    CIProfileEvents* (*clone)(CIProfileEvents* cb);
    void (*destruct)(CIProfileEvents* cb);
    uint32_t* profileEvents;
    size_t numOfProfileEvents;
};

struct CSubscribeInfo {
    uint32_t profileEvent;
    const char* extraInfo;
};

struct CISubscribeInfos {
    const CSubscribeInfo* subscribeInfos;
    size_t nSubscribeInfos;
};

enum CProfileChangeType: int32_t {
    UNKNOWN_CHANGE_TYPE,
    INSERTED,
    UPDATED,
    DELETED,
};

struct CProfileEntry {
    const char* key;
    const char* value;
    CProfileChangeType changeType;
    const CProfileEntry* next;
};

struct CProfileChangeNotification {
    const CProfileEntry* profileEntries;
    size_t nProfileEntries;
    const char* deviceId;
    int32_t localFlag;
};

struct CIProfileEventCb {
    CIProfileEventCb* (*clone)(CIProfileEventCb* cb);
    void (*destruct)(CIProfileEventCb* cb);
    void (*onProfileChanged)(CIProfileEventCb* cb, const CProfileChangeNotification* notification);
};

enum CSyncMode : int32_t {
    PULL,
    PUSH,
    PUSH_PULL,
};

struct CSyncOptions {
    CSyncMode syncMode;
    const char *const* deviceIds;
    size_t nDeviceIds;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int32_t PutDeviceProfile(const CServiceCharacteristicProfile* profile);
int32_t GetDeviceProfile(const char* udId, const char* serviceId, CServiceCharacteristicProfile* profile);
int32_t SubscribeProfileEvents(const CISubscribeInfos* subscribeInfos,
                               CIProfileEventCb* eventCb,
                               CIProfileEvents** failedEvents);
int32_t UnsubscribeProfileEvents(const CIProfileEvents* profileEvents,
                                 CIProfileEventCb* eventCb,
                                 CIProfileEvents** failedEvents);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // FUSION_DEVICE_PROFILE_H
