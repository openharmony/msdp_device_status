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

#include "drag_drawing.h"

#include <atomic>
#include <cstdint>
#include <fstream>
#include <string>

#include "../wm/window.h"
#include "display_manager.h"
#include "include/core/SkTextBlob.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "pointer_event.h"
#include "string_ex.h"
#include "transaction/rs_interfaces.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_root_node.h"
#include "ui/rs_surface_extractor.h"
#include "ui/rs_surface_node.h"
#include "ui/rs_ui_director.h"

#include "devicestatus_define.h"
#include "drag_data_adapter.h"
#include "util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawing" };
constexpr int32_t BASELINE_DENSITY = 160;
constexpr int32_t DEVICE_INDEPENDENT_PIXELS = 40;
constexpr int32_t EIGHT_SIZE = 8;
constexpr int32_t IMAGE_WIDTH = 400;
constexpr int32_t IMAGE_HEIGHT = 500;
constexpr int64_t START_TIME = 181154000809;
constexpr int64_t INTERVAL_TIME = 16666667;
constexpr int32_t FRAMERATE = 30;
constexpr int32_t SVG_HEIGHT = 40;
constexpr int32_t SIXTEEN = 16;
constexpr int32_t FAIL_ANIMATION_DURATION = 1000;
constexpr int32_t SUCCESS_ANIMATION_DURATION = 300;
constexpr int32_t VIEW_BOX_POS = 2;
constexpr int32_t PIXEL_MAP_INDEX = 0;
constexpr int32_t DRAG_STYLE_INDEX = 1;
constexpr int32_t MOUSE_ICON_INDEX = 2;
constexpr size_t TOUCH_NODE_MIN_COUNT = 2;
constexpr size_t MOUSE_NODE_MIN_COUNT = 3;
constexpr double ONETHOUSAND = 1000.0;
constexpr float BEGIN_ALPHA = 1.0f;
constexpr float END_ALPHA = 0.0f;
constexpr float BEGIN_SCALE = 1.0f;
constexpr float END_SCALE_SUCCESS = 1.2f;
constexpr float END_SCALE_FAIL = 0.1f;
constexpr float PIVOT_X = 0.5f;
constexpr float PIVOT_Y = 0.5f;
const std::string COPY_DRAG_PATH = "/system/etc/device_status/drag_icon/Copy_Drag.svg";
const std::string COPY_ONE_DRAG_PATH = "/system/etc/device_status/drag_icon/Copy_One_Drag.svg";
const std::string FORBID_DRAG_PATH = "/system/etc/device_status/drag_icon/Forbid_Drag.svg";
const std::string MOUSE_DRAG_PATH = "/system/etc/device_status/drag_icon/Mouse_Drag.png";
struct DrawingInfo {
    std::atomic<bool> isRunning { false };
    int32_t sourceType { -1 };
    int32_t currentDragNum { -1 };
    DragCursorStyle currentStyle { DragCursorStyle::DEFAULT };
    int32_t displayId { -1 };
    int32_t pixelMapX { -1 };
    int32_t pixelMapY { -1 };
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t rootNodeWidth { -1 };
    int32_t rootNodeHeight { -1 };
    sptr<OHOS::Rosen::Window> dragWindow { nullptr };
    std::vector<std::shared_ptr<OHOS::Rosen::RSCanvasNode>> nodes;
    std::shared_ptr<OHOS::Rosen::RSNode> rootNode { nullptr };
    std::shared_ptr<OHOS::Rosen::RSSurfaceNode> surfaceNode { nullptr };
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap { nullptr };
} g_drawingInfo;
} // namespace

int32_t DragDrawing::Init(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.isRunning) {
        FI_HILOGE("Drag drawing is running, can not init again");
        return INIT_CANCEL;
    }
    CHKPR(dragData.shadowInfo.pixelMap, INIT_FAIL);
    if ((dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN)) {
        FI_HILOGE("Invalid sourceType:%{public}d", dragData.sourceType);
        return INIT_FAIL;
    }
    if (dragData.dragNum < 0) {
        FI_HILOGE("Invalid dragNum:%{public}d", dragData.dragNum);
        return INIT_FAIL;
    }
    InitDrawingInfo(dragData);
    CreateWindow(dragData.displayX, dragData.displayY);
    CHKPR(g_drawingInfo.dragWindow, INIT_FAIL);
    g_drawingInfo.dragWindow->Show();
    if (InitLayer() != RET_OK) {
        FI_HILOGE("Init layer failed");
        return INIT_FAIL;
    }
    if (DrawShadow() != RET_OK) {
        FI_HILOGE("Draw shadow failed");
        return INIT_FAIL;
    }
    if (DrawStyle() != RET_OK) {
        FI_HILOGE("Draw style failed");
        return INIT_FAIL;
    }
    InitAnimation();
    CHKPR(rsUiDirector_, RET_ERR);
    if (g_drawingInfo.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        rsUiDirector_->SendMessages();
        return RET_OK;
    }
    if (DrawMouseIcon() != RET_OK) {
        FI_HILOGE("Draw mouse icon failed");
        return INIT_FAIL;
    }
    rsUiDirector_->SendMessages();
    return INIT_SUCCESS;
}

void DragDrawing::Draw(int32_t displayId, int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    if (displayId < 0) {
        FI_HILOGE("Invalid display:%{public}d", displayId);
        return;
    }
    g_drawingInfo.displayId = displayId;
    g_drawingInfo.displayX = displayX;
    g_drawingInfo.displayY = displayY;
    if (displayX < 0) {
        g_drawingInfo.displayX = 0;
    }
    if (displayY < 0) {
        g_drawingInfo.displayY = 0;
    }

    if (g_drawingInfo.dragWindow != nullptr) {
        g_drawingInfo.dragWindow->MoveTo(g_drawingInfo.displayX + g_drawingInfo.pixelMapX,
            g_drawingInfo.displayY + g_drawingInfo.pixelMapY);
        return;
    }
    CreateWindow(g_drawingInfo.displayX + g_drawingInfo.pixelMapX, g_drawingInfo.displayY + g_drawingInfo.pixelMapY);
    CHKPV(g_drawingInfo.dragWindow);
    g_drawingInfo.dragWindow->Show();
}

int32_t DragDrawing::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    if (style < DragCursorStyle::DEFAULT || style > DragCursorStyle::MOVE) {
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if (g_drawingInfo.currentStyle == style) {
        FI_HILOGD("Not need update drag style");
        return RET_OK;
    }
    g_drawingInfo.currentStyle = style;
    DrawStyle();
    CHKPR(rsUiDirector_, RET_ERR);
    rsUiDirector_->SendMessages();
    return RET_OK;
}

void DragDrawing::OnDragSuccess()
{
    CALL_DEBUG_ENTER;
    if (drawDynamicEffectModifier_ != nullptr) {
        CHKPV(g_drawingInfo.rootNode);
        g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
    }
    drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(BEGIN_ALPHA);
    drawDynamicEffectModifier_->SetScale(BEGIN_SCALE);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(SUCCESS_ANIMATION_DURATION);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(END_ALPHA);
        drawDynamicEffectModifier_->SetScale(END_SCALE_SUCCESS);
    });
    CHKPV(runner_);
    runner_->Run();
}

void DragDrawing::OnDragFail()
{
    CALL_DEBUG_ENTER;
    if (drawDynamicEffectModifier_ != nullptr) {
        CHKPV(g_drawingInfo.rootNode);
        g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
    }
    drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(BEGIN_ALPHA);
    drawDynamicEffectModifier_->SetScale(BEGIN_SCALE);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(FAIL_ANIMATION_DURATION);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(END_ALPHA);
        drawDynamicEffectModifier_->SetScale(END_SCALE_FAIL);
    });
    CHKPV(runner_);
    runner_->Run();
}

void DragDrawing::EraseMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Nodes size invalid, node size:%{public}zu", g_drawingInfo.nodes.size());
        return;
    }
    auto mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->RemoveChild(mouseIconNode);
    CHKPV(rsUiDirector_);
    rsUiDirector_->SendMessages();
}

void DragDrawing::DestroyDragWindow()
{
    CALL_DEBUG_ENTER;
    startNum_ = START_TIME;
    rsUiDirector_ = nullptr;
    g_drawingInfo.currentStyle = DragCursorStyle::DEFAULT;
    g_drawingInfo.pixelMap = nullptr;
    g_drawingInfo.surfaceNode = nullptr;
    if (!g_drawingInfo.nodes.empty()) {
        g_drawingInfo.nodes.clear();
    }
    if (g_drawingInfo.rootNode != nullptr) {
        g_drawingInfo.rootNode->ClearChildren();
        g_drawingInfo.rootNode = nullptr;
    }
    if (g_drawingInfo.dragWindow != nullptr) {
        g_drawingInfo.dragWindow->Destroy();
        g_drawingInfo.dragWindow = nullptr;
    }
}

void DragDrawing::UpdateDrawingState()
{
    CALL_DEBUG_ENTER;
    g_drawingInfo.isRunning = false;
}

void DragDrawing::InitAnimation()
{
    CALL_DEBUG_ENTER;
    if (runner_ == nullptr) {
        runner_ = AppExecFwk::EventRunner::Create(false);
    }
    if (handler_ == nullptr) {
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    }
    handler_->PostTask(std::bind(&DragDrawing::InitVSync, this));
}

int32_t DragDrawing::DrawShadow()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when mouse type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when touchscreen type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    auto pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPR(pixelMapNode, RET_ERR);
    if (drawPixelMapModifier_ != nullptr) {
        pixelMapNode->RemoveModifier(drawPixelMapModifier_);
    }
    drawPixelMapModifier_ = std::make_shared<DrawPixelMapModifier>();
    pixelMapNode->AddModifier(drawPixelMapModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Nodes size invalid, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    auto mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPR(mouseIconNode, RET_ERR);
    if (drawMouseIconModifier_ != nullptr) {
        mouseIconNode->RemoveModifier(drawMouseIconModifier_);
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    mouseIconNode->AddModifier(drawMouseIconModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawStyle()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when mouse type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when touchscreen type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    auto dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPR(dragStyleNode, RET_ERR);
    if (drawSVGModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawSVGModifier_);
    }
    drawSVGModifier_ = std::make_shared<DrawSVGModifier>();
    dragStyleNode->AddModifier(drawSVGModifier_);
    return RET_OK;
}

int32_t DragDrawing::InitVSync()
{
    CALL_DEBUG_ENTER;
    startNum_ = START_TIME;
    CHKPR(g_drawingInfo.surfaceNode, RET_ERR);
    g_drawingInfo.surfaceNode->SetPivot(PIVOT_X, PIVOT_Y);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    auto& rsClient = OHOS::Rosen::RSInterfaces::GetInstance();
    if (receiver_ == nullptr) {
        CHKPR(handler_, RET_ERR);
        receiver_ = rsClient.CreateVSyncReceiver("DragDrawing", handler_);
    }
    CHKPR(receiver_, RET_ERR);
    int32_t ret = receiver_->Init();
    if (ret != RET_OK) {
        FI_HILOGE("Receiver init failed");
        return RET_ERR;
    }

    OHOS::Rosen::VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = std::bind(&DragDrawing::OnVsync, this),
    };
    int32_t changeFreq = static_cast<int32_t>((ONETHOUSAND / FRAMERATE) / SIXTEEN);
    ret = receiver_->SetVSyncRate(fcb, changeFreq);
    if (ret) {
        FI_HILOGE("Set vsync rate failed");
        return RET_ERR;
    }
    return RET_OK;
}

void DragDrawing::OnVsync()
{
    CALL_DEBUG_ENTER;
    CHKPV(rsUiDirector_);
    bool hasRunningAnimation = rsUiDirector_->RunningCustomAnimation(startNum_);
    if (!hasRunningAnimation) {
        FI_HILOGD("Stop runner, hasRunningAnimation:%{public}d", hasRunningAnimation);
        CHKPV(runner_);
        runner_->Stop();
        DestroyDragWindow();
        g_drawingInfo.isRunning = false;
        return;
    }
    rsUiDirector_->SendMessages();
    startNum_ += INTERVAL_TIME;
}

void DragDrawing::InitDrawingInfo(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    g_drawingInfo.isRunning = true;
    g_drawingInfo.currentDragNum = dragData.dragNum;
    g_drawingInfo.sourceType = dragData.sourceType;
    g_drawingInfo.displayId = dragData.displayId;
    g_drawingInfo.pixelMap = dragData.shadowInfo.pixelMap;
    g_drawingInfo.pixelMapX = dragData.shadowInfo.x;
    g_drawingInfo.pixelMapY = dragData.shadowInfo.y;
}

int32_t DragDrawing::InitLayer()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.dragWindow == nullptr) {
        FI_HILOGE("Init layer failed, dragWindow is nullptr");
        return RET_ERR;
    }
    g_drawingInfo.surfaceNode = g_drawingInfo.dragWindow->GetSurfaceNode();
    if (g_drawingInfo.surfaceNode == nullptr) {
        g_drawingInfo.dragWindow->Destroy();
        g_drawingInfo.dragWindow = nullptr;
        FI_HILOGE("Init layer failed, surfaceNode is nullptr");
        return RET_ERR;
    }
    auto surface = g_drawingInfo.surfaceNode->GetSurface();
    if (surface == nullptr) {
        g_drawingInfo.dragWindow->Destroy();
        g_drawingInfo.dragWindow = nullptr;
        FI_HILOGE("Init layer failed, surface is nullptr");
        return RET_ERR;
    }
    rsUiDirector_ = OHOS::Rosen::RSUIDirector::Create();
    CHKPR(rsUiDirector_, RET_ERR);
    rsUiDirector_->Init();
    rsUiDirector_->SetRSSurfaceNode(g_drawingInfo.surfaceNode);
    InitCanvas(IMAGE_WIDTH, IMAGE_HEIGHT);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    return RET_OK;
}

void DragDrawing::InitCanvas(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.rootNode == nullptr) {
        g_drawingInfo.rootNode = OHOS::Rosen::RSRootNode::Create();
        CHKPV(g_drawingInfo.rootNode);
    }
    g_drawingInfo.rootNode->SetBounds(g_drawingInfo.displayX, g_drawingInfo.displayY, width, height);
    g_drawingInfo.rootNode->SetFrame(g_drawingInfo.displayX, g_drawingInfo.displayY, width, height);
    g_drawingInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);

    auto pixelMapNode = OHOS::Rosen::RSCanvasNode::Create();
    CHKPV(pixelMapNode);
    CHKPV(g_drawingInfo.pixelMap);
    pixelMapNode->SetBounds(0, SVG_HEIGHT, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    pixelMapNode->SetFrame(0, SVG_HEIGHT, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.nodes.emplace_back(pixelMapNode);

    auto dragStyleNode = OHOS::Rosen::RSCanvasNode::Create();
    CHKPV(dragStyleNode);
    dragStyleNode->SetBounds(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    dragStyleNode->SetFrame(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    g_drawingInfo.nodes.emplace_back(dragStyleNode);

    if (g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        auto mouseIconNode = OHOS::Rosen::RSCanvasNode::Create();
        CHKPV(mouseIconNode);
        mouseIconNode->SetBounds(ChangeNumber(g_drawingInfo.pixelMapX), ChangeNumber(g_drawingInfo.pixelMapY),
            SVG_HEIGHT, SVG_HEIGHT);
        mouseIconNode->SetFrame(ChangeNumber(g_drawingInfo.pixelMapX), ChangeNumber(g_drawingInfo.pixelMapY),
            SVG_HEIGHT, SVG_HEIGHT);
        g_drawingInfo.nodes.emplace_back(mouseIconNode);
        g_drawingInfo.rootNode->AddChild(pixelMapNode);
        g_drawingInfo.rootNode->AddChild(dragStyleNode);
        g_drawingInfo.rootNode->AddChild(mouseIconNode);
        rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
        return;
    }
    g_drawingInfo.rootNode->AddChild(pixelMapNode);
    g_drawingInfo.rootNode->AddChild(dragStyleNode);
    rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
}

void DragDrawing::CreateWindow(int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    sptr<OHOS::Rosen::WindowOption> option = new (std::nothrow) OHOS::Rosen::WindowOption();
    CHKPV(option);
    option->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_POINTER);
    option->SetWindowMode(OHOS::Rosen::WindowMode::WINDOW_MODE_FLOATING);
    OHOS::Rosen::Rect rect = {
        .posX_ = displayX,
        .posY_ = displayY,
        .width_ = IMAGE_WIDTH,
        .height_ = IMAGE_HEIGHT,
    };
    option->SetWindowRect(rect);
    option->SetFocusable(false);
    option->SetTouchable(false);
    std::string windowName = "drag window";
    g_drawingInfo.dragWindow = OHOS::Rosen::Window::Create(windowName, option, nullptr);
}

void DrawSVGModifier::Draw(OHOS::Rosen::RSDrawingContext& context) const
{
    CALL_DEBUG_ENTER;
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    std::string filePath;
    if (g_drawingInfo.currentStyle == DragCursorStyle::FORBIDDEN) {
        filePath = FORBID_DRAG_PATH;
    } else if (g_drawingInfo.currentStyle == DragCursorStyle::COPY) {
        filePath = COPY_DRAG_PATH;
    } else {
        filePath = COPY_ONE_DRAG_PATH;
    }
    
    if (!IsValidSvgFile(filePath)) {
        FI_HILOGE("Svg file is invalid");
        return;
    }
    auto pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPV(pixelMapNode);
    CHKPV(g_drawingInfo.pixelMap);
    pixelMapNode->SetBounds(0, EIGHT_SIZE, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    pixelMapNode->SetFrame(0, EIGHT_SIZE, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = DecodeSvgToPixelMap(filePath);
    CHKPV(pixelMap);
    int32_t svgTouchPositionX = g_drawingInfo.pixelMap->GetWidth() + EIGHT_SIZE - pixelMap->GetWidth();
    if (svgTouchPositionX < 0) {
        FI_HILOGD("svgTouchPositionX:%{public}d", svgTouchPositionX);
        svgTouchPositionX = 0;
    }
    auto dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    dragStyleNode->SetBounds(svgTouchPositionX, 0, pixelMap->GetWidth(), pixelMap->GetHeight());
    dragStyleNode->SetFrame(svgTouchPositionX, 0, pixelMap->GetWidth(), pixelMap->GetHeight());
    dragStyleNode->SetBgImageWidth(pixelMap->GetWidth());
    dragStyleNode->SetBgImageHeight(SVG_HEIGHT);
    dragStyleNode->SetBgImagePositionX(0);
    dragStyleNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    dragStyleNode->SetBgImage(rosenImage);
    g_drawingInfo.rootNodeWidth = g_drawingInfo.pixelMap->GetWidth() + EIGHT_SIZE;
    g_drawingInfo.rootNodeHeight = pixelMap->GetHeight() + g_drawingInfo.pixelMap->GetHeight() + EIGHT_SIZE;
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->SetBounds(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.rootNode->SetFrame(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    CHKPV(g_drawingInfo.dragWindow);
    g_drawingInfo.dragWindow->Resize(g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
}

int32_t DrawSVGModifier::UpdateSvgNodeInfo(xmlNodePtr curNode, int32_t extendSvgWidth) const
{
    CALL_DEBUG_ENTER;
    if (xmlStrcmp(curNode->name, BAD_CAST "svg")) {
        FI_HILOGE("Svg format invalid");
        return RET_ERR;
    }
    std::ostringstream oStrStream;
    oStrStream << xmlGetProp(curNode, BAD_CAST "width");
    std::string srcSvgWidth = oStrStream.str();
    if (!IsNum(srcSvgWidth)) {
        FI_HILOGE("srcSvgWidth is not digital, srcSvgWidth:%{public}s", srcSvgWidth.c_str());
        return RET_ERR;
    }
    int32_t number = std::stoi(srcSvgWidth) + extendSvgWidth;
    std::string tgtSvgWidth  = std::to_string(number);
    xmlSetProp(curNode, BAD_CAST "width", BAD_CAST tgtSvgWidth.c_str());
    oStrStream.clear();
    oStrStream << xmlGetProp(curNode, BAD_CAST "viewBox");
    std::string srcViewBox = oStrStream.str();
    std::istringstream iStrStream(srcViewBox);
    std::string tmpString;
    std::string tgtViewBox;
    int32_t i = 0;
    while (iStrStream >> tmpString) {
        if (i == VIEW_BOX_POS) {
            if (!IsNum(tmpString)) {
                FI_HILOGE("tmpString is not digital, tmpString:%{public}s", tmpString.c_str());
                return RET_ERR;
            }
            number = std::stoi(tmpString) + extendSvgWidth;
            tmpString = std::to_string(number);
        }
        tgtViewBox.append(tmpString);
        tgtViewBox += " ";
        ++i;
    }

    xmlSetProp(curNode, BAD_CAST "viewBox", BAD_CAST tgtViewBox.c_str());
    return RET_OK;
}

xmlNodePtr DrawSVGModifier::GetRectNode(xmlNodePtr curNode) const
{
    CALL_DEBUG_ENTER;
    curNode = curNode->xmlChildrenNode;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "g")) {
            while (!xmlStrcmp(curNode->name, BAD_CAST "g")) {
                curNode = curNode->xmlChildrenNode;
            }
            break;
        }
        curNode = curNode->next;
    }
    return curNode;
}

xmlNodePtr DrawSVGModifier::UpdateRectNode(xmlNodePtr curNode, int32_t extendSvgWidth) const
{
    CALL_DEBUG_ENTER;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "rect")) {
            std::ostringstream oStrStream;
            oStrStream << xmlGetProp(curNode, BAD_CAST "width");
            std::string srcRectWidth = oStrStream.str();
            if (!IsNum(srcRectWidth)) {
                FI_HILOGE("srcRectWidth is not digital, srcRectWidth:%{public}s", srcRectWidth.c_str());
                return nullptr;
            }
            int32_t number = std::stoi(srcRectWidth) + extendSvgWidth;
            xmlSetProp(curNode, BAD_CAST "width", BAD_CAST std::to_string(number).c_str());
        }
        if (!xmlStrcmp(curNode->name, BAD_CAST "text")) {
            return curNode->xmlChildrenNode;
        }
        curNode = curNode->next;
    }
    return nullptr;
}

void DrawSVGModifier::UpdateTspanNode(xmlNodePtr curNode) const
{
    CALL_DEBUG_ENTER;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "tspan")) {
            xmlNodeSetContent(curNode, BAD_CAST std::to_string(g_drawingInfo.currentDragNum).c_str());
        }
        curNode = curNode->next;
    }
}

int32_t DrawSVGModifier::ParseAndAdjustSvgInfo(xmlNodePtr curNode) const
{
    CALL_DEBUG_ENTER;
    CHKPR(curNode, RET_ERR);
    std::string strStyle = std::to_string(g_drawingInfo.currentDragNum);
    if (strStyle.size() == 1) {
        FI_HILOGE("NO need adjust svg");
        return RET_OK;
    }
    if (strStyle.size() < 1) {
        FI_HILOGE("strStyle size:%{public}zu invalid", strStyle.size());
        return RET_ERR;
    }
    int32_t extendSvgWidth = (strStyle.size() - 1) * EIGHT_SIZE;
    xmlKeepBlanksDefault(0);
    int32_t ret = UpdateSvgNodeInfo(curNode, extendSvgWidth);
    if (ret != RET_OK) {
        FI_HILOGE("Update svg node info failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    curNode = GetRectNode(curNode);
    CHKPR(curNode, RET_ERR);
    curNode = UpdateRectNode(curNode, extendSvgWidth);
    CHKPR(curNode, RET_ERR);
    UpdateTspanNode(curNode);
    return RET_OK;
}

std::shared_ptr<OHOS::Media::PixelMap> DrawSVGModifier::DecodeSvgToPixelMap(
    const std::string &filePath) const
{
    CALL_DEBUG_ENTER;
    xmlDocPtr xmlDoc = xmlReadFile(filePath.c_str(), 0, XML_PARSE_NOBLANKS);
    if (g_drawingInfo.currentStyle != DragCursorStyle::FORBIDDEN) {
        xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
        CHKPP(node);
        int32_t ret = ParseAndAdjustSvgInfo(node);
        if (ret != RET_OK) {
            FI_HILOGE("Parse and adjust svg info failed, ret:%{public}d", ret);
            return nullptr;
        }
    }
    xmlChar *xmlbuff;
    int32_t buffersize;
    xmlDocDumpFormatMemory(xmlDoc, &xmlbuff, &buffersize, 1);
    std::ostringstream oStrStream;
    oStrStream << xmlbuff;
    std::string content = oStrStream.str();
    xmlFree(xmlbuff);
    xmlFreeDoc(xmlDoc);
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/svg+xml";
    uint32_t errCode = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(reinterpret_cast<const uint8_t*>(content.c_str()),
        content.size(), opts, errCode);
    CHKPP(imageSource);
    OHOS::Media::DecodeOptions decodeOpts;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    return pixelMap;
}

void DrawPixelMapModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.pixelMap == nullptr) {
        FI_HILOGE("pixelMap is nullptr");
        return;
    }
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    rosenImage->SetPixelMap(g_drawingInfo.pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t pixelMapWidth = g_drawingInfo.pixelMap->GetWidth();
    int32_t pixelMapHeight = g_drawingInfo.pixelMap->GetHeight();
    auto pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPV(pixelMapNode);
    pixelMapNode->SetBoundsWidth(pixelMapWidth);
    pixelMapNode->SetBoundsHeight(pixelMapHeight);
    pixelMapNode->SetBgImageWidth(pixelMapWidth);
    pixelMapNode->SetBgImageHeight(pixelMapHeight);
    pixelMapNode->SetBgImagePositionX(0);
    pixelMapNode->SetBgImagePositionY(0);
    pixelMapNode->SetBgImage(rosenImage);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawMouseIconModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    std::string imagePath = MOUSE_DRAG_PATH;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/png";
    uint32_t errCode = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(imagePath, opts, errCode);
    CHKPV(imageSource);
    int32_t iconSize = GetIconSize();
    if (iconSize <= 0) {
        FI_HILOGE("Get icon size failed");
        return;
    }
    OHOS::Media::DecodeOptions decodeOpts;
    decodeOpts.desiredSize = {
        .width = iconSize,
        .height = iconSize
    };
    decodeOpts.allocatorType = OHOS::Media::AllocatorType::SHARE_MEM_ALLOC;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    CHKPV(pixelMap);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    auto mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    mouseIconNode->SetBoundsWidth(decodeOpts.desiredSize.width);
    mouseIconNode->SetBoundsHeight(decodeOpts.desiredSize.height);
    mouseIconNode->SetBgImageWidth(decodeOpts.desiredSize.width);
    mouseIconNode->SetBgImageHeight(decodeOpts.desiredSize.height);
    mouseIconNode->SetBgImagePositionX(0);
    mouseIconNode->SetBgImagePositionY(0);
    mouseIconNode->SetBgImage(rosenImage);
}

int32_t DrawMouseIconModifier::GetIconSize() const
{
    CALL_DEBUG_ENTER;
    auto displayInfo = OHOS::Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    CHKPR(displayInfo, RET_ERR);
    return displayInfo->GetDpi() * DEVICE_INDEPENDENT_PIXELS / BASELINE_DENSITY;
}

void DrawDynamicEffectModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->SetAlpha(alpha_->Get());
    CHKPV(scale_);
    CHKPV(g_drawingInfo.surfaceNode);
    g_drawingInfo.surfaceNode->SetScale(scale_->Get(), scale_->Get());
    auto rsSurface = OHOS::Rosen::RSSurfaceExtractor::ExtractRSSurface(g_drawingInfo.surfaceNode);
    CHKPV(rsSurface);
    auto frame = rsSurface->RequestFrame(g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    if (frame == nullptr) {
        FI_HILOGE("Failed to create frame");
        return;
    }
    FI_HILOGD("alpha_:%{public}f, scale_:%{public}f", alpha_->Get(), scale_->Get());
    frame->SetDamageRegion(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    rsSurface->FlushFrame(frame);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawDynamicEffectModifier::SetAlpha(float alpha)
{
    CALL_DEBUG_ENTER;
    if (alpha_ == nullptr) {
        alpha_ = std::make_shared<OHOS::Rosen::RSAnimatableProperty<float>>(alpha);
        OHOS::Rosen::RSModifier::AttachProperty(alpha_);
    } else {
        alpha_->Set(alpha);
    }
}

void DrawDynamicEffectModifier::SetScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (scale_ == nullptr) {
        scale_ = std::make_shared<OHOS::Rosen::RSAnimatableProperty<float>>(scale);
        OHOS::Rosen::RSModifier::AttachProperty(scale_);
    } else {
        scale_->Set(scale);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS