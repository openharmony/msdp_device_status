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
#define BUFF_SIZE 100
#include <future>
#include "pointer_event.h"
#include "securec.h"

#include "drag_manager_test.h"
#include "devicestatus_errors.h"
#include "interaction_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
constexpr uint32_t DEFAULT_ICON_COLOR { 0xFF };
constexpr int32_t PIXEL_MAP_HEIGHT { 3 };
constexpr int32_t PIXEL_MAP_WIDTH { 3 };
constexpr int32_t MAX_PIXEL_MAP_WIDTH { 600 };
constexpr int32_t MAX_PIXEL_MAP_HEIGHT { 600 };
constexpr int32_t POINTER_ID { 0 };
constexpr int32_t DISPLAY_X { 50 };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DISPLAY_Y { 50 };
constexpr int32_t DRAG_NUM_ONE { 1 };
constexpr int32_t INT32_BYTE { 4 };
int32_t SHADOWINFO_X { 0 };
int32_t SHADOWINFO_Y { 0 };
constexpr bool HAS_CANCELED_ANIMATION { true };
const std::string UD_KEY { "Unified data key" };
const std::string FILTER_INFO { "Undefined filter info" };
const std::string EXTRA_INFO { "Undefined extra info" };
constexpr int32_t SHADOW_NUM_ONE { 1 };
} // namespace

void DragManagerTest::SetUpTestCase() {}

void DragManagerTest::SetUp() {
}

void DragManagerTest::TearDown()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

