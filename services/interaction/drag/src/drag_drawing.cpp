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
constexpr int32_t CANVAS_VECTOR_MIN_SIZE = 2;
constexpr int32_t PIXEL_MAP_INDEX = 0;
constexpr int32_t MOUSE_ICON_INDEX = 2;
const std::string COPY_DRAG_PATH = "/system/etc/device_status/mouse_icon/Copy_Drag.svg";
const std::string COPY_ONE_DRAG_PATH = "/system/etc/device_status/mouse_icon/Copy_One_Drag.svg";
const std::string FORBID_DRAG_PATH = "/system/etc/device_status/mouse_icon/Forbid_Drag.svg";
const std::string MOUSE_DRAG_PATH = "/system/etc/device_status/mouse_icon/Mouse_Drag.png";
const std::string SVG_PATH = "/system/etc/device_status/mouse_icon/";

struct DragDringInfo {
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
}g_dragDringInfo;
} // namespace

int32_t DragDrawing::InitPicture(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    CHKPR(dragData.pictureResourse.pixelMap, RET_ERR);
    if ((dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
      (dragData.sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN)) {
        FI_HILOGE("Invalid sourceType:%{public}d", dragData.sourceType);
        return RET_ERR;
    }
    if ((dragData.pictureResourse.x > 0) || (dragData.pictureResourse.y > 0)) {
        FI_HILOGE("Invalid pictureResourse, pictureResourse.x:%{public}d, pictureResourse.y:%{public}d",
            dragData.pictureResourse.x, dragData.pictureResourse.y);
        return RET_ERR;
    }
    sourceType_ = dragData.sourceType;
    g_dragDringInfo.displayId = dragData.displayId;
    g_dragDringInfo.pixelMap = dragData.pictureResourse.pixelMap;
    g_dragDringInfo.pixelMapX = dragData.pictureResourse.x;
    g_dragDringInfo.pixelMapY = dragData.pictureResourse.y;
    int32_t ret = InitPictureLayer(dragData.displayX, dragData.displayY);
    if (ret != RET_OK) {
        FI_HILOGE("Init Picture failed");
        return RET_ERR;
    }
    return RET_OK;
}

void DragDrawing::Draw(int32_t displayId, int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    if (displayId < 0) {
        FI_HILOGE("Invalid display:%{public}d", displayId);
        return;
    }
    g_dragDringInfo.displayId = displayId;
    g_dragDringInfo.displayX = displayX;
    g_dragDringInfo.displayY = displayY;
    if (displayX < 0) {
        g_dragDringInfo.displayX = 0;
    }
    if (displayY < 0) {
        g_dragDringInfo.displayY = 0;
    }

    if (g_dragDringInfo.dragWindow != nullptr) {
        g_dragDringInfo.dragWindow->MoveTo(g_dragDringInfo.displayX + g_dragDringInfo.pixelMapX,
            g_dragDringInfo.displayY + g_dragDringInfo.pixelMapY);
        g_dragDringInfo.dragWindow->Show();
        return;
    }
    CreateDragWindow(g_dragDringInfo.displayX + g_dragDringInfo.pixelMapX,
        g_dragDringInfo.displayY + g_dragDringInfo.pixelMapY);
    g_dragDringInfo.dragWindow->Show();
}

void DragDrawing::DrawPicture()
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.nodes.size() < CANVAS_VECTOR_MIN_SIZE) {
        FI_HILOGE("Canvas nodes vector size invalid, nodes.size(): %{public}d", g_dragDringInfo.nodes.size());
        return;
    }
    if (drawPixelMapModifier_ != nullptr) {
        g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->RemoveModifier(drawPixelMapModifier_);
    }
    drawPixelMapModifier_ = std::make_shared<DrawPixelMapModifier>();
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->AddModifier(drawPixelMapModifier_);
}

void DragDrawing::DrawMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.nodes.size() < CANVAS_VECTOR_MIN_SIZE) {
        FI_HILOGE("Canvas nodes vector size invalid, nodes.size(): %{public}d", g_dragDringInfo.nodes.size());
        return;
    }
    if (sourceType_ != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGD("Touch type not need draw mouse icon");
        return;
    }
    if (drawMouseIconModifier_ != nullptr) {
        g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->RemoveModifier(drawMouseIconModifier_);
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->AddModifier(drawMouseIconModifier_);
    return;
}

int32_t DragDrawing::InitLayer()
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.dragWindow == nullptr) {
        FI_HILOGE("Init layer failed, dragWindow is nullptr");
        return RET_ERR;
    }
    g_dragDringInfo.surfaceNode = g_dragDringInfo.dragWindow->GetSurfaceNode();
    if (g_dragDringInfo.surfaceNode == nullptr) {
        g_dragDringInfo.dragWindow->Destroy();
        g_dragDringInfo.dragWindow = nullptr;
        FI_HILOGE("Init layer failed, surfaceNode is nullptr");
        return RET_ERR;
    }
    auto surface = g_dragDringInfo.surfaceNode->GetSurface();
    if (surface == nullptr) {
        g_dragDringInfo.dragWindow->Destroy();
        g_dragDringInfo.dragWindow = nullptr;
        FI_HILOGE("Init layer failed, surface is nullptr");
        return RET_ERR;
    }
    rsUiDirector_ = OHOS::Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    rsUiDirector_->SetRSSurfaceNode(g_dragDringInfo.surfaceNode);
    InitCanvas(IMAGE_WIDTH, IMAGE_HEIGHT);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    return RET_OK;
}

void DragDrawing::InitCanvas(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.rootNode == nullptr) {
        g_dragDringInfo.rootNode = OHOS::Rosen::RSRootNode::Create();
    }
    g_dragDringInfo.rootNode->SetBounds(g_dragDringInfo.displayX, g_dragDringInfo.displayY, width, height);
    g_dragDringInfo.rootNode->SetFrame(g_dragDringInfo.displayX, g_dragDringInfo.displayY, width, height);
    g_dragDringInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);

    auto pixelMapNode = OHOS::Rosen::RSCanvasNode::Create();
    pixelMapNode->SetBounds(0, SVG_HEIGHT, g_dragDringInfo.pixelMap->GetWidth(), g_dragDringInfo.pixelMap->GetHeight());
    pixelMapNode->SetFrame(0, SVG_HEIGHT, g_dragDringInfo.pixelMap->GetWidth(), g_dragDringInfo.pixelMap->GetHeight());
    g_dragDringInfo.nodes.emplace_back(pixelMapNode);

    auto dragStyleNode = OHOS::Rosen::RSCanvasNode::Create();
    dragStyleNode->SetBounds(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    dragStyleNode->SetFrame(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    g_dragDringInfo.nodes.emplace_back(dragStyleNode);

    if (sourceType_ == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        auto mouseIconNode = OHOS::Rosen::RSCanvasNode::Create();
        mouseIconNode->SetBounds(0 - g_dragDringInfo.pixelMapX, 0 - g_dragDringInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        mouseIconNode->SetFrame(0 - g_dragDringInfo.pixelMapX, 0 - g_dragDringInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        g_dragDringInfo.nodes.emplace_back(mouseIconNode);
        g_dragDringInfo.rootNode->AddChild(pixelMapNode, -1);
        g_dragDringInfo.rootNode->AddChild(dragStyleNode, -1);
        g_dragDringInfo.rootNode->AddChild(mouseIconNode, -1);
        rsUiDirector_->SetRoot(g_dragDringInfo.rootNode->GetId());
        return;
    }
    g_dragDringInfo.rootNode->AddChild(pixelMapNode, -1);
    g_dragDringInfo.rootNode->AddChild(dragStyleNode, -1);
    rsUiDirector_->SetRoot(g_dragDringInfo.rootNode->GetId());
}

int32_t DragDrawing::InitPictureLayer(int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    CreateDragWindow(displayX, displayY);
    int32_t ret = InitLayer();
    if (ret != RET_OK) {
        FI_HILOGE("Init layer failed");
        return RET_ERR;
    }
    DrawPicture();
    DrawMouseIcon();
    return RET_OK;
}

void DragDrawing::CreateDragWindow(int32_t displayX, int32_t displayY)
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
    option->SetTouchable(true);
    std::string windowName = "drag window";
    g_dragDringInfo.dragWindow = OHOS::Rosen::Window::Create(windowName, option, nullptr);
}

void DrawPixelMapModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.nodes.size() < CANVAS_VECTOR_MIN_SIZE) {
        FI_HILOGE("Canvas nodes vector size invalid, nodes.size(): %{public}d", g_dragDringInfo.nodes.size());
        return;
    }
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    if (g_dragDringInfo.pixelMap == nullptr) {
        FI_HILOGE("DragDringInfo.pixelMap is nullptr");
        return;
    }
    rosenImage->SetPixelMap(g_dragDringInfo.pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t pixelMapWidth = g_dragDringInfo.pixelMap->GetWidth();
    int32_t pixelMapHeight = g_dragDringInfo.pixelMap->GetHeight();
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBoundsWidth(pixelMapWidth);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBoundsHeight(pixelMapHeight);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBgImageWidth(pixelMapWidth);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBgImageHeight(pixelMapHeight);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBgImagePositionX(0);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBgImagePositionY(0);
    g_dragDringInfo.nodes[PIXEL_MAP_INDEX]->SetBgImage(rosenImage);
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawMouseIconModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    if (g_dragDringInfo.nodes.size() < CANVAS_VECTOR_MIN_SIZE) {
        FI_HILOGE("Canvas nodes vector size invalid, nodes.size(): %{public}d", g_dragDringInfo.nodes.size());
        return;
    }
    std::string imagePath = MOUSE_DRAG_PATH;
    OHOS::Media::SourceOptions opts;
    opts.formatHint = "image/png";
    uint32_t errCode = 0;
    auto imageSource = OHOS::Media::ImageSource::CreateImageSource(imagePath, opts, errCode);
    CHKPV(imageSource);
    std::set<std::string> formats;
    imageSource->GetSupportedFormats(formats);
    auto displayInfo = OHOS::Rosen::DisplayManager::GetInstance().GetDisplayById(g_dragDringInfo.displayId);
    CHKPV(displayInfo);
    OHOS::Media::DecodeOptions decodeOpts;
    decodeOpts.desiredSize = {
        .width = displayInfo->GetDpi() * DEVICE_INDEPENDENT_PIXELS / BASELINE_DENSITY,
        .height = displayInfo->GetDpi() * DEVICE_INDEPENDENT_PIXELS / BASELINE_DENSITY
    };
    decodeOpts.allocatorType = OHOS::Media::AllocatorType::SHARE_MEM_ALLOC;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBoundsWidth(decodeOpts.desiredSize.width);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBoundsHeight(decodeOpts.desiredSize.height);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBgImageWidth(decodeOpts.desiredSize.width);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBgImageHeight(decodeOpts.desiredSize.height);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBgImagePositionX(0);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBgImagePositionY(0);
    g_dragDringInfo.nodes[MOUSE_ICON_INDEX]->SetBgImage(rosenImage);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS