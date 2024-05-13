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

#define BUFF_SIZE 100
#include "drag_server_test.h"

#include "coordination_event_manager.h"
#include "devicestatus_service.h"
#include "drag_data_manager.h"
#include "drag_params.h"
#include "drag_server.h"
#include "interaction_manager.h"
#include "ipc_skeleton.h"
#include "singleton.h"
#include "tunnel_client.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using namespace testing::ext;
namespace {
constexpr int32_t TIME_WAIT_FOR_OP_MS { 20 };
constexpr int32_t POINTER_ID { 0 };
constexpr int32_t DRAG_NUM_ONE { 1 };
constexpr int32_t SHADOW_NUM_ONE { 1 };
constexpr int32_t PIXEL_MAP_WIDTH { 3 };
constexpr int32_t PIXEL_MAP_HEIGHT { 3 };
constexpr int32_t WINDOW_ID { -1 };
constexpr int32_t READ_OK { 1 };
constexpr uint32_t DEFAULT_ICON_COLOR { 0xFF };
const std::string FILTER_INFO { "Undefined filter info" };
const std::string UD_KEY { "Unified data key" };
const std::string EXTRA_INFO { "Undefined extra info" };
const std::string CURVE_NAME { "cubic-bezier" };
constexpr int32_t FOREGROUND_COLOR_IN { 0x33FF0000 };
constexpr int32_t DISPLAY_ID { 0 };
constexpr int32_t DISPLAY_X { 50 };
constexpr int32_t DISPLAY_Y { 50 };
constexpr int32_t INT32_BYTE { 4 };
int32_t g_shadowinfo_x { 0 };
int32_t g_shadowinfo_y { 0 };
constexpr int32_t ANIMATION_DURATION { 500 };
constexpr int32_t MAX_PIXEL_MAP_WIDTH { 600 };
constexpr int32_t MAX_PIXEL_MAP_HEIGHT { 600 };
constexpr bool HAS_CANCELED_ANIMATION { true };
constexpr bool HAS_CUSTOM_ANIMATION { true };
Intention g_intention { Intention::UNKNOWN_INTENTION };
std::shared_ptr<DragServer> g_dragServer { nullptr };
std::shared_ptr<DragServer> g_dragServerOne { nullptr };
IContext *g_context { nullptr };
IContext *g_contextOne { nullptr };
std::shared_ptr<TunnelClient> g_tunnel { nullptr };
} // namespace

void DragServerTest::SetUpTestCase() {}

void DragServerTest::SetUp()
{
    g_context = dynamic_cast<IContext *>(DelayedSingleton<DeviceStatusService>::GetInstance().get());
    g_dragServer = std::make_shared<DragServer>(g_context);
    g_dragServerOne = std::make_shared<DragServer>(g_contextOne);
    g_tunnel = std::make_shared<TunnelClient>();
}

void DragServerTest::TearDown()
{
    g_dragServer = nullptr;
    g_context = nullptr;
    g_tunnel = nullptr;
    g_dragServerOne = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_WAIT_FOR_OP_MS));
}

std::shared_ptr<Media::PixelMap> DragServerTest::CreatePixelMap(int32_t width, int32_t height)
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

std::optional<DragData> DragServerTest::CreateDragData(int32_t sourceType,
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
        dragData.shadowInfos.push_back({ pixelMap, g_shadowinfo_x, g_shadowinfo_y });
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

void DragServerTest::AssignToAnimation(PreviewAnimation &animation)
{
    animation.duration = ANIMATION_DURATION;
    animation.curveName = CURVE_NAME;
    animation.curve = { 0.33, 0, 0.67, 1 };
}

/**
 * @tc.name: DragServerTest1
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest1, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    int32_t ret = g_dragServer->Enable(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest2
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest2, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->Disable(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest3
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest3, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->Start(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest4
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest4, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->Stop(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest5
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest5, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    int32_t ret = -1;
    MessageParcel reply;
    MessageParcel datas;
    std::vector<DragRequestID> dragRequestIDs = {DragRequestID::UNKNOWN_DRAG_ACTION,
        DragRequestID::ADD_DRAG_LISTENER, DragRequestID::ADD_SUBSCRIPT_LISTENER};
    for (const auto& dragRequestID : dragRequestIDs) {
        GTEST_LOG_(INFO) << "dragRequestID: " << dragRequestID;
        ret = g_dragServer->AddWatch(context, dragRequestID, datas, reply);
        EXPECT_EQ(ret, RET_ERR);
    }
}

/**
 * @tc.name: DragServerTest6
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest6, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = -1;
    std::vector<DragRequestID> dragRequestIDs = {DragRequestID::UNKNOWN_DRAG_ACTION,
        DragRequestID::ADD_DRAG_LISTENER, DragRequestID::ADD_SUBSCRIPT_LISTENER};
    for (const auto& dragRequestID : dragRequestIDs) {
        GTEST_LOG_(INFO) << "dragRequestID: " << dragRequestID;
        ret = g_dragServer->RemoveWatch(context, dragRequestID, datas, reply);
        EXPECT_EQ(ret, RET_ERR);
    }
}

/**
 * @tc.name: DragServerTest7
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest7, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = -1;
    std::vector<DragRequestID> dragRequestIDs = {DragRequestID::UNKNOWN_DRAG_ACTION,
        DragRequestID::SET_DRAG_WINDOW_VISIBLE, DragRequestID::UPDATE_DRAG_STYLE,
        DragRequestID::UPDATE_SHADOW_PIC, DragRequestID::UPDATE_PREVIEW_STYLE,
        DragRequestID::UPDATE_PREVIEW_STYLE_WITH_ANIMATION};
    for (const auto& dragRequestID : dragRequestIDs) {
        GTEST_LOG_(INFO) << "dragRequestID: " << dragRequestID;
        ret = g_dragServer->SetParam(context, dragRequestID, datas, reply);
        EXPECT_EQ(ret, RET_ERR);
    }
}

/**
 * @tc.name: DragServerTest8
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest8, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = -1;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    std::vector<DragRequestID> dragRequestIDs = {DragRequestID::UNKNOWN_DRAG_ACTION,
        DragRequestID::GET_DRAG_TARGET_PID, DragRequestID::GET_UDKEY,
        DragRequestID::GET_SHADOW_OFFSET, DragRequestID::GET_DRAG_DATA,
        DragRequestID::GET_DRAG_STATE, DragRequestID::GET_DRAG_SUMMARY,
        DragRequestID::GET_DRAG_ACTION, DragRequestID::GET_EXTRA_INFO};
    for (const auto& dragRequestID : dragRequestIDs) {
        GTEST_LOG_(INFO) << "dragRequestID: " << dragRequestID;
        if (dragRequestID == DragRequestID::UNKNOWN_DRAG_ACTION ||
            dragRequestID == DragRequestID::GET_UDKEY ||
            dragRequestID == DragRequestID::GET_SHADOW_OFFSET ||
            dragRequestID == DragRequestID::GET_DRAG_DATA ||
            dragRequestID == DragRequestID::GET_DRAG_ACTION||
            dragRequestID == DragRequestID::GET_EXTRA_INFO) {
                ret = g_dragServer->GetParam(context, dragRequestID, datas, reply);
                EXPECT_EQ(ret, RET_ERR);
        } else {
            ret = g_dragServer->GetParam(context, dragRequestID, datas, reply);
            EXPECT_EQ(ret, RET_OK);
        }
    }
}

/**
 * @tc.name: DragServerTest9
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest9, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    int32_t ret = -1;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    std::vector<DragRequestID> dragRequestIDs = {DragRequestID::UNKNOWN_DRAG_ACTION,
        DragRequestID::ADD_PRIVILEGE, DragRequestID::ENTER_TEXT_EDITOR_AREA};
    for (const auto& dragRequestID : dragRequestIDs) {
        GTEST_LOG_(INFO) << "dragRequestID: " << dragRequestID;
        ret = g_dragServer->Control(context, dragRequestID, datas, reply);
        EXPECT_EQ(ret, RET_ERR);
    }
}

/**
 * @tc.name: DragServerTest10
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest10, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->SetDragWindowVisible(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest11
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest11, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->UpdateDragStyle(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest12
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest12, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->UpdateShadowPic(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest13
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest13, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->UpdatePreviewStyle(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest14
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest14, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->UpdatePreviewAnimation(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest15
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest15, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragTargetPid(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragServerTest16
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest16, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DRAG_DATA_MGR.Init(dragData.value());
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetUdKey(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DRAG_DATA_MGR.dragData_ = {};
}

/**
 * @tc.name: DragServerTest17
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest17, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetShadowOffset(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest18
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest18, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragData(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest19
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest19, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragState(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragServerTest20
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest20, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragSummary(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
}

/**
 * @tc.name: DragServerTest21
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest21, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragAction(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest22
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest22, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetExtraInfo(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest23
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest23, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->EnterTextEditorArea(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest24
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest24, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetUdKey(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest25
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest25, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DRAG_DATA_MGR.Init(dragData.value());
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetShadowOffset(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DRAG_DATA_MGR.dragData_ = {};
}

/**
 * @tc.name: DragServerTest26
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest26, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DRAG_DATA_MGR.Init(dragData.value());
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetExtraInfo(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DRAG_DATA_MGR.dragData_ = {};
}

/**
 * @tc.name: DragServerTest27
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest27, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    
    MessageParcel reply;
    MessageParcel datas;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    SetDragWindowVisibleParam param { true, true };
    int32_t ret = param.Marshalling(datas);
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->SetDragWindowVisible(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
    DRAG_DATA_MGR.dragData_ = {};
}

/**
 * @tc.name: DragServerTest28
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest28, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    StopDragParam param { dropResult };

    int32_t ret = param.Marshalling(datas);
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->Stop(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest29
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest29, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DragDropResult dropResult { DragResult::DRAG_SUCCESS, HAS_CUSTOM_ANIMATION, WINDOW_ID };
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    StopDragParam param { dropResult };

    int32_t ret = param.Marshalling(datas);
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServerOne->Stop(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest30
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest30, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    UpdateDragStyleParam param { DragCursorStyle::COPY };
    bool ret = param.Marshalling(datas);
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->UpdateDragStyle(context, datas, reply);
    EXPECT_TRUE(ret);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest31
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest31, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    std::shared_ptr<Media::PixelMap> pixelMap = CreatePixelMap(PIXEL_MAP_WIDTH, PIXEL_MAP_HEIGHT);
    ASSERT_NE(pixelMap, nullptr);
    ShadowInfo shadowInfo = { pixelMap, 0, 0 };
    std::string extraInfo;
    UpdateShadowPicParam param { shadowInfo };
    bool ret = param.Marshalling(datas);;
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->UpdateShadowPic(context, datas, reply);
    EXPECT_TRUE(ret);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest32
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest32, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    PreviewStyle previewStyleIn;
    previewStyleIn.types = { PreviewType::FOREGROUND_COLOR };
    previewStyleIn.foregroundColor = FOREGROUND_COLOR_IN;
    UpdatePreviewStyleParam param { previewStyleIn };
    bool ret = param.Marshalling(datas);;
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->UpdatePreviewStyle(context, datas, reply);
    EXPECT_TRUE(ret);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest33
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest33, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    PreviewStyle previewStyleIn;
    previewStyleIn.types = { PreviewType::FOREGROUND_COLOR };
    previewStyleIn.foregroundColor = FOREGROUND_COLOR_IN;
    PreviewAnimation animationOut;
    AssignToAnimation(animationOut);
    UpdatePreviewAnimationParam param { previewStyleIn, animationOut };
    bool ret = param.Marshalling(datas);;
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->UpdatePreviewAnimation(context, datas, reply);
    EXPECT_FALSE(ret);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest34
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest34, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel datas;
    MessageParcel reply;
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    GetDragTargetPidReply targetPidReply { IPCSkeleton::GetCallingPid() };
    bool ret = targetPidReply.Marshalling(datas);
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->GetDragTargetPid(context, datas, reply);
    EXPECT_FALSE(ret);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest35
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest35, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    MessageParcel reply;
    MessageParcel datas;
    DRAG_DATA_MGR.Init(dragData.value());
    int32_t ret = g_dragServer->GetDragData(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DRAG_DATA_MGR.dragData_ = {};
    ret = g_dragServer->GetDragData(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest36
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest36, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::ERROR;
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragState(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    ret = g_dragServer->GetDragState(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest37
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest37, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::ERROR;
    MessageParcel reply;
    MessageParcel datas;
    int32_t ret = g_dragServer->GetDragAction(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::START;
    ret = g_dragServer->GetDragAction(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DelayedSingleton<DeviceStatusService>::GetInstance()->dragMgr_.dragState_ = DragState::STOP;
}

/**
 * @tc.name: DragServerTest38
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest38, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    std::optional<DragData> dragData = CreateDragData(
        MMI::PointerEvent::SOURCE_TYPE_MOUSE, POINTER_ID, DRAG_NUM_ONE, false, SHADOW_NUM_ONE);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    DRAG_DATA_MGR.Init(dragData.value());
    int32_t ret = g_dragServer->GetExtraInfo(context, datas, reply);
    EXPECT_EQ(ret, RET_OK);
    DRAG_DATA_MGR.dragData_ = {};
    ret = g_dragServer->GetExtraInfo(context, datas, reply);
    EXPECT_EQ(ret, RET_ERR);
}

/**
 * @tc.name: DragServerTest39
 * @tc.desc: Drag Drawing
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DragServerTest, DragServerTest39, TestSize.Level0)
{
    CALL_TEST_DEBUG;
    g_dragServer->GetPackageName(IPCSkeleton::GetCallingTokenID());
    g_dragServer->GetPackageName(-1);
    CallingContext context {
        .intention = g_intention,
        .tokenId = IPCSkeleton::GetCallingTokenID(),
        .uid = IPCSkeleton::GetCallingUid(),
        .pid = IPCSkeleton::GetCallingPid(),
    };
    MessageParcel reply;
    MessageParcel datas;
    EnterTextEditorAreaParam param { true };
    bool ret = param.Marshalling(datas);;
    EXPECT_EQ(ret, READ_OK);
    ret = g_dragServer->EnterTextEditorArea(context, datas, reply);
    EXPECT_EQ(ret, READ_OK);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
