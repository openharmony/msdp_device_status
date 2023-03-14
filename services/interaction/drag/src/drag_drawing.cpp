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

#include <cstdint>
#include <fstream>
#include <string>

#include "display_manager.h"
#include "include/core/SkTextBlob.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "string_ex.h"
#include "pointer_event.h"
#include "transaction/rs_interfaces.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_root_node.h"
#include "ui/rs_surface_extractor.h"
#include "ui/rs_surface_node.h"
#include "ui/rs_ui_director.h"
#include "../wm/window.h"

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
constexpr float SUCCESS_ENLARGE_SCALE = 1.2f;
constexpr float FAIL_ENLARGE_SCALE = 0.1f;
constexpr float PIVOT_X = 0.5f;
constexpr float PIVOT_Y = 0.5f;
const std::string COPY_DRAG_PATH = "/system/etc/device_status/drag_icon/Copy_Drag.svg";
const std::string COPY_ONE_DRAG_PATH = "/system/etc/device_status/drag_icon/Copy_One_Drag.svg";
const std::string FORBID_DRAG_PATH = "/system/etc/device_status/drag_icon/Forbid_Drag.svg";
const std::string MOUSE_DRAG_PATH = "/system/etc/device_status/drag_icon/Mouse_Drag.png";
struct DrawingInfo {
    int32_t sourceType { -1 };
    int32_t currentStyle { -1 };
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
    CHKPR(dragData.pictureResourse.pixelMap, RET_ERR);
    if ((dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN)) {
        FI_HILOGE("Invalid sourceType:%{public}d", dragData.sourceType);
        return RET_ERR;
    }
    g_drawingInfo.sourceType = dragData.sourceType;
    g_drawingInfo.displayId = dragData.displayId;
    g_drawingInfo.pixelMap = dragData.pictureResourse.pixelMap;
    g_drawingInfo.pixelMapX = dragData.pictureResourse.x;
    g_drawingInfo.pixelMapY = dragData.pictureResourse.y;
    CreateWindow(dragData.displayX, dragData.displayY);
    CHKPR(g_drawingInfo.dragWindow, RET_ERR);
    if (InitLayer() != RET_OK) {
        FI_HILOGE("Init Layer failed");
        return RET_ERR;
    }
    if (DrawShadowPic() != RET_OK) {
        FI_HILOGE("Draw shadow picture failed");
        return RET_ERR;
    }
    if (g_drawingInfo.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        g_drawingInfo.dragWindow->Show();
        InitAnimation();
        return RET_OK;
    }
    if (DrawMouseIcon() != RET_OK) {
        FI_HILOGE("Draw mouse icon failed");
        return RET_ERR;
    }
    g_drawingInfo.dragWindow->Show();
    InitAnimation();
    return RET_OK;
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
    g_drawingInfo.dragWindow->Show();
}

int32_t DragDrawing::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    CHKPR(rsUiDirector_, RET_ERR);
    if (style < 0) {
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if (g_drawingInfo.currentStyle == style) {
        FI_HILOGD("No need update drag style");
        return RET_OK;
    }
    g_drawingInfo.currentStyle = style;
    DrawStyle();
    rsUiDirector_->SendMessages();
    return RET_OK;
}

void DragDrawing::OnDragSuccess()
{
    CALL_DEBUG_ENTER;
    CHKPV(runner_);
    CHKPV(rsUiDirector_);
    CHKPV(g_drawingInfo.rootNode);
    if (drawDynamicEffectModifier_ != nullptr){
        g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
    }
    drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    g_drawingInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(1);
    drawDynamicEffectModifier_->SetScale(1);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(SUCCESS_ANIMATION_DURATION);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(0);
        drawDynamicEffectModifier_->SetScale(SUCCESS_ENLARGE_SCALE);
    });
    runner_->Run();
}

void DragDrawing::OnDragFail()
{
    CALL_DEBUG_ENTER;
    CHKPV(runner_);
    CHKPV(rsUiDirector_);
    CHKPV(g_drawingInfo.rootNode);
    if (drawDynamicEffectModifier_ != nullptr){
        g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
    }
    drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    g_drawingInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(1);
    drawDynamicEffectModifier_->SetScale(1);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(FAIL_ANIMATION_DURATION);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(0);
        drawDynamicEffectModifier_->SetScale(FAIL_ENLARGE_SCALE);
    });
    runner_->Run();
}

void DragDrawing::EraseMouseIcon()
{
    CALL_DEBUG_ENTER;
    CHKPV(g_drawingInfo.rootNode);
    CHKPV(rsUiDirector_);
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Canvas nodes vector size invalid, nodes.size(): %{public}d", g_drawingInfo.nodes.size());
        return;
    }
    if (g_drawingInfo.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGE("Touch type not need erase mouse icon");
        return;
    }
    g_drawingInfo.rootNode->RemoveChild(g_drawingInfo.nodes[MOUSE_ICON_INDEX]);
    rsUiDirector_->SendMessages();
}

void DragDrawing::DestroyDragWindow()
{
    CALL_DEBUG_ENTER;
    g_drawingInfo.sourceType = -1;
    startNum_ = START_TIME;
    g_drawingInfo.currentStyle = -1;
    if (g_drawingInfo.pixelMap != nullptr) {
        g_drawingInfo.pixelMap = nullptr;
    }
    if (!g_drawingInfo.nodes.empty()) {
        g_drawingInfo.nodes.clear();
    }
    if (g_drawingInfo.rootNode != nullptr) {
        g_drawingInfo.rootNode->ClearChildren();
        g_drawingInfo.rootNode = nullptr;
    }
    if (g_drawingInfo.surfaceNode != nullptr) {
        g_drawingInfo.surfaceNode = nullptr;
    }
    if (g_drawingInfo.dragWindow != nullptr) {
        g_drawingInfo.dragWindow->Destroy();
        g_drawingInfo.dragWindow = nullptr;
    }
    if (rsUiDirector_ != nullptr) {
        rsUiDirector_ = nullptr;
    }
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

int32_t DragDrawing::DrawShadowPic()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if (drawPixelMapModifier_ != nullptr) {
        g_drawingInfo.nodes[PIXEL_MAP_INDEX]->RemoveModifier(drawPixelMapModifier_);
    }
    drawPixelMapModifier_ = std::make_shared<DrawPixelMapModifier>();
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->AddModifier(drawPixelMapModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Nodes size invalid, node size: %{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if (drawMouseIconModifier_ != nullptr) {
        g_drawingInfo.nodes[MOUSE_ICON_INDEX]->RemoveModifier(drawMouseIconModifier_);
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->AddModifier(drawMouseIconModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawStyle()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if ((g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if (drawSVGModifier_ != nullptr) {
        g_drawingInfo.nodes[DRAG_STYLE_INDEX]->RemoveModifier(drawSVGModifier_);
    }
    auto drawSVGModifier_ = std::make_shared<DrawSVGModifier>();
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->AddModifier(drawSVGModifier_);
    return RET_OK;
}

int32_t DragDrawing::InitVSync()
{
    CALL_DEBUG_ENTER;
    CHKPR(handler_, RET_ERR);
    CHKPR(runner_, RET_ERR);
    CHKPR(g_drawingInfo.surfaceNode, RET_ERR);

    startNum_ = START_TIME;
    g_drawingInfo.surfaceNode->SetPivot(PIVOT_X, PIVOT_Y);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    auto rsClient = std::static_pointer_cast<OHOS::Rosen::RSRenderServiceClient>(
        OHOS::Rosen::RSIRenderClient::CreateRenderServiceClient());
    if (receiver_ == nullptr) {
        receiver_ = rsClient->CreateVSyncReceiver("DragDrawing", handler_);
    }
    auto ret = receiver_->Init();
    if (ret) {
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
    CHKPV(runner_);
    bool hasRunningAnimation = rsUiDirector_->RunningCustomAnimation(startNum_);
    if (!hasRunningAnimation) {
        FI_HILOGD("Stop runner_, hasRunningAnimation: %{public}d", hasRunningAnimation);
        runner_->Stop();
        return;
    }
    rsUiDirector_->SendMessages();
    startNum_ += INTERVAL_TIME;
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
    }
    g_drawingInfo.rootNode->SetBounds(g_drawingInfo.displayX, g_drawingInfo.displayY, width, height);
    g_drawingInfo.rootNode->SetFrame(g_drawingInfo.displayX, g_drawingInfo.displayY, width, height);
    g_drawingInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);

    auto pixelMapNode = OHOS::Rosen::RSCanvasNode::Create();
    pixelMapNode->SetBounds(0, SVG_HEIGHT, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    pixelMapNode->SetFrame(0, SVG_HEIGHT, g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.nodes.emplace_back(pixelMapNode);

    auto dragStyleNode = OHOS::Rosen::RSCanvasNode::Create();
    dragStyleNode->SetBounds(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    dragStyleNode->SetFrame(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    g_drawingInfo.nodes.emplace_back(dragStyleNode);

    if (g_drawingInfo.sourceType == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        auto mouseIconNode = OHOS::Rosen::RSCanvasNode::Create();
        mouseIconNode->SetBounds(0 - g_drawingInfo.pixelMapX, 0 - g_drawingInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        mouseIconNode->SetFrame(0 - g_drawingInfo.pixelMapX, 0 - g_drawingInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        g_drawingInfo.nodes.emplace_back(mouseIconNode);
        g_drawingInfo.rootNode->AddChild(pixelMapNode, -1);
        g_drawingInfo.rootNode->AddChild(dragStyleNode, -1);
        g_drawingInfo.rootNode->AddChild(mouseIconNode, -1);
        rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
        return;
    }
    g_drawingInfo.rootNode->AddChild(pixelMapNode, -1);
    g_drawingInfo.rootNode->AddChild(dragStyleNode, -1);
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
    std::string filePath = "";
    if (g_drawingInfo.currentStyle == 0) {
        filePath = FORBID_DRAG_PATH;
    } else if (g_drawingInfo.currentStyle == 1) {
        filePath = COPY_ONE_DRAG_PATH;
    } else {
        filePath = COPY_DRAG_PATH;
    }
    
    if (!IsValidSvgFile(filePath)) {
        FI_HILOGE("Svg file is invalid");
        return;
    }
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = DecodeSvgToPixelMap(filePath);
    CHKPV(pixelMap);
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t svgTouchPositionX = g_drawingInfo.pixelMap->GetWidth() + EIGHT_SIZE - pixelMap->GetWidth();
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBounds(0, EIGHT_SIZE,
        g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetFrame(0, EIGHT_SIZE,
        g_drawingInfo.pixelMap->GetWidth(), g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBounds(svgTouchPositionX, 0,
        pixelMap->GetWidth(), pixelMap->GetHeight());
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetFrame(svgTouchPositionX, 0,
        pixelMap->GetWidth(), pixelMap->GetHeight());
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBgImageWidth(pixelMap->GetWidth());
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBgImageHeight(SVG_HEIGHT);
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBgImagePositionX(0);
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBgImagePositionY(0);
    g_drawingInfo.nodes[DRAG_STYLE_INDEX]->SetBgImage(rosenImage);
    g_drawingInfo.rootNodeWidth = g_drawingInfo.pixelMap->GetWidth() + EIGHT_SIZE;
    g_drawingInfo.rootNodeHeight = pixelMap->GetHeight() + g_drawingInfo.pixelMap->GetHeight() + EIGHT_SIZE;
    g_drawingInfo.rootNode->SetBounds(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.rootNode->SetFrame(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.dragWindow->Resize(g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
}

int32_t DrawSVGModifier::UpdateSvgNodeInfo(xmlNodePtr &curNode, int32_t strSize) const
{
    CALL_DEBUG_ENTER;
    if (xmlStrcmp(curNode->name, BAD_CAST "svg")) {
        FI_HILOGE("Svg format invalid");
        return RET_ERR;
    }
    int32_t number = 0;
    std::ostringstream oStrSteam;
    oStrSteam << xmlGetProp(curNode, BAD_CAST "width");
    std::string srcSvgWidth = oStrSteam.str();
    number = std::stoi(srcSvgWidth) + strSize;
    std::string tgtSvgWidth  = std::to_string(number);
    xmlSetProp(curNode, BAD_CAST "width", BAD_CAST tgtSvgWidth.c_str());
    oStrSteam.clear();
    oStrSteam << xmlGetProp(curNode, BAD_CAST "viewBox");
    std::string srcViewBox = oStrSteam.str();
    std::istringstream iStrSteam(srcViewBox);
    std::string word;
    std::string tgtViewBox;
    int32_t i = 0;
    int32_t size = srcViewBox.size();
    while ((iStrSteam >> word) && (i < size)) {
        if (i == VIEW_BOX_POS) {
            number = std::stoi(word) + strSize;
            word = std::to_string(number);
        }
        tgtViewBox.append(word);
        tgtViewBox += " ";
        ++i;
    }

    xmlSetProp(curNode, BAD_CAST "viewBox", BAD_CAST tgtViewBox.c_str());
    return RET_OK;
}

xmlNodePtr DrawSVGModifier::FindRectNode(xmlNodePtr &curNode) const
{
    CALL_DEBUG_ENTER;
    curNode = curNode->xmlChildrenNode;
    while (curNode != NULL) {
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

xmlNodePtr DrawSVGModifier::UpdateRectNode(xmlNodePtr &curNode, int32_t strSize) const
{
    CALL_DEBUG_ENTER;
    int32_t number = 0;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "rect")) {
            std::ostringstream oStrSteam;
            oStrSteam << xmlGetProp(curNode, BAD_CAST "width");
            std::string srcRectWidth = oStrSteam.str();
            number = std::stoi(srcRectWidth) + strSize;
            std::string tgtRectWidth = std::to_string(number);
            xmlSetProp(curNode, BAD_CAST "width", BAD_CAST tgtRectWidth.c_str());
        }
        if (!xmlStrcmp(curNode->name, BAD_CAST "text")) {
            return curNode->xmlChildrenNode;
        }
        curNode = curNode->next;
    }
    return nullptr;
}

void DrawSVGModifier::UpdateTspanNode(xmlNodePtr &curNode) const
{
    CALL_DEBUG_ENTER;
    while (curNode != NULL) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "tspan")) {
            std::string tgtTspanValue = std::to_string(g_drawingInfo.currentStyle);
            xmlNodeSetContent(curNode, BAD_CAST tgtTspanValue.c_str());
        }
        curNode = curNode->next;
    }
}

int32_t DrawSVGModifier::ParseAndAdjustSvgInfo(xmlNodePtr &curNode) const
{
    CALL_DEBUG_ENTER;
    std::string strStyle = std::to_string(g_drawingInfo.currentStyle);
    int32_t strSize = (strStyle.size() - 1) * EIGHT_SIZE;
    xmlKeepBlanksDefault(0);
    int32_t ret = UpdateSvgNodeInfo(curNode, strSize);
    if (ret != RET_OK) {
        FI_HILOGE("Update svg node info failed");
        return RET_ERR;
    }
    curNode = FindRectNode(curNode);
    curNode = UpdateRectNode(curNode, strSize);
    UpdateTspanNode(curNode);
    return RET_OK;
}

std::shared_ptr<OHOS::Media::PixelMap> DrawSVGModifier::DecodeSvgToPixelMap(
    const std::string &filePath) const
{
    CALL_DEBUG_ENTER;
    xmlDocPtr xmlDoc = xmlReadFile(filePath.c_str(), 0, XML_PARSE_NOBLANKS);
    if (g_drawingInfo.currentStyle != 0) {
        xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
        int32_t ret = ParseAndAdjustSvgInfo(node);
        if (ret != RET_OK) {
            FI_HILOGE("Parse and adjust svg info failed");
            return nullptr;
        }
    }
    xmlChar *xmlbuff;
    int32_t buffersize;
    xmlDocDumpFormatMemory(xmlDoc, &xmlbuff, &buffersize, 1);
    std::ostringstream oStrSteam;
    oStrSteam << xmlbuff;
    std::string content = oStrSteam.str();
    xmlFree(xmlbuff);
    xmlFreeDoc(xmlDoc);
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/svg+xml";
    uint32_t ret = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(reinterpret_cast<const uint8_t*>(content.c_str()),
        content.size(), opts, ret);
    CHKPP(imageSource);
    OHOS::Media::DecodeOptions decodeOpts;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, ret);
    return pixelMap;
}

void DrawPixelMapModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    if (g_drawingInfo.pixelMap == nullptr) {
        FI_HILOGE("Param pixelMap is nullptr");
        return;
    }
    rosenImage->SetPixelMap(g_drawingInfo.pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t pixelMapWidth = g_drawingInfo.pixelMap->GetWidth();
    int32_t pixelMapHeight = g_drawingInfo.pixelMap->GetHeight();
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBoundsWidth(pixelMapWidth);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBoundsHeight(pixelMapHeight);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBgImageWidth(pixelMapWidth);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBgImageHeight(pixelMapHeight);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBgImagePositionX(0);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBgImagePositionY(0);
    g_drawingInfo.nodes[PIXEL_MAP_INDEX]->SetBgImage(rosenImage);
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
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBoundsWidth(decodeOpts.desiredSize.width);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBoundsHeight(decodeOpts.desiredSize.height);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBgImageWidth(decodeOpts.desiredSize.width);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBgImageHeight(decodeOpts.desiredSize.height);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBgImagePositionX(0);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBgImagePositionY(0);
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->SetBgImage(rosenImage);
}

int32_t DrawMouseIconModifier::GetIconSize() const
{
    CALL_DEBUG_ENTER;
    auto displayInfo = OHOS::Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    CHKPR(displayInfo, 0);
    return displayInfo->GetDpi() * DEVICE_INDEPENDENT_PIXELS / BASELINE_DENSITY;
}

void DrawDynamicEffectModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(scale_);
    auto rsSurface = OHOS::Rosen::RSSurfaceExtractor::ExtractRSSurface(g_drawingInfo.surfaceNode);
    CHKPV(rsSurface);
    g_drawingInfo.rootNode->SetAlpha(alpha_->Get());
    g_drawingInfo.surfaceNode->SetScale(scale_->Get(), scale_->Get());
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