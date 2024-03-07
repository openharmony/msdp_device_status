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

#define private public
#define protected public

#include <gtest/gtest.h>

#include "channel.h"
#include "fi_log.h"

#undef LOG_TAG
#define LOG_TAG "ChannelTest"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;

class ChannelTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ChannelTest001
 * @tc.desc: test channel throuthput when sending speed is greater than receiving speed.
 * @tc.type: FUNC
 */
HWTEST_F(ChannelTest, ChannelTest001, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto [sender, receiver] = Channel<int32_t>::OpenChannel();
    constexpr int32_t count = 65535;

    std::thread worker([sender = sender, count]() mutable {
        for (int32_t index = 0; index < count; ++index) {
            sender.Send(index);
        }
    });
    for (int32_t expected = 0; expected < count;) {
        int32_t received = receiver.Receive();
        ASSERT_EQ(received, expected);
        if ((++expected % 10) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    if (worker.joinable()) {
        worker.join();
    }
}

/**
 * @tc.name: ChannelTest002
 * @tc.desc: test channel throuthput when sending speed is less than receiving speed.
 * @tc.type: FUNC
 */
HWTEST_F(ChannelTest, ChannelTest002, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto [sender, receiver] = Channel<int32_t>::OpenChannel();
    constexpr int32_t count = 65535;

    std::thread worker([sender = sender, count]() mutable {
        for (int32_t index = 0; index < count;) {
            sender.Send(index);
            if ((++index % 10) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });
    for (int32_t expected = 0; expected < count; ++expected) {
        int32_t received = receiver.Receive();
        ASSERT_EQ(received, expected);
    }
    if (worker.joinable()) {
        worker.join();
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
