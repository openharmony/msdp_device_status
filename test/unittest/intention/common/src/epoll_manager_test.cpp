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
    if ((ev.events & EPOLLIN) == EPOLLIN) {
        FI_HILOGI("Epoll input");
    } else if ((ev.events & (EPOLLHUP | EPOLLERR)) != 0) {
        FI_HILOGE("Epoll hangup, errno:%{public}s", strerror(errno));
    }
}

void EpollManagerTest::SetUp() {}

void EpollManagerTest::TearDown() {}

/**
 * @tc.name: EpollManagerTest_Open001
 * @tc.desc: Test Open
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
    MonitorEvent monitor_;
    ret = epollMgr.Add(monitor_);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: EpollManagerTest_Open002
 * @tc.desc: Test Open
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
 * @tc.desc: Test Add
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
    MonitorEvent monitor_;
    ret = epollMgr.Add(monitor_);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    }
    ret = epollMgr.Add(monitor_);
    if (ret == RET_ERR) {
        FI_HILOGI("Do not repeat add the same event, so success");
    } else {
        FI_HILOGE("Add the same event then okay, so failed");
    }
    epollMgr.Close();
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: EpollManagerTest_Remove001
 * @tc.desc: Test Remove
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
    MonitorEvent monitor_;
    ret = epollMgr.Add(monitor_);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    } else {
        epollMgr.Remove(monitor_);
        FI_HILOGI("Remove the event");
    }

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
    MonitorEvent monitor_;
    ret = epollMgr.Add(monitor_);
    if (ret != RET_OK) {
        FI_HILOGE("Add listening event failed");
    } else {
        uint32_t evtType = monitor_.GetEvents();
        ret = epollMgr.Update(monitor_);
        FI_HILOGI("Update the event. the eventtype is %{public}zu", evtType);
    }

    epollMgr.Close();
    EXPECT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS