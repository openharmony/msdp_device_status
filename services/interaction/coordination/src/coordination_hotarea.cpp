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

#include "coordination_hotarea.h"

#include "coordination_sm.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "CoordinationHotArea"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
std::shared_ptr<CoordinationHotArea> g_instance = nullptr;
constexpr int32_t HOT_AREA_WIDTH { 100 };
constexpr int32_t HOT_AREA_MARGIN { 200 };
}; // namespace

std::shared_ptr<CoordinationHotArea> CoordinationHotArea::GetInstance()
{
    static std::once_flag flag;
    std::call_once(flag, [&]() {
        CoordinationHotArea *cooContext = new (std::nothrow) CoordinationHotArea();
        CHKPL(cooContext);
        g_instance.reset(cooContext);
    });
    return g_instance;
}

void CoordinationHotArea::AddHotAreaListener(sptr<HotAreaInfo> event)
{
    CALL_DEBUG_ENTER;
    CHKPV(event);
    auto it = std::find_if(hotAreaCallbacks_.begin(), hotAreaCallbacks_.end(),
        [event](auto info) {
            return (*info).sess == event->sess;
        });
    if (it != hotAreaCallbacks_.end()) {
        *it = event;
        return;
    }
    hotAreaCallbacks_.emplace_back(event);
}

void CoordinationHotArea::RemoveHotAreaListener(sptr<HotAreaInfo> event)
{
    CALL_DEBUG_ENTER;
    if (hotAreaCallbacks_.empty() || event == nullptr) {
        FI_HILOGE("Failed to remove hot area listener");
        return;
    }
    for (auto it = hotAreaCallbacks_.begin(); it != hotAreaCallbacks_.end(); ++it) {
        if ((*it)->sess == event->sess) {
            hotAreaCallbacks_.erase(it);
            return;
        }
    }
}

int32_t CoordinationHotArea::OnHotAreaMessage(HotAreaType msg, bool isEdge)
{
    CALL_DEBUG_ENTER;
    if (hotAreaCallbacks_.empty()) {
        FI_HILOGW("Failed to invoke the listening interface, hotAreaCallbacks_ is empty");
        return RET_ERR;
    }
    for (auto it = hotAreaCallbacks_.begin(); it != hotAreaCallbacks_.end(); ++it) {
        sptr<HotAreaInfo> info = *it;
        CHKPC(info);
        NotifyHotAreaMessage(info->sess, info->msgId, msg, isEdge);
    }
    return RET_OK;
}

int32_t CoordinationHotArea::ProcessData(std::shared_ptr<MMI::PointerEvent> pointerEvent)
{
    CALL_DEBUG_ENTER;
    if (hotAreaCallbacks_.empty()) {
        return RET_ERR;
    }
    CHKPR(pointerEvent, RET_ERR);
    MMI::PointerEvent::PointerItem pointerItem;
    pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem);
    displayX_ = pointerItem.GetDisplayX();
    displayY_ = pointerItem.GetDisplayY();
    deltaX_ = pointerItem.GetRawDx();
    deltaY_ = pointerItem.GetRawDy();
    CheckInHotArea();
    CheckPointerToEdge(type_);
    NotifyMessage();
    return RET_OK;
}

void CoordinationHotArea::CheckInHotArea()
{
    CALL_DEBUG_ENTER;
    if (displayX_ <= HOT_AREA_WIDTH && displayY_ >= HOT_AREA_MARGIN &&
        displayY_ <= (height_ - HOT_AREA_MARGIN)) {
        type_ = HotAreaType::AREA_LEFT;
    } else if (displayX_ >= (width_ - HOT_AREA_WIDTH) && displayY_ >= HOT_AREA_MARGIN &&
        displayY_ <= (height_ - HOT_AREA_MARGIN)) {
        type_ = HotAreaType::AREA_RIGHT;
    } else if (displayY_ <= HOT_AREA_WIDTH && displayX_ >= HOT_AREA_MARGIN &&
        displayX_ <= (width_ - HOT_AREA_MARGIN)) {
        type_ = HotAreaType::AREA_TOP;
    } else if (displayY_ >= (height_ - HOT_AREA_WIDTH) && displayX_ >= HOT_AREA_MARGIN &&
        displayX_ <= (width_ - HOT_AREA_MARGIN)) {
        type_ = HotAreaType::AREA_BOTTOM;
    } else {
        type_ = HotAreaType::AREA_NONE;
    }
}

void CoordinationHotArea::CheckPointerToEdge(HotAreaType type)
{
    CALL_DEBUG_ENTER;
    if (type == HotAreaType::AREA_LEFT) {
        isEdge_ = displayX_ <= 0 && deltaX_ < 0;
    } else if (type == HotAreaType::AREA_RIGHT) {
        isEdge_ = displayX_ >= (width_ - 1) && deltaX_ > 0;
    } else if (type == HotAreaType::AREA_TOP) {
        isEdge_ = displayY_ <= 0 && deltaY_ < 0;
    } else if (type == HotAreaType::AREA_BOTTOM) {
        isEdge_ = displayY_ >= (height_ - 1) && deltaY_ > 0;
    } else {
        isEdge_ = false;
    }
}

void CoordinationHotArea::NotifyHotAreaMessage(SessionPtr sess, MessageId msgId, HotAreaType msg, bool isEdge)
{
    CALL_DEBUG_ENTER;
    CHKPV(sess);
    NetPacket pkt(msgId);
    pkt << displayX_ << displayY_ << static_cast<int32_t>(msg) << isEdge;
    if (pkt.ChkRWError()) {
        FI_HILOGE("Packet write data failed");
        return;
    }
    if (!sess->SendMsg(pkt)) {
        FI_HILOGE("Sending failed");
        return;
    }
}

void CoordinationHotArea::SetWidth(int32_t screenWidth)
{
    width_ = screenWidth;
}

void CoordinationHotArea::SetHeight(int32_t screenHight)
{
    height_ = screenHight;
}

void CoordinationHotArea::NotifyMessage()
{
    CALL_DEBUG_ENTER;
    OnHotAreaMessage(type_, isEdge_);
}
} // DeviceStatus
} // Msdp
} // OHOS