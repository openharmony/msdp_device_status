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
#include "drag_drawing_test.h"

#include <gtest/gtest.h>

#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "pointer_event.h"

#include "devicestatus_common.h"
#include "devicestatus_define.h"
#include "drag_data.h"

using namespace testing::ext;
using namespace OHOS::Msdp::DeviceStatus;
using namespace OHOS;
using namespace std;

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawingTest" };
constexpr int32_t TIME_WAIT_FOR_OP = 100;
const std::string CURSOR_DRAG_PATH = "/system/etc/device_status/mouse_icon/File_Drag.png";
} // namespace

class DragDrawingTest : public testing::Test {
public:
    void SetUp() {};
    void TearDown() {};
    static void SetUpTestCase();
};

void DragDrawingTest::SetUpTestCase()
{
}

void DragDrawingTest::SetUp()
{
}

void DragDrawingTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));
}

std::shared_ptr<OHOS::Media::PixelMap> CreatePixelMap()
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
        .width = 90,
        .height = 90
    };
    decodeOpts.allocatorType = OHOS::Media::AllocatorType::SHARE_MEM_ALLOC;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    return pixelMap;
}

/**
 * @tc.name: DragDrawingTest_InitPicture_001
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_InitPicture_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    ASSERT_EQ(ret, RET_OK);
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_InitPicture_002
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_InitPicture_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    ASSERT_EQ(ret, RET_OK);
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_InitPicture_003
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_InitPicture_003, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_UNKNOWN);
    ASSERT_EQ(ret, RET_ERR);
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_UpdateDragStyle_001
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_UpdateDragStyle_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    ASSERT_EQ(ret, RET_OK);
    int32_t dragStyle = 100;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_UpdateDragStyle_002
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_UpdateDragStyle_002, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    ASSERT_EQ(ret, RET_OK);
    int32_t dragStyle = -1;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_ERR);
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_DrawTouchPicture_001
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_DrawTouchPicture_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
    ASSERT_EQ(ret, RET_OK);
    int32_t dragStyle = 1;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    int32_t x = 100;
    int32_t y = 100;
    dragDrawing->Draw(0, x, y);
    sleep(2);
    x += 200;
    y += 200;
    dragStyle = 0;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    dragDrawing->Draw(0, x, y);
    sleep(2);
    dragStyle = 2222;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    sleep(2);
    x += 200;
    y += 200;
    dragDrawing->Draw(0, x, y);
    sleep(2);
    dragDrawing->OnDragSuccess();
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}

/**
 * @tc.name: DragDrawingTest_DrawMousePicture_001
 * @tc.desc: 
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragDrawingTest, DragDrawingTest_DrawMousePicture_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto dragDrawing = new (std::nothrow) DragDrawing;
    ASSERT_NE(dragDrawing, nullptr);
    shared_ptr<OHOS::Media::PixelMap> pixelMap = CreatePixelMap();
    ASSERT_NE(pixelMap, nullptr);
    PictureResourse pictureResourse;
    pictureResourse.pixelMap = pixelMap;
    pictureResourse.x = -10;
    pictureResourse.y = -10;
    int32_t ret = dragDrawing->InitPicture(pictureResourse, OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE);
    ASSERT_EQ(ret, RET_OK);
    int32_t dragStyle = 1;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    int32_t x = 0;
    int32_t y = 0;
    dragDrawing->Draw(0, x, y);
    sleep(2);
    x += 100;
    y += 100;
    dragStyle = 0;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    sleep(2);
    x += 100;
    y += 100;
    dragDrawing->Draw(0, x, y);
    sleep(2);
    x += 100;
    y += 100;
    dragStyle = 100000;
    ret = dragDrawing->UpdateDragStyle(dragStyle);
    ASSERT_EQ(ret, RET_OK);
    dragDrawing->Draw(0, x, y);
    sleep(2);
    dragDrawing->OnDragFail();
    dragDrawing->DestroyPointerWindow();
    delete dragDrawing;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS