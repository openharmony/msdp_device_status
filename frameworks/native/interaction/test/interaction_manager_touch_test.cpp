/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <iostream>

#include <gtest/gtest.h>
#include "pointer_event.h"

#include "image_source.h"

#include "coordination_message.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerTouchTest" };
constexpr int32_t TIME_WAIT_FOR_OP = 100;
const std::string CURSOR_DRAG_PATH = "/system/etc/device_status/mouse_icon/File_Drag.png";
} // namespace
class InteractionManagerTouchTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};

void InteractionManagerTouchTest::SetUpTestCase()
{
}

void InteractionManagerTouchTest::SetUp()
{
}

void InteractionManagerTouchTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));
}

std::shared_ptr<OHOS::Media::PixelMap> CreatePixelMap(int32_t pixelMapWidth, int32_t pixelMapHeight)
{
    std::string imagePath = CURSOR_DRAG_PATH;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/png";
    uint32_t errCode = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(imagePath, opts, errCode);
    if (imageSource == nullptr) {
        return nullptr;
    }
    std::set<std::string> formats;
    imageSource->GetSupportedFormats(formats);
    OHOS::Media::DecodeOptions decodeOpts;
    decodeOpts.desiredSize = {
        .width = pixelMapWidth,
        .height = pixelMapHeight
    };
    decodeOpts.allocatorType = OHOS::Media::AllocatorType::SHARE_MEM_ALLOC;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    return pixelMap;
}

int32_t SetParam(int32_t width, int32_t height, DragData& dragData)
{
    auto pixelMap = CreatePixelMap(width, height);
    dragData.pictureResourse.pixelMap = pixelMap;
    dragData.pictureResourse.x = -50;
    dragData.pictureResourse.y = -50;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    dragData.pointerId = 0;
    dragData.dragNum = 1;
    return RET_OK;
}

/**
 * @tc.name: InteractionManagerTouchTest_Drag
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTouchTest, InteractionManagerTouchTest_Drag, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DragData dragData;
    int32_t ret = SetParam(90, 90, dragData);
    ASSERT_EQ(ret, RET_OK);
    std::function<void(int32_t)> callback = [](int32_t result) {
        FI_HILOGD("StartDrag success");
    };
    ret = InteractionManager::GetInstance()->StartDrag(dragData, callback);
    ASSERT_EQ(ret, RET_OK);
    std::cout << "please touch the screen" << std::endl;
    std::cout << "start Drag" << std::endl;
    int32_t dragStyle = 0;
    ret = InteractionManager::GetInstance()->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    std::cout << "update drag style 0" << std::endl;
    sleep(10);
    dragStyle = 22;
    ret = InteractionManager::GetInstance()->UpdateDragStyle(dragStyle);
    std::cout << "change drag style" << std::endl;
    sleep(10);
    ret = InteractionManager::GetInstance()->StopDrag(0);
    std::cout << "stop drag" << std::endl;
    ASSERT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
