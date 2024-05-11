/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#define private public
#include "epoll_manager_test.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include "devicestatus_common.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "EpollManagerTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t MAX_N_EVENTS { 64 };
constexpr int32_t TIMEOUT { 30 };
constexpr int32_t BLOCK_EPOLL { -1 };
constexpr int32_t UNBLOCK_EPOLL { 0 };
constexpr int32_t EVENTS_COUNT { 10 };
constexpr int32_t TIME_WAIT_FOR_OP_MS { 1001 };
constexpr int32_t DISPATCH_TIMES { 5 };
constexpr int32_t EXPIRE_TIME { 2 };
} // namespace

void EpollManagerTest::SetUpTestCase() {}

void EpollManagerTest::TearDownTestCase() {}

MonitorEvent::MonitorEvent()
{
    inotifyFd_ = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (inotifyFd_ < 0) {
        FI_HILOGE("timerfd_create failed");
    }
}

MonitorEvent::~MonitorEvent()
{
    if (inotifyFd_ < 0) {
        FI_HILOGE("Invalid timerFd_");
        return;
    }
    if (close(inotifyFd_) < 0) {
        FI_HILOGE("Close timer fd failed, error:%{public}s, timerFd_:%{public}d", strerror(errno), inotifyFd_);
    }
    inotifyFd_ = -1;
}

void MonitorEvent::Dispatch(const struct epoll_event &ev)
{
    CALL_DEBUG_ENTER;
    if (inotifyFd_ < 0) {
        FI_HILOGE("inotifyFd_ is invalid.");
        return;
    }
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        int32_t buf[EXPIRE_TIME] = { 0 };
        int32_t ret = read(inotifyFd_, buf, sizeof(buf));
        if (ret < 0) {
            FI_HILOGE("epoll callback read error");
        } else {
            FI_HILOGI("Epoll input, buf[0]: %{public}d, buf[1]: %{public}d", buf[0], buf[1]);
        }
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup, errno:%{public}s", strerror(errno));
    }
}

int32_t MonitorEvent::SetTimer()
{
    if (inotifyFd_ < 0) {
        FI_HILOGE("TimerManager is not initialized");
        return RET_ERR;
    }
    struct itimerspec tspec {};
    tspec.it_interval = {1, 0}; // period timeout value = 1s
    tspec.it_value = {1, 0};  // initial timeout value = 1s0ns
    if (timerfd_settime(inotifyFd_, 0, &tspec, NULL) != 0) {
        FI_HILOGE("Timer: the inotifyFd_ is error");
        return RET_ERR;
    }
    return RET_OK;
}

void EpollManagerTest::SetUp() {}

void EpollManagerTest::TearDown() {}

/**
 * @tc.name: EpollManagerTest_Open001
 * @tc.desc: Test Open, open and close once
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Open001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Open002
 * @tc.desc: Test Open, open and close twice
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Open002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);

    ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Close001
 * @tc.desc: Test Close
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Close001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    epollMgr.Close();
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_GetFd001
 * @tc.desc: Test GetFd
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_GetFd001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.GetFd();
    if (ret < 0) {
        FI_HILOGI("Get the fd which does not exist, then error, so success");
    } else {
        FI_HILOGE("Get the fd which does not exist, but okay, so failed");
    }
    EXPECT_NE(ret, RET_OK);

    ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    ret = epollMgr.GetFd();
    if (ret < 0) {
        FI_HILOGE("Get fd failed");
    } else {
        FI_HILOGI("Get the fd %{public}d success", ret);
    }
    epollMgr.Close();
    EXPECT_GE(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Add001
 * @tc.desc: Test Add, Add duplicate events
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Add001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    ret = epollMgr.Add(monitor);
    if (ret == RET_ERR) {
        FI_HILOGI("Do not repeat add the same event, so success");
    } else {
        FI_HILOGE("Add the same event but okay, so failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: EpollManagerTest_Add002
 * @tc.desc: Test Add, Add the event with fd of -1
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Add002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    int32_t fd = monitor.GetFd();
    monitor.inotifyFd_ = -1;
    ret = epollMgr.Add(monitor);
    monitor.inotifyFd_ = fd;
    if (ret == RET_ERR) {
        FI_HILOGI("Add the event with fd of -1, then error, so success");
    } else {
        FI_HILOGE("Add the event with fd of -1, but okay, so failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: EpollManagerTest_Remove001
 * @tc.desc: Test Remove, remove existing events
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Remove001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    } else {
        epollMgr.Remove(monitor);
        FI_HILOGI("Remove the event");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Remove002
 * @tc.desc: Test Remove, remove events that have not been added
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Remove002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    epollMgr.Remove(monitor);
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Remove003
 * @tc.desc: Test Remove, remove the event with fd of -1
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Remove003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    int32_t fd = monitor.GetFd();
    monitor.inotifyFd_ = -1;
    epollMgr.Remove(monitor);
    monitor.inotifyFd_ = fd;
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Update001
 * @tc.desc: Test Update
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Update001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    } else {
        uint32_t evtType = monitor.GetEvents();
        ret = epollMgr.Update(monitor);
        FI_HILOGI("Update the event. the eventtype is 0x%{public}x", evtType);
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Update002
 * @tc.desc: Test Update, modify a non-existent event
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Update002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Update(monitor);
    if (ret == RET_ERR) {
        FI_HILOGI("Modifying a non-existent event will result in an error, so success");
    } else {
        FI_HILOGE("Modify a non-existent event but okay, so failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout001
 * @tc.desc: Test WaitTimeout
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epollMgr.WaitTimeout(evs, MAX_N_EVENTS, TIMEOUT);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout002
 * @tc.desc: Test WaitTimeout, no event to wait
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epollMgr.WaitTimeout(evs, MAX_N_EVENTS, TIMEOUT);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout003
 * @tc.desc: Test WaitTimeout, use of block mode
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = monitor.SetTimer();
    EXPECT_EQ(ret, RET_OK);
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    EXPECT_EQ(ret, RET_OK);
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epollMgr.WaitTimeout(evs, MAX_N_EVENTS, BLOCK_EPOLL);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout004
 * @tc.desc: Test WaitTimeout, use of non blocking mode
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout004, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epollMgr.WaitTimeout(evs, MAX_N_EVENTS, UNBLOCK_EPOLL);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout005
 * @tc.desc: Test WaitTimeout, Abnormal scenarios with small memory and multiple events
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout005, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[EVENTS_COUNT];
    int32_t cnt = epollMgr.WaitTimeout(evs, MAX_N_EVENTS, TIMEOUT);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_WaitTimeout006
 * @tc.desc: Test WaitTimeout, Abnormal scenarios with big memory and zero events
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_WaitTimeout006, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[EVENTS_COUNT];
    int32_t cnt = epollMgr.WaitTimeout(evs, 0, TIMEOUT);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_LT(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Wait001
 * @tc.desc: Test Wait
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Wait001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = monitor.SetTimer();
    EXPECT_EQ(ret, RET_OK);
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[MAX_N_EVENTS];
    int32_t cnt = epollMgr.Wait(evs, MAX_N_EVENTS);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Wait002
 * @tc.desc: Test Wait, Abnormal scenarios with small memory and multiple events
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Wait002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = monitor.SetTimer();
    EXPECT_EQ(ret, RET_OK);
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event evs[EVENTS_COUNT];
    int32_t cnt = epollMgr.Wait(evs, MAX_N_EVENTS);
    FI_HILOGI("No event input, timeout will end, cnt is %{public}d", cnt);
    epollMgr.Close();
    EXPECT_GE(cnt, 0);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Dispatch001
 * @tc.desc: Test Dispatch, dispatch when there is event input
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Dispatch001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = &epollMgr;
    epollMgr.Dispatch(ev);
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Dispatch002
 * @tc.desc: Test Dispatch, dispatch when there are errors
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Dispatch002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event ev {};
    ev.events = EPOLLERR;
    ev.data.ptr = &epollMgr;
    epollMgr.Dispatch(ev);
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Dispatch003
 * @tc.desc: Test Dispatch, dispatch when there is event input
 * @tc.type: FUNC
 */
HWTEST_F(EpollManagerTest, EpollManagerTest_Dispatch003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    EpollManager epollMgr;
    int32_t ret = epollMgr.Open();
    EXPECT_EQ(ret, RET_OK);
    if (ret != RET_OK) {
        FI_HILOGE("Open epoll-manager failed");
        return;
    }
    MonitorEvent monitor;
    ret = monitor.SetTimer();
    EXPECT_EQ(ret, RET_OK);
    ret = epollMgr.Add(monitor);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    struct epoll_event ev {};
    ev.events = EPOLLIN;
    ev.data.ptr = &monitor;
    ev.data.fd = monitor.GetFd();
    FI_HILOGI("The timerfd is %{public}d", monitor.GetFd());
    for (int32_t i = 0; i < DISPATCH_TIMES; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
        FI_HILOGI("Dispatch times: %{public}d", i);
        epollMgr.Dispatch(ev);
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS