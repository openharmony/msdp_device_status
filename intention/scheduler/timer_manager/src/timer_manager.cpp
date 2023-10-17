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

#include "timer_manager.h"

#include <numeric>

#include <sys/timerfd.h>

#include "fi_log.h"
#include "devicestatus_define.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "TimerManager" };
constexpr int32_t MIN_DELAY { -1 };
constexpr int32_t MIN_INTERVAL { 50 };
constexpr int32_t MAX_INTERVAL_MS { 10000 };
constexpr int32_t NONEXISTENT_ID { -1 };
constexpr int32_t TIME_CONVERSION { 1000 };
constexpr size_t MAX_TIMER_COUNT { 64 };
} // namespace

int32_t TimerManager::Init(IContext *context)
{
    CHKPR(context, RET_ERR);
    return context->GetTaskScheduler().PostSyncTask(std::bind(&TimerManager::OnInit, this, context));
}

int32_t TimerManager::OnInit(IContext *context)
{
    CHKPR(context, RET_ERR);
    context_ = context;

    timerFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (timerFd_ < 0) {
        FI_HILOGE("Timer: timerfd_create failed");
        return RET_ERR;
    }
    return RET_OK;
}

int32_t TimerManager::AddTimer(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback)
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    return context_->GetTaskScheduler().PostSyncTask(
        std::bind(&TimerManager::OnAddTimer, this, intervalMs, repeatCount, callback));
}

int32_t TimerManager::OnAddTimer(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback)
{
    int32_t timerId = AddTimerInternal(intervalMs, repeatCount, callback);
    ArmTimer();
    return timerId;
}

int32_t TimerManager::RemoveTimer(int32_t timerId)
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    return context_->GetTaskScheduler().PostSyncTask(std::bind(&TimerManager::OnRemoveTimer, this, timerId));
}

int32_t TimerManager::OnRemoveTimer(int32_t timerId)
{
    int32_t ret = RemoveTimerInternal(timerId);
    if (ret == RET_OK) {
        ArmTimer();
    }
    return ret;
}

int32_t TimerManager::ResetTimer(int32_t timerId)
{
    CALL_INFO_TRACE;
    CHKPR(context_, RET_ERR);
    return context_->GetTaskScheduler().PostSyncTask(std::bind(&TimerManager::OnResetTimer, this, timerId));
}

int32_t TimerManager::OnResetTimer(int32_t timerId)
{
    int32_t ret = ResetTimerInternal(timerId);
    ArmTimer();
    return ret;
}

bool TimerManager::IsExist(int32_t timerId) const
{
    CHKPR(context_, false);
    std::packaged_task<bool(int32_t)> task { std::bind(&TimerManager::OnIsExist, this, std::placeholders::_1) };
    auto fu = task.get_future();

    int32_t ret = context_->GetTaskScheduler().PostSyncTask(
        std::bind(&TimerManager::RunIsExist, this, std::ref(task), timerId));
    if (ret != RET_OK) {
        FI_HILOGE("Post task failed");
        return false;
    }
    return fu.get();
}

bool TimerManager::OnIsExist(int32_t timerId) const
{
    for (auto tIter = timers_.begin(); tIter != timers_.end(); ++tIter) {
        if ((*tIter)->id == timerId) {
            return true;
        }
    }
    return false;
}

int32_t TimerManager::RunIsExist(std::packaged_task<bool(int32_t)> &task, int32_t timerId) const
{
    task(timerId);
    return RET_OK;
}

void TimerManager::ProcessTimers()
{
    CALL_INFO_TRACE;
    CHKPV(context_);
    context_->GetTaskScheduler().PostAsyncTask(std::bind(&TimerManager::OnProcessTimers, this));
}

int32_t TimerManager::OnProcessTimers()
{
    ProcessTimersInternal();
    ArmTimer();
    return RET_OK;
}

int32_t TimerManager::TakeNextTimerId()
{
    uint64_t timerSlot = std::accumulate(timers_.cbegin(), timers_.cend(), uint64_t(0U),
        [] (uint64_t s, const auto &timer) {
            return (s |= (uint64_t(1U) << timer->id));
        });
    for (size_t i = 0; i < MAX_TIMER_COUNT; ++i) {
        if ((timerSlot & (uint64_t(1U) << i)) == 0) {
            return i;
        }
    }
    return NONEXISTENT_ID;
}

int32_t TimerManager::AddTimerInternal(int32_t intervalMs, int32_t repeatCount, std::function<void()> callback)
{
    CALL_INFO_TRACE;
    if (intervalMs < MIN_INTERVAL) {
        intervalMs = MIN_INTERVAL;
    } else if (intervalMs > MAX_INTERVAL_MS) {
        intervalMs = MAX_INTERVAL_MS;
    }
    if (callback == nullptr) {
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
    int64_t nowTime = GetMillisTime();
    if (!AddInt64(nowTime, timer->intervalMs, timer->nextCallTime)) {
        FI_HILOGE("The addition of nextCallTime in TimerItem overflows");
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
            int64_t nowTime = GetMillisTime();
            if (!AddInt64(nowTime, timer->intervalMs, timer->nextCallTime)) {
                FI_HILOGE("The addition of nextCallTime in TimerItem overflows");
                return RET_ERR;
            }
            timer->callbackCount = 0;
            InsertTimerInternal(timer);
            return RET_OK;
        }
    }
    return RET_ERR;
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
    int64_t delay = MIN_DELAY;
    if (!timers_.empty()) {
        int64_t nowTime = GetMillisTime();
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
    int64_t nowTime = GetMillisTime();
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
            FI_HILOGE("The addition of nextCallTime in TimerItem overflows");
            return;
        }
        auto callback = curTimer->callback;
        InsertTimerInternal(curTimer);
        callback();
    }
}

int32_t TimerManager::ArmTimer()
{
    CALL_INFO_TRACE;
    if (timerFd_ < 0) {
        FI_HILOGE("TimerManager is uninitialized");
        return RET_ERR;
    }
    struct itimerspec tspec {};
    int64_t expire = CalcNextDelayInternal();
    FI_HILOGI("Next expire %{public}" PRId64, expire);

    if (expire == 0) {
        expire = 1;
    }
    if (expire > 0) {
        tspec.it_value.tv_sec = expire / TIME_CONVERSION;
        tspec.it_value.tv_nsec = (expire % TIME_CONVERSION) * TIME_CONVERSION * TIME_CONVERSION;
    }

    if (timerfd_settime(timerFd_, 0, &tspec, NULL) != 0) {
        FI_HILOGE("Timer: timerfd_settime error");
        return RET_ERR;
    }
    return RET_OK;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
