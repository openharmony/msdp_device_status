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
#include <sys/stat.h>

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
#include "ui/rs_surface_extractor.h"
#include "ui/rs_surface_node.h"
#include "ui/rs_ui_director.h"
#include "ui/rs_root_node.h"
#include "../wm/window.h"

#include "devicestatus_define.h"
#include "drag_data_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawing" };
constexpr int32_t BASELINE_DENSITY = 160;
constexpr int32_t DEVICE_INDEPENDENT_PIXELS = 40;
constexpr int32_t IMAGE_WIDTH = 400;
constexpr int32_t IMAGE_HEIGHT = 500;
constexpr int32_t SVG_HEIGHT = 40;
constexpr int32_t TOUCH_NODE_MIN_COUNT = 2;
constexpr int32_t MOUSE_NODE_MIN_COUNT = 3;
constexpr int32_t PIXEL_MAP_INDEX = 0;
constexpr int32_t MOUSE_ICON_INDEX = 2;
const std::string MOUSE_DRAG_PATH = "/system/etc/device_status/mouse_icon/Mouse_Drag.png";

static struct DrawingInfo {
    int32_t sourceType_ { -1 };
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
}g_drawingInfo;
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
    g_drawingInfo.sourceType_ = dragData.sourceType;
    g_drawingInfo.displayId = dragData.displayId;
    g_drawingInfo.pixelMap = dragData.pictureResourse.pixelMap;
    g_drawingInfo.pixelMapX = dragData.pictureResourse.x;
    g_drawingInfo.pixelMapY = dragData.pictureResourse.y;
    CreateWindow(dragData.displayX, dragData.displayY);
    CHKPR(g_drawingInfo.dragWindow, RET_ERR);
    int32_t ret = InitLayer();
    if (ret != RET_OK) {
        FI_HILOGE("Init Layer failed");
        return RET_ERR;
    }
    ret = DrawShadowPic();
    if (ret != RET_OK) {
        FI_HILOGE("Draw shadow picture failed");
        return RET_ERR;
    }
    
    if (g_drawingInfo.sourceType_ != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        g_drawingInfo.dragWindow->Show();
        return RET_OK;
    }
    ret = DrawMouseIcon();
    if (ret != RET_OK) {
        FI_HILOGE("Draw mouse icon failed");
        return RET_ERR;
    }
    g_drawingInfo.dragWindow->Show();
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
    CreateWindow(g_drawingInfo.displayX + g_drawingInfo.pixelMapX,
        g_drawingInfo.displayY + g_drawingInfo.pixelMapY);
    g_drawingInfo.dragWindow->Show();
}

int32_t DragDrawing::DrawShadowPic()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType_ == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}d", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if ((g_drawingInfo.sourceType_ == OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size: %{public}d", g_drawingInfo.nodes.size());
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
        FI_HILOGE("Nodes size invalid, node size: %{public}d", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    if (drawMouseIconModifier_ != nullptr) {
        g_drawingInfo.nodes[MOUSE_ICON_INDEX]->RemoveModifier(drawMouseIconModifier_);
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    g_drawingInfo.nodes[MOUSE_ICON_INDEX]->AddModifier(drawMouseIconModifier_);
    return RET_OK;
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

    if (g_drawingInfo.sourceType_ == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
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
    std::set<std::string> formats;
    imageSource->GetSupportedFormats(formats);
    int32_t iconSize = GetIconSize();
    if (iconSize == RET_ERR) {
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
    CHKPR(displayInfo, RET_ERR);
    return displayInfo->GetDpi() * DEVICE_INDEPENDENT_PIXELS / BASELINE_DENSITY;
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS