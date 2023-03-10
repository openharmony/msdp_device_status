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
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerMouseTest" };
constexpr int32_t TIME_WAIT_FOR_OP = 100;
static bool stopCallbackFlag { false };
const std::string CURSOR_DRAG_PATH = "/system/etc/device_status/mouse_icon/File_Drag.png";
} // namespace
class InteractionManagerMouseTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};

void InteractionManagerMouseTest::SetUpTestCase()
{
}

void InteractionManagerMouseTest::SetUp()
{
}

void InteractionManagerMouseTest::TearDown()
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

int32_t SetParam(int32_t width, int32_t height, DragData& dragData, int32_t sourceType, int32_t pointerId)
{
    auto pixelMap = CreatePixelMap(width, height);
    dragData.pictureResourse.pixelMap = pixelMap;
    dragData.pictureResourse.x = -50;
    dragData.pictureResourse.y = -50;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = sourceType;
    dragData.pointerId = pointerId;
    dragData.dragNum = 1;
    dragData.displayX = 0;
    dragData.displayY = 0;
    return RET_OK;
}

/**
 * @tc.name: InteractionManagerMouseTest_Drag
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerMouseTest, InteractionManagerMouseTest_Drag, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DragData dragData;
    int32_t ret = SetParam(90, 90, dragData, MMI::PointerEvent::SOURCE_TYPE_MOUSE, 0);
    ASSERT_EQ(ret, RET_OK);
    stopCallbackFlag = false;
    std::function<void(const DragParam&)> callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, targetPid%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    ret = InteractionManager::GetInstance()->StartDrag(dragData, callback);
    ASSERT_EQ(ret, RET_OK);
    std::cout << "please move the mouse" << std::endl;
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

/**
 * @tc.name: InteractionManagerMouseTest_Drag_002
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerMouseTest, InteractionManagerMouseTest_Drag_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    DragData dragData;
    int32_t ret = SetParam(200, 200, dragData, MMI::PointerEvent::SOURCE_TYPE_MOUSE, 0);
    ASSERT_EQ(ret, RET_OK);
    stopCallbackFlag = false;
    std::function<void(const DragParam&)> callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, targetPid%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    ret = InteractionManager::GetInstance()->StartDrag(dragData, callback);
    ASSERT_EQ(ret, RET_OK);
    std::cout << "please move the mouse" << std::endl;
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
    ret = InteractionManager::GetInstance()->StopDrag(-1);
    std::cout << "stop drag" << std::endl;
    ASSERT_EQ(ret, RET_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