std::shared_ptr<Media::PixelMap> DragManagerTest::CreatePixelMap(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (width <= 0 || width > MAX_PIXEL_MAP_WIDTH || height <= 0 || height > MAX_PIXEL_MAP_HEIGHT) {
        FI_HILOGE("invalid, height:%{public}d, width:%{public}d", height, width);
        return nullptr;
    }
    Media::InitializationOptions opts;
    opts.size.width = width;
    opts.size.height = height;
    opts.pixelFormat = Media::PixelFormat::BGRA_8888;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    opts.scaleMode = Media::ScaleMode::FIT_TARGET_SIZE;

    int32_t colorLen = width * height;
    uint32_t *pixelColors = new (std::nothrow) uint32_t[BUFF_SIZE];
    CHKPP(pixelColors);
    int32_t colorByteCount = colorLen * INT32_BYTE;
    errno_t ret = memset_s(pixelColors, BUFF_SIZE, DEFAULT_ICON_COLOR, colorByteCount);
    if (ret != EOK) {
        FI_HILOGE("memset_s failed");
        delete[] pixelColors;
        return nullptr;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = Media::PixelMap::Create(pixelColors, colorLen, opts);
    if (pixelMap == nullptr) {
        FI_HILOGE("Create pixelMap failed");
        delete[] pixelColors;
        return nullptr;
    }
    delete[] pixelColors;
    return pixelMap;
}

std::optional<DragData> DragManagerTest::CreateDragData(int32_t sourceType,
    int32_t pointerId, int32_t dragNum, bool hasCoordinateCorrected, int32_t shadowNum)
{
    CALL_DEBUG_ENTER;
    DragData dragData;
    for (int32_t i = 0; i < shadowNum; i++) {
        std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(PIXEL_MAP_WIDTH, PIXEL_MAP_HEIGHT);
        if (pixelMap == nullptr) {
            FI_HILOGE("pixelMap nullptr");
            return std::nullopt;
        }
        dragData.shadowInfos.push_back({ pixelMap, SHADOWINFO_X, SHADOWINFO_Y });
    }
    dragData.buffer = std::vector<uint8_t>(MAX_BUFFER_SIZE, 0);
    dragData.extraInfo = FILTER_INFO;
    dragData.udKey = UD_KEY;
    dragData.sourceType = sourceType;
    dragData.extraInfo = EXTRA_INFO;
    dragData.displayId = DISPLAY_ID;
    dragData.pointerId = pointerId;
    dragData.dragNum = dragNum;
    dragData.displayX = DISPLAY_X;
    dragData.displayY = DISPLAY_Y;
    dragData.hasCoordinateCorrected = hasCoordinateCorrected;
    dragData.hasCanceledAnimation = HAS_CANCELED_ANIMATION;
    return dragData;
}

class TestStartDragListener : public IStartDragListener {
public:
    explicit TestStartDragListener(std::function<void(const DragNotifyMsg&)> function) : function_(function) { }
    void OnDragEndMessage(const DragNotifyMsg &msg) override
    {
        FI_HILOGD("DisplayX:%{public}d, displayY:%{public}d, targetPid:%{public}d, result:%{public}d",
            msg.displayX, msg.displayY, msg.targetPid, static_cast<int32_t>(msg.result));
        if (function_ != nullptr) {
            function_(msg);
        }
        FI_HILOGD("Test OnDragEndMessage");
    }

    void OnHideIconMessage() override
    {
        FI_HILOGD("Test OnHideIconMessage");
    }
private:
    std::function<void(const DragNotifyMsg&)> function_;
};

class DragListenerTest : public IDragListener {
public:
    DragListenerTest() {}
    explicit DragListenerTest(const std::string& name) : moduleName_(name) {}
    void OnDragMessage(DragState state) override
    {
        if (moduleName_.empty()) {
            moduleName_ = std::string("DragListenerTest");
        }
        FI_HILOGD("%{public}s, state:%{public}s", moduleName_.c_str(), PrintDragMessage(state).c_str());
    }
private:
    std::string PrintDragMessage(DragState state)
    {
        std::string type = "unknow";
        const std::map<DragState, std::string> stateType = {
            { DragState::ERROR, "error"},
            { DragState::START, "start"},
            { DragState::STOP, "stop"},
            { DragState::CANCEL, "cancel"}
        };
        auto item = stateType.find(state);
        if (item != stateType.end()) {
            type = item->second;
        }
        return type;
    }
private:
    std::string moduleName_;
};

class SubscriptListenerTest : public ISubscriptListener {
public:
    SubscriptListenerTest() {}
    explicit SubscriptListenerTest(const std::string& name) : moduleName_(name) {}
    void OnMessage(DragCursorStyle style) override
    {
        SetDragSyle(style);
        if (moduleName_.empty()) {
            moduleName_ = std::string("SubscriptListenerTest");
        }
        FI_HILOGD("subscriptListener, %{public}s, state:%{public}s",
            moduleName_.c_str(), PrintStyleMessage(style).c_str());
    }

    DragCursorStyle GetDragStyle()
    {
        return dragStyle_;
    }

private:
    void SetDragSyle(DragCursorStyle style)
    {
        dragStyle_ = style;
    }

    std::string PrintStyleMessage(DragCursorStyle style)
    {
        std::string type = "unknow";
        const std::map<DragCursorStyle, std::string> cursorStyles = {
            { DragCursorStyle::DEFAULT, "default"},
            { DragCursorStyle::FORBIDDEN, "forbidden"},
            { DragCursorStyle::COPY, "copy"},
            { DragCursorStyle::MOVE, "move"}
        };
        auto item = cursorStyles.find(style);
        if (item != cursorStyles.end()) {
            type = item->second;
        }
        return type;
    }

private:
    DragCursorStyle dragStyle_ { DragCursorStyle::DEFAULT };
    std::string moduleName_;
};

/**
 * @tc.name: DragManagerTest1
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(), nullptr);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragManagerTest2
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    auto callback = [&promiseFlag](const DragNotifyMsg &notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, 0);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<TestStartDragListener>(callback));
    ASSERT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: DragManagerTest3
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest3, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::promise<bool> promiseFlag;
    SHADOWINFO_X = 2;
    auto callback = [&promiseFlag](const DragNotifyMsg &notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<TestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragManagerTest4
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest4, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    SHADOWINFO_X = 0;
    std::promise<bool> promiseFlag;
    auto callback = [&promiseFlag](const DragNotifyMsg &notifyMessage) {
        FI_HILOGD("displayX:%{public}d, displayY:%{public}d, result:%{public}d",
            notifyMessage.displayX, notifyMessage.displayY, notifyMessage.result);
        promiseFlag.set_value(true);
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, -1, false, SHADOW_NUM_ONE);
    ASSERT_TRUE(dragData);
    int32_t ret = InteractionManager::GetInstance()->StartDrag(dragData.value(),
        std::make_shared<TestStartDragListener>(callback));
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragManagerTest5
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest5, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = InteractionManager::GetInstance()->AddDraglistener(nullptr);
    ASSERT_EQ(ret, RET_ERR);
    ret = InteractionManager::GetInstance()->RemoveDraglistener(nullptr);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest6
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest6, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto listener = std::make_shared<DragListenerTest>(std::string("Draglistener_Mouse"));
    int32_t ret = InteractionManager::GetInstance()->AddDraglistener(listener);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest7
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest7, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = InteractionManager::GetInstance()->AddSubscriptListener(nullptr);
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragManagerTest8
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest8, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto listener = std::make_shared<SubscriptListenerTest>("SubscriptListener");
    int32_t ret = InteractionManager::GetInstance()->AddSubscriptListener(listener);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest9
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest9, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    auto listener = std::make_shared<SubscriptListenerTest>("SubscriptListener");
    int32_t ret = InteractionManager::GetInstance()->RemoveSubscriptListener(listener);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest10
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest10, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = InteractionManager::GetInstance()->RemoveSubscriptListener(nullptr);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest11
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest11, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = InteractionManager::GetInstance()->RemoveSubscriptListener(nullptr);
    auto listener = std::make_shared<SubscriptListenerTest>("SubscriptListener");
    ret = InteractionManager::GetInstance()->AddSubscriptListener(listener);
    ret = InteractionManager::GetInstance()->RemoveSubscriptListener(nullptr);
    ASSERT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragManagerTest12
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest12, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = InteractionManager::GetInstance()->UpdateDragStyle(static_cast<DragCursorStyle>(-1));
    ASSERT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragManagerTest13
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragManagerTest, DragManagerTest13, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(PIXEL_MAP_WIDTH, PIXEL_MAP_HEIGHT);
    ASSERT_NE(pixelMap, nullptr);
    ShadowInfo shadowInfo = { pixelMap, 1, 0 };
    int32_t ret = InteractionManager::GetInstance()->UpdateShadowPic(shadowInfo);
    ASSERT_EQ(ret, RET_ERR);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
