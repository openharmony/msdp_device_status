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

#include <atomic>
#include <optional>
#include <utility>
#include <vector>

#include <gtest/gtest.h>
#include "image_source.h"
#include "input_manager.h"
#include "pointer_event.h"

#include "coordination_message.h"
#include "devicestatus_define.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "InteractionManagerTest" };
constexpr int32_t TIME_WAIT_FOR_OP = 100;
constexpr int32_t TIME_WAIT_FOR_INJECT_MS = 10000;
constexpr int32_t TIME_WAIT_FOR_PROCESS_CALLBACK = 500000;
constexpr int32_t DEFAULT_DEVICE_ID = 0;
constexpr int32_t MOUSE_POINTER_ID { 0 };
constexpr int32_t TOUCH_POINTER_ID { 1 };
constexpr int32_t DRAG_SRC_X { 0 };
constexpr int32_t DRAG_SRC_Y { 0 };
constexpr int32_t DRAG_DST_X { 10 };
constexpr int32_t DRAG_DST_Y { 10 };
static std::atomic_bool stopCallbackFlag { false };
static const std::string IMAGE_INPUT_JPG_PATH_600 = "/data/local/tmp/image/test600.jpg";
static const std::string IMAGE_INPUT_PNG_PATH_600 = "/data/local/tmp/image/test600.png";
static const std::string IMAGE_INPUT_JPG_PATH_200 = "/data/local/tmp/image/test200.jpg";
static const std::string IMAGE_INPUT_PNG_PATH_200 = "/data/local/tmp/image/test200.png";
#define INPUT_MANAGER  MMI::InputManager::GetInstance()
} // namespace
class InteractionManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
};

void InteractionManagerTest::SetUpTestCase()
{
}

void InteractionManagerTest::SetUp()
{
}

void InteractionManagerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP));
}

std::shared_ptr<Media::PixelMap> CreatePixelMap(const std::string& path)
{
    CALL_DEBUG_ENTER;
    Media::SourceOptions opts;
    int32_t pos = path.find_last_of(".");
    if (pos < 0) {
        FI_HILOGE("Invalid image path:%{public}s", path.c_str());
        return nullptr;
    }
    auto suffix = path.substr(pos);
    if (suffix == std::string(".png")) {
        opts.formatHint = "image/png";  
    } else if (suffix == std::string(".jpg")) {
        opts.formatHint = "image/jpg";
    } else if (suffix == std::string(".jpeg")) {
        opts.formatHint = "image/jpeg";
    } else {
        FI_HILOGE("Invalid image path:%{public}s", path.c_str());
        return nullptr;
    }
    uint32_t errorCode = 0;
    std::unique_ptr<Media::ImageSource> imageSource =
        Media::ImageSource::CreateImageSource(path, opts, errorCode);
    Media::ImageInfo imageInfo;
    imageSource->GetImageInfo(0, imageInfo);
    FI_HILOGD("imageInfo.size.width:%{public}d, imageInfo.size.height:%{public}d",
        imageInfo.size.width, imageInfo.size.width);
    if (errorCode != 0 || imageSource.get() == nullptr) {
        FI_HILOGE("CreateImageSource failed");
        return nullptr;
    }
    Media::DecodeOptions decodeOpts;
    decodeOpts.allocatorType = Media::AllocatorType::SHARE_MEM_ALLOC;
    std::unique_ptr<Media::PixelMap> uniquePixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    std::shared_ptr<Media::PixelMap> pixelMap = std::move(uniquePixelMap);
    return pixelMap;
}

std::shared_ptr<Media::PixelMap> CreatePixelMap(int32_t pixelMapWidth, int32_t pixelMapHeight)
{
    CALL_DEBUG_ENTER;
    if (pixelMapWidth <= 0 || pixelMapWidth > MAX_PIXEL_MAP_WIDTH ||
        pixelMapHeight <= 0 || pixelMapHeight > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size, pixelMapWidth:%{public}d, pixelMapHeight:%{public}d", pixelMapWidth, pixelMapHeight);
        return nullptr;
    }
    OHOS::Media::InitializationOptions opts;
    opts.size.width = pixelMapWidth;
    opts.size.height = pixelMapHeight;
    std::unique_ptr<Media::PixelMap> uniquePixelMap = Media::PixelMap::Create(opts);
    std::shared_ptr<Media::PixelMap> pixelMap = std::move(uniquePixelMap);
    return pixelMap;
}

std::optional<DragData> CreateDragData(int32_t width, int32_t height, int32_t sourceType,
    int32_t pointerId, std::pair<int32_t, int32_t> loc)
{
    CALL_DEBUG_ENTER;
    // std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(width, height);
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(IMAGE_INPUT_PNG_PATH_600);
    if (pixelMap == nullptr) {
        FI_HILOGE("CreatePixelMap failed");
        return std::nullopt;
    }
    DragData dragData;
    dragData.pictureResourse.pixelMap = pixelMap;
    dragData.pictureResourse.x = 0;
    dragData.pictureResourse.y = 0;
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.sourceType = sourceType;
    dragData.pointerId = pointerId;
    dragData.dragNum = 1;
    dragData.displayX = loc.first;
    dragData.displayY = loc.second;
    return dragData;
}

MMI::PointerEvent::PointerItem CreatePointerItem(int32_t pointerId,
    int32_t deviceId, std::pair<int, int> displayLoc, bool isPressed)
{
    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    item.SetDeviceId(deviceId);
    int32_t displayX = displayLoc.first;
    int32_t displayY = displayLoc.second;
    item.SetDisplayX(displayX);
    item.SetDisplayY(displayY);
    item.SetWindowX(0);
    item.SetWindowY(0);
    item.SetWidth(0);
    item.SetHeight(0);
    item.SetPressed(isPressed);
    isPressed ? item.SetPressure(1) : item.SetPressed(0);
    return item;
}

std::shared_ptr<MMI::PointerEvent> SetupPointerEvent(
    std::pair<int, int> displayLoc, int32_t action, int32_t sourceType, int32_t pointerId, bool isPressed)
{
    CALL_DEBUG_ENTER;
    auto pointerEvent = MMI::PointerEvent::Create();
    CHKPP(pointerEvent);
    pointerEvent->SetPointerAction(action);
    pointerEvent->SetSourceType(sourceType);
    pointerEvent->SetPointerId(pointerId);

    auto curPointerItem = CreatePointerItem(pointerId, DEFAULT_DEVICE_ID, displayLoc, isPressed);
    pointerEvent->AddPointerItem(curPointerItem);
    return pointerEvent;
}

void SimulateDown(std::pair<int, int> loc, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<MMI::PointerEvent> pointerEvent =
        SetupPointerEvent(loc, MMI::PointerEvent::POINTER_ACTION_DOWN, sourceType, pointerId, true);
    FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
    INPUT_MANAGER->SimulateInputEvent(pointerEvent);
}

void SimulateMove(std::pair<int, int> srcLoc, std::pair<int, int> dstLoc,
    int32_t sourceType, int32_t pointerId, bool isPressed)
{
    CALL_DEBUG_ENTER;
    int32_t srcX = srcLoc.first;
    int32_t srcY = srcLoc.second;
    int32_t dstX = dstLoc.first;
    int32_t dstY = dstLoc.second;
    std::vector<std::pair<int32_t, int32_t>> pointers;
    if (dstX == srcX) {
        for (int32_t y = srcY; y <= dstY; y++) {
            pointers.push_back({srcX, y});
        }
    } else if (dstY == srcY) {
        for (int32_t x = srcX; x <= dstX; x++) {
            pointers.push_back({x, srcY});
        }
    } else {
        double ratio = (dstY - srcY) * 1.0 / (dstX - srcX);
        for (int32_t x = srcX; x < dstX; x++) {
            pointers.push_back({x, srcY + static_cast<int32_t>(ratio * (x - srcX))});
        }
        pointers.push_back({dstX, dstY});
    }
    for (const auto& pointer : pointers) {
        std::shared_ptr<MMI::PointerEvent> pointerEvent =
            SetupPointerEvent(pointer, MMI::PointerEvent::POINTER_ACTION_MOVE, sourceType, pointerId, isPressed);
        FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
            pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
        INPUT_MANAGER->SimulateInputEvent(pointerEvent);
        usleep(TIME_WAIT_FOR_INJECT_MS);
    }
}

void SimulateUp(std::pair<int, int> loc, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<MMI::PointerEvent> pointerEvent =
        SetupPointerEvent(loc, MMI::PointerEvent::POINTER_ACTION_UP, sourceType, pointerId, false);
    FI_HILOGD("TEST:sourceType:%{public}d, pointerId:%{public}d, pointerAction:%{public}d",
        pointerEvent->GetSourceType(), pointerEvent->GetPointerId(), pointerEvent->GetPointerAction());
    INPUT_MANAGER->SimulateInputEvent(pointerEvent);
}

/**
 * @tc.name: InteractionManagerTest_RegisterCoordinationListener_001
 * @tc.desc: Register coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_RegisterCoordinationListener_001, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<ICoordinationListener> consumer = nullptr;
    int32_t ret = InteractionManager::GetInstance()->RegisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_ERR);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_RegisterCoordinationListener_002
 * @tc.desc: Register coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_RegisterCoordinationListener_002, TestSize.Level1)
{
    CALL_DEBUG_ENTER;
    class CoordinationListenerTest : public ICoordinationListener {
    public:
        CoordinationListenerTest() : ICoordinationListener() {}
        void OnCoordinationMessage(const std::string &deviceId, CoordinationMessage msg) override
        {
            FI_HILOGD("RegisterCoordinationListenerTest");
        };
    };
    std::shared_ptr<CoordinationListenerTest> consumer =
        std::make_shared<CoordinationListenerTest>();
    int32_t ret = InteractionManager::GetInstance()->RegisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
    ret = InteractionManager::GetInstance()->UnregisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_UnregisterCoordinationListener
 * @tc.desc: Unregister coordination listener
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_UnregisterCoordinationListener, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<ICoordinationListener> consumer = nullptr;
    int32_t ret = InteractionManager::GetInstance()->UnregisterCoordinationListener(consumer);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_EnableCoordination
 * @tc.desc: Enable coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_EnableCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    bool enabled = false;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Enable coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->EnableCoordination(enabled, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StartCoordination
 * @tc.desc: Start coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    std::string sinkDeviceId("");
    int32_t srcDeviceId = -1;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Start coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->StartCoordination(sinkDeviceId, srcDeviceId, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_NE(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StopCoordination
 * @tc.desc: Stop coordination
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopCoordination, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    auto fun = [](std::string listener, CoordinationMessage coordinationMessages) {
        FI_HILOGD("Stop coordination success");
    };
    int32_t ret = InteractionManager::GetInstance()->StopCoordination(fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_NE(ret, ERROR_UNSUPPORT);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_GetCoordinationState
 * @tc.desc: Get coordination state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_GetCoordinationState, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    const std::string deviceId("");
    auto fun = [](bool state) {
        FI_HILOGD("Get coordination state success");
    };
    int32_t ret = InteractionManager::GetInstance()->GetCoordinationState(deviceId, fun);
#ifdef OHOS_BUILD_ENABLE_COORDINATION
    ASSERT_EQ(ret, RET_OK);
#else
    ASSERT_EQ(ret, ERROR_UNSUPPORT);
#endif // OHOS_BUILD_ENABLE_COORDINATION
}

/**
 * @tc.name: InteractionManagerTest_StartDrag_Mouse
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartDrag_Mouse, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    stopCallbackFlag = false;
    auto callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    SimulateDown({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    std::optional<DragData> dragData = CreateDragData(MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT,
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    int32_t ret = dragData.has_value() ? RET_OK : RET_ERR;
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    SimulateMove({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_DST_X, DRAG_DST_Y },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID, true);
}

/**
 * @tc.name: InteractionManagerTest_StopDrag_Mouse
 * @tc.desc: Stop drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopDrag_Mouse, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SimulateUp({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, MOUSE_POINTER_ID);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(static_cast<int32_t>(DragResult::DRAG_SUCCESS));
    usleep(TIME_WAIT_FOR_PROCESS_CALLBACK);
    ASSERT_TRUE(stopCallbackFlag);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: InteractionManagerTest_StartDrag_Touch
 * @tc.desc: Start Drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StartDrag_Touch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    stopCallbackFlag = false;
    auto callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, target:%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    SimulateDown({ DRAG_SRC_X, DRAG_SRC_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    std::optional<DragData> dragData = CreateDragData(MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, { DRAG_SRC_X, DRAG_SRC_Y });
    int32_t ret = dragData.has_value() ? RET_OK : RET_ERR;
    ASSERT_EQ(ret, RET_OK);
    ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    SimulateMove({ DRAG_SRC_X, DRAG_SRC_Y }, { DRAG_DST_X, DRAG_DST_Y },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID, true);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: InteractionManagerTest_StopDrag_Touch
 * @tc.desc: Stop drag
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_StopDrag_Touch, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    SimulateUp({ DRAG_DST_X, DRAG_DST_Y }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, TOUCH_POINTER_ID);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(static_cast<int32_t>(DragResult::DRAG_SUCCESS));
    usleep(TIME_WAIT_FOR_PROCESS_CALLBACK);
    ASSERT_TRUE(stopCallbackFlag);
    ASSERT_EQ(ret, RET_OK);
}

 /**
*  @tc.name: InteractionManagerTest_GetDragTargetPid
 * @tc.desc: Get Drag Target Pid
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(InteractionManagerTest, InteractionManagerTest_GetDragTargetPid, TestSize.Level1)
{
    CALL_TEST_DEBUG;
    int32_t targetPid = InteractionManager::GetInstance()->GetDragTargetPid();
    FI_HILOGD("target:%{public}d", targetPid);
    ASSERT_TRUE(targetPid >= -1);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
