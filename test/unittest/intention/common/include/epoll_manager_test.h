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

#ifndef EPOLL_MANAGER_TEST_H
#define EPOLL_MANAGER_TEST_H
#define private public

#include <fcntl.h>

#include <gtest/gtest.h>

#include "devicestatus_define.h"
#include "epoll_manager.h"
#include "i_epoll_event_source.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class MonitorEvent final : public IEpollEventSource {
public:
    MonitorEvent();
    DISALLOW_COPY_AND_MOVE(MonitorEvent);
    ~MonitorEvent();

    int32_t GetFd() const override;
    void Dispatch(const struct epoll_event &ev) override;
    int32_t SetTimer();
public:
    int32_t inotifyFd_ { -1 };
};

inline int32_t MonitorEvent::GetFd() const
{
    return inotifyFd_;
}

class EpollManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // EPOLL_MANAGER_TEST_H