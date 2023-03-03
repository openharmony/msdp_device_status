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
constexpr int32_t TIME_WAIT_FOR_INJECT = 10000;
static int32_t mousePointerId { 0 };
static int32_t touchPointerId { 1 };
static std::atomic_bool stopCallbackFlag { false };
static int32_t dragSrcX { 0 };
static int32_t dragSrcY { 0 };
static int32_t dragDstX { 10 };
static int32_t dragDstY { 10 };
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


int32_t CreatePixelMap(int32_t pixelMapWidth, int32_t pixelMapHeight, std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{
    CALL_DEBUG_ENTER;
    if (pixelMapWidth <= 0 || pixelMapWidth > MAX_PIXEL_MAP_WIDTH ||
        pixelMapHeight <= 0 || pixelMapHeight > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("Invalid size, pixelMapWidth:%{public}d, pixelMapHeight:%{public}d", pixelMapWidth, pixelMapHeight);
        return RET_ERR;
    }
    OHOS::Media::ImageInfo info;
    info.size.width = pixelMapWidth;
    info.size.height = pixelMapHeight;
    info.pixelFormat = Media::PixelFormat::RGB_888;
    info.colorSpace = OHOS::Media::ColorSpace::SRGB;
    pixelMap->SetImageInfo(info);
    int32_t bufferSize = pixelMap->GetByteCount();
    char *buffer = static_cast<char *>(malloc(bufferSize));
    if (buffer == nullptr) {
        FI_HILOGE("Malloc buffer failed");
        return RET_ERR;
    }
    char *character = buffer;
    for (int32_t i = 0; i < bufferSize; i++) {
        *(character++) = static_cast<char>(i);
    }
    pixelMap->SetPixelsAddr(buffer, nullptr, bufferSize, OHOS::Media::AllocatorType::HEAP_ALLOC, nullptr);
    return RET_OK;
}

std::optional<DragData> CreateDragData(int32_t width, int32_t height, int32_t sourceType, int32_t pointerId)
{
    CALL_DEBUG_ENTER;
    auto pixelMap = std::make_shared<OHOS::Media::PixelMap>();
    if (CreatePixelMap(width, height, pixelMap) != RET_OK) {
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
    dragData.displayX = dragSrcX;
    dragData.displayY = dragSrcY;
    return dragData;
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

    MMI::PointerEvent::PointerItem item;
    item.SetPointerId(pointerId);
    int32_t displayX = displayLoc.first;
    int32_t displayY = displayLoc.second;
    item.SetDisplayX(displayX);
    item.SetDisplayY(displayY);
    item.SetWindowX(0);
    item.SetWindowY(0);
    item.SetDeviceId(1);
    item.SetWidth(0);
    item.SetHeight(0);
    item.SetPressed(isPressed);
    isPressed ? item.SetPressure(1) : item.SetPressed(0);

    pointerEvent->AddPointerItem(item);
    if (sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        item.SetPointerId(pointerId);
        item.SetDisplayX(displayX + 1);
        item.SetDisplayY(displayY + 1);
        item.SetPressure(1);
        item.SetDeviceId(1);
        isPressed ? item.SetPressure(1) : item.SetPressed(0);
        pointerEvent->AddPointerItem(item);
    }
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
        usleep(TIME_WAIT_FOR_INJECT);
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
    std::optional<DragData> dragData = CreateDragData(MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT,
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, mousePointerId);
    int32_t ret = dragData.has_value() ? RET_OK : RET_ERR;
    ASSERT_EQ(ret, RET_OK);
    stopCallbackFlag = false;
    std::function<void(const DragParam&)> callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, targetPid%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    SimulateDown({ dragSrcX, dragSrcY }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, mousePointerId);
    ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    ASSERT_EQ(ret, RET_OK);
    SimulateMove({ dragSrcX, dragSrcY }, { dragDstX, dragDstY },
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, mousePointerId, true);
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
    int32_t result = 0;
    SimulateUp({ dragDstX, dragDstY }, MMI::PointerEvent::SOURCE_TYPE_MOUSE, mousePointerId);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(result);
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
    std::optional<DragData> dragData = CreateDragData(MAX_PIXEL_MAP_WIDTH, MAX_PIXEL_MAP_HEIGHT,
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, touchPointerId);
    int32_t ret = dragData.has_value() ? RET_OK : RET_ERR;
    ASSERT_EQ(ret, RET_OK);
    stopCallbackFlag = false;
    std::function<void(const DragParam&)> callback = [](const DragParam& dragParam) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d, targetPid%{public}d",
            dragParam.displayX, dragParam.displayY, dragParam.result, dragParam.targetPid);
        stopCallbackFlag = true;
    };
    SimulateDown({ dragSrcX, dragSrcY }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, touchPointerId);
    ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), callback);
    SimulateMove({ dragSrcX, dragSrcY }, { dragDstX, dragDstY },
        MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, touchPointerId, true);
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
    int32_t result = 0;
    SimulateUp({ dragDstX, dragDstY }, MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN, touchPointerId);
    int32_t ret = InteractionManager::GetInstance()->StopDrag(result);
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
    FI_HILOGD("targetPid:%{public}d", targetPid);
    ASSERT_TRUE(targetPid > -1);
}

} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
