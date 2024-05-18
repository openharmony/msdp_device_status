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

#include "drag_smooth_processor.h"

#include <utility>

#include "devicestatus_common.h"
#include "include/util.h"

#undef LOG_TAG
#define LOG_TAG "DragSmoothProcessor"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr uint64_t ONE_MS_IN_NS { 1 * 1000 * 1000 }; // 1ms
constexpr int32_t RESAMPLE_COORD_TIME_THRESHOLD { 20 * 1000 * 1000 };  // 20ms
constexpr uint64_t INTERPOLATION_THRESHOLD { 100 * 1000 * 1000 }; // 100ms
}
void DragSmoothProcessor::InsertEvent(DragMoveEvent &event)
{
    std::lock_guard<std::mutex> lock(mtx_);
    moveEvents_.emplace_back(event);
}

DragMoveEvent DragSmoothProcessor::SmoothMoveEvent(uint64_t nanoTimestamp, uint64_t vSyncPeriod)
{
    resampleTimeStamp_ = nanoTimestamp - vSyncPeriod + ONE_MS_IN_NS;
    auto targetTimeStamp = resampleTimeStamp_;
    FI_HILOGD("VSync nanoTimestamp:%{public}" PRId64 ", vSync period:%{public}" PRId64
        ", resample timeStamp:%{public}" PRId64, nanoTimestamp, vSyncPeriod, targetTimeStamp);
    std::vector<DragMoveEvent> currentEvents;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        currentEvents.swap(moveEvents_);
    }
    DragMoveEvent latestEvent = currentEvents.back();
    auto resampleEvent = GetResampleEvent(lastEvents_, currentEvents, targetTimeStamp);
    if (resampleEvent) {
        latestEvent = resampleEvent.value();
    }
    lastEvents_ = currentEvents;
    return latestEvent;
}

void DragSmoothProcessor::ResetParameters()
{
    std::lock_guard<std::mutex> lock(mtx_);
    moveEvents_.erase(moveEvents_.begin(), moveEvents_.end());
    lastEvents_.erase(lastEvents_.begin(), lastEvents_.end());
    resampleTimeStamp_ = 0;
}

std::optional<DragMoveEvent> DragSmoothProcessor::GetResampleEvent(const std::vector<DragMoveEvent>& history,
    const std::vector<DragMoveEvent>& current, uint64_t nanoTimestamp)
{
    auto event = Resample(history, current, nanoTimestamp);
    DragMoveEvent nearestEvent = GetNearestEvent(current, nanoTimestamp);
    if (event.has_value()) {
        nearestEvent = event.value();
    }
    return nearestEvent;
}

DragMoveEvent DragSmoothProcessor::GetNearestEvent(const std::vector<DragMoveEvent>& events, uint64_t nanoTimestamp)
{
    DragMoveEvent nearestEvent;
    uint64_t gap = UINT64_MAX;
    for (auto &event : events) {
        if (event.timeStamp == nanoTimestamp) {
            nearestEvent = event;
            return nearestEvent;
        }
        if (event.timeStamp > nanoTimestamp) {
            if (event.timeStamp - nanoTimestamp < gap) {
                gap = event.timeStamp - nanoTimestamp;
                nearestEvent = event;
            }
        } else {
            if (nanoTimestamp - event.timeStamp < gap) {
                gap = nanoTimestamp - event.timeStamp;
                nearestEvent = event;
            }
        }
    }
    return nearestEvent;
}

std::optional<DragMoveEvent> DragSmoothProcessor::Resample(const std::vector<DragMoveEvent>& history,
    const std::vector<DragMoveEvent>& current, uint64_t nanoTimestamp)
{
    if (history.empty() || current.empty()) {
        FI_HILOGW("history or current is empty, history size:%{public}zu, current size:%{public}zu,"
            "nanoTimestamp:%{public}" PRId64, history.size(), current.size(), nanoTimestamp);
        return std::nullopt;
    }
    DragMoveEvent latestEvent;
    for (auto &event : current) {
        if (latestEvent.timeStamp < event.timeStamp) {
            latestEvent = event;
        }
    }
    if (nanoTimestamp > RESAMPLE_COORD_TIME_THRESHOLD + latestEvent.timeStamp) {
        FI_HILOGW("latestEvent is beyond the sampling range, use this this latest event, x:%{public}f, "
            "y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d, sampling nanoTimestamp:%{public}" PRId64,
            latestEvent.displayX, latestEvent.displayY, latestEvent.timeStamp, latestEvent.displayId, nanoTimestamp);
        return latestEvent;
    }
    auto historyAvgEvent = GetAvgCoordinate(history);
    auto currentAvgEvent = GetAvgCoordinate(current);
    DumpMoveEvent(history, current, historyAvgEvent, currentAvgEvent, latestEvent);
    return LinearInterpolation(historyAvgEvent, currentAvgEvent, nanoTimestamp);
}

std::optional<DragMoveEvent> DragSmoothProcessor::LinearInterpolation(DragMoveEvent &historyAvgEvent,
    DragMoveEvent &currentAvgEvent, uint64_t nanoTimestamp)
{
    if ((nanoTimestamp <= historyAvgEvent.timeStamp) || (nanoTimestamp == currentAvgEvent.timeStamp) ||
        (currentAvgEvent.timeStamp <= historyAvgEvent.timeStamp) ||
        ((currentAvgEvent.timeStamp - historyAvgEvent.timeStamp) > INTERPOLATION_THRESHOLD)) {
            FI_HILOGW("No need linear interpolation, historyAvgEvent x:%{public}f, "
            "y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d, currentAvgEvent x:%{public}f"
            "y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d, nanoTimestamp: %{public}" PRId64,
            historyAvgEvent.displayX, historyAvgEvent.displayY, historyAvgEvent.timeStamp, historyAvgEvent.displayId,
            currentAvgEvent.displayX, currentAvgEvent.displayY, currentAvgEvent.timeStamp, currentAvgEvent.displayId,
            nanoTimestamp);
        return std::nullopt;
    }
    DragMoveEvent event;
    if (nanoTimestamp < currentAvgEvent.timeStamp) {
        float alpha = static_cast<float>(nanoTimestamp - historyAvgEvent.timeStamp) /
            static_cast<float>(currentAvgEvent.timeStamp - historyAvgEvent.timeStamp);
        event.displayX = historyAvgEvent.displayX + alpha * (currentAvgEvent.displayX - historyAvgEvent.displayX);
        event.displayY = historyAvgEvent.displayY + alpha * (currentAvgEvent.displayY - historyAvgEvent.displayY);
        event.timeStamp = nanoTimestamp;
        event.displayId = currentAvgEvent.displayId;
    } else if (nanoTimestamp > currentAvgEvent.timeStamp) {
        float alpha = static_cast<float>(nanoTimestamp - currentAvgEvent.timeStamp) /
            static_cast<float>(currentAvgEvent.timeStamp - historyAvgEvent.timeStamp);
        event.displayX = currentAvgEvent.displayX + alpha * (currentAvgEvent.displayX - historyAvgEvent.displayX);
        event.displayY = currentAvgEvent.displayY + alpha * (currentAvgEvent.displayY - historyAvgEvent.displayY);
        event.timeStamp = nanoTimestamp;
        event.displayId = currentAvgEvent.displayId;
    }
    return event;
}

void DragSmoothProcessor::DumpMoveEvent(const std::vector<DragMoveEvent>& history,
    const std::vector<DragMoveEvent>& current, const DragMoveEvent &historyAvgEvent,
    const DragMoveEvent &currentAvgEvent, const DragMoveEvent &latestEvent)
{
    for (auto &event : history) {
        FI_HILOGD("history event, x:%{public}f, y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d",
            event.displayX, event.displayY, event.timeStamp, event.displayId);
    }
    for (auto &event : current) {
        FI_HILOGD("current event, x:%{public}f, y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d",
            event.displayX, event.displayY, event.timeStamp, event.displayId);
    }
    FI_HILOGD("history average event, x:%{public}f, y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d",
        historyAvgEvent.displayX, historyAvgEvent.displayY, historyAvgEvent.timeStamp, historyAvgEvent.displayId);
    FI_HILOGD("current average event, x:%{public}f, y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d",
        currentAvgEvent.displayX, currentAvgEvent.displayY, currentAvgEvent.timeStamp, currentAvgEvent.displayId);
    FI_HILOGD("latest event, x:%{public}f, y:%{public}f, timeStamp:%{public}" PRId64 "displayId:%{public}d",
        latestEvent.displayX, latestEvent.displayY, latestEvent.timeStamp, latestEvent.displayId);
}

DragMoveEvent DragSmoothProcessor::GetAvgCoordinate(const std::vector<DragMoveEvent>& events)
{
    DragMoveEvent avgEvent;
    int32_t i = 0;
    uint64_t lastTime = 0;
    for (auto &event : events) {
        if ((lastTime == 0) || (lastTime != event.timeStamp)) {
            avgEvent.displayX += event.displayX;
            avgEvent.displayY += event.displayY;
            avgEvent.timeStamp += event.timeStamp;
            avgEvent.displayId = event.displayId;
            i++;
            lastTime = event.timeStamp;
        }
    }
    if (i > 0) {
        avgEvent.displayX /= i;
        avgEvent.displayY /= i;
        avgEvent.timeStamp /= i;
    }
    return avgEvent;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS