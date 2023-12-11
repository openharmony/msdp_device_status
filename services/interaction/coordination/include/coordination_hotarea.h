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

#ifndef COORDINATION_HOTAREA_H
#define COORDINATION_HOTAREA_H

#include <list>
#include "refbase.h"
#include "stream_session.h"

#include "coordination_message.h"
#include "devicestatus_define.h"
#include "input_manager.h"
#include "proto.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class CoordinationHotArea {
public:
    struct HotAreaInfo : public RefBase {
        SessionPtr sess { nullptr };
        MessageId msgId { MessageId::INVALID };
        HotAreaType msg { HotAreaType::AREA_NONE };
        bool isEdge { false };
    };

    CoordinationHotArea() = default;
    ~CoordinationHotArea() = default;
    static std::shared_ptr<CoordinationHotArea> GetInstance();
    void AddHotAreaListener(sptr<HotAreaInfo> event);
    void RemoveHotAreaListener(sptr<HotAreaInfo> event);
    int32_t OnHotAreaMessage(HotAreaType msg, bool isEdge);
    int32_t ProcessData(std::shared_ptr<MMI::PointerEvent> pointerEvent);
    void SetWidth(int32_t deviceWidth);
    void SetHeight(int32_t deviceHight);
    void NotifyMessage();

private:
    void CheckInHotArea();
    void CheckPointerToEdge(HotAreaType type);
    void NotifyHotAreaMessage(SessionPtr sess, MessageId msgId, HotAreaType msg, bool isEdge);

private:
    int32_t width_ { 720 };
    int32_t height_ { 1280 };
    int32_t displayX_ { 0 };
    int32_t displayY_ { 0 };
    int32_t deltaX_ { 0 };
    int32_t deltaY_ { 0 };
    bool isEdge_ { false };
    HotAreaType type_ { HotAreaType::AREA_NONE };
    std::list<sptr<HotAreaInfo>> hotAreaCallbacks_;
};

#define HOT_AREA OHOS::DelayedSingleton<CoordinationHotArea>::GetInstance()
} // DeviceStatus
} // Msdp
} // OHOS
#endif // COORDINATION_HOTAREA_H