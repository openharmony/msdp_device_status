/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "timer_manager.h"

#include <sys/timerfd.h>

#include "devicestatus_hilog_wrapper.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr int32_t MIN_DELAY = -1;
constexpr int32_t MIN_INTERVAL = 50;
constexpr int32_t MAX_INTERVAL_MS = 10000;
constexpr int32_t MAX_TIMER_COUNT = 64;
constexpr int32_t NONEXISTENT_ID = -1;
} // namespace

int32_t TimerManager::Init()
{
    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (timerFd_ < 0) {
        DEV_HILOGE(SERVICE, "timer: timerfd_create failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t TimerManager::AddTimer(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback)
{
    int32_t timerId = AddTimerInternal(intervalMs, repeatCount, callback);
    if (timerId != NONEXISTENT_ID) {
        ArmTimer();
    }
    return timerId;
}

int32_t TimerManager::RemoveTimer(int32_t timerId)
{
    int32_t ret = RemoveTimerInternal(timerId);
    if (ret != RET_OK) {
        ArmTimer();
    }
    return ret;
}

int32_t TimerManager::ResetTimer(int32_t timerId)
{
    int32_t ret = ResetTimerInternal(timerId);
    ArmTimer();
    return ret;
}

bool TimerManager::IsExist(int32_t timerId)
{
    return IsExistInternal(timerId);
}

int64_t TimerManager::CalcNextDelay()
{
    return CalcNextDelayInternal();
}

void TimerManager::ProcessTimers()
{
    ProcessTimersInternal();
    ArmTimer();
}

int32_t TimerManager::TakeNextTimerId()
{
    uint64_t timerSlot = 0;
    uint64_t one = 1;

    for (const auto &timer : timers_) {
        timerSlot |= (one << timer->id);
    }

    for (int32_t i = 0; i < MAX_TIMER_COUNT; ++i) {
        if ((timerSlot & (one << i)) == 0) {
            return i;
        }
    }
    return NONEXISTENT_ID;
}

int32_t TimerManager::AddTimerInternal(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback)
{
    if (intervalMs < MIN_INTERVAL) {
        intervalMs = MIN_INTERVAL;
    } else if (intervalMs > MAX_INTERVAL_MS) {
        intervalMs = MAX_INTERVAL_MS;
    }
    if (!callback) {
        return NONEXISTENT_ID;
    }
    int32_t timerId = TakeNextTimerId();
    if (timerId < 0) {
        return NONEXISTENT_ID;
    }
    auto timer = std::make_unique<TimerItem>();
    timer->id = timerId;
    timer->intervalMs = intervalMs;
    timer->repeatCount = repeatCount;
    timer->callbackCount = 0;
    auto nowTime = GetMillisTime();
    if (!AddInt64(nowTime, timer->intervalMs, timer->nextCallTime)) {
        DEV_HILOGE(SERVICE, "The addition of nextCallTime in TimerItem overflows");
        return NONEXISTENT_ID;
    }
    timer->callback = callback;
    InsertTimerInternal(timer);
    return timerId;
}

int32_t TimerManager::RemoveTimerInternal(int32_t timerId)
{
    for (auto tIter = timers_.begin(); tIter != timers_.end(); ++tIter) {
        if ((*tIter)->id == timerId) {
            timers_.erase(tIter);
            return RET_OK;
        }
    }
    return RET_ERR;
}

int32_t TimerManager::ResetTimerInternal(int32_t timerId)
{
    for (auto tIter = timers_.begin(); tIter != timers_.end(); ++tIter) {
        if ((*tIter)->id == timerId) {
            auto timer = std::move(*tIter);
            timers_.erase(tIter);
            auto nowTime = GetMillisTime();
            if (!AddInt64(nowTime, timer->intervalMs, timer->nextCallTime)) {
                DEV_HILOGE(SERVICE, "The addition of nextCallTime in TimerItem overflows");
                return RET_ERR;
            }
            timer->callbackCount = 0;
            InsertTimerInternal(timer);
            return RET_OK;
        }
    }
    return RET_ERR;
}

bool TimerManager::IsExistInternal(int32_t timerId)
{
    for (auto tIter = timers_.begin(); tIter != timers_.end(); ++tIter) {
        if ((*tIter)->id == timerId) {
            return true;
        }
    }
    return false;
}

void TimerManager::InsertTimerInternal(std::unique_ptr<TimerItem>& timer)
{
    for (auto tIter = timers_.begin(); tIter != timers_.end(); ++tIter) {
        if ((*tIter)->nextCallTime > timer->nextCallTime) {
            timers_.insert(tIter, std::move(timer));
            return;
        }
    }
    timers_.push_back(std::move(timer));
}

int64_t TimerManager::CalcNextDelayInternal()
{
    auto delay = MIN_DELAY;
    if (!timers_.empty()) {
        auto nowTime = GetMillisTime();
        const auto& item = *timers_.begin();
        if (nowTime >= item->nextCallTime) {
            delay = 0;
        } else {
            delay = item->nextCallTime - nowTime;
        }
    }
    return delay;
}

void TimerManager::ProcessTimersInternal()
{
    if (timers_.empty()) {
        return;
    }
    auto nowTime = GetMillisTime();
    for (;;) {
        auto tIter = timers_.begin();
        if (tIter == timers_.end()) {
            break;
        }
        if ((*tIter)->nextCallTime > nowTime) {
            break;
        }
        auto curTimer = std::move(*tIter);
        timers_.erase(tIter);
        ++curTimer->callbackCount;
        if ((curTimer->repeatCount >= 1) && (curTimer->callbackCount >= curTimer->repeatCount)) {
            curTimer->callback();
            continue;
        }
        if (!AddInt64(curTimer->nextCallTime, curTimer->intervalMs, curTimer->nextCallTime)) {
            DEV_HILOGE(SERVICE, "The addition of nextCallTime in TimerItem overflows");
            return;
        }
        auto callback = curTimer->callback;
        InsertTimerInternal(curTimer);
        callback();
    }
}

int32_t TimerManager::ArmTimer()
{
    if (timerFd_ < 0) {
        DEV_HILOGE(SERVICE, "Timer has not been created");
        return RET_ERR;
    }
    struct itimerspec tspec {};
    int64_t expire = CalcNextDelayInternal();

    if (expire == 0) {
        expire = 1;
    }
    if (expire > 0) {
        tspec.it_value.tv_sec = expire / 1000;
        tspec.it_value.tv_nsec = expire * 1000000;
    }

    if (timerfd_settime(timerFd_, 0, &tspec, NULL) != 0) {
        DEV_HILOGE(SERVICE, "timer: timerfd_settime error");
        return RET_ERR;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
