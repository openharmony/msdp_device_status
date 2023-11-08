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

#include <dlfcn.h>

#include "cJSON.h"
#include "display_manager.h"
#include "include/core/SkTextBlob.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "input_manager.h"
#include "parameters.h"
#include "pointer_event.h"
#include "pointer_style.h"
#include "render/rs_filter.h"
#include "string_ex.h"
#include "transaction/rs_interfaces.h"
#include "ui/rs_surface_extractor.h"
#include "ui/rs_surface_node.h"
#include "ui/rs_ui_director.h"
#include "devicestatus_define.h"
#include "drag_data_manager.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawing" };
constexpr int32_t BASELINE_DENSITY { 160 };
constexpr int32_t DEVICE_INDEPENDENT_PIXEL { 40 };
constexpr int32_t DRAG_NUM_ONE { 1 };
constexpr int32_t STRING_PX_LENGTH { 2 };
constexpr int32_t EIGHT_SIZE { 8 };
constexpr int32_t TWELVE_SIZE { 12 };
constexpr int32_t IMAGE_WIDTH { 400 };
constexpr int32_t IMAGE_HEIGHT { 500 };
constexpr int64_t START_TIME { 181154000809 };
constexpr int64_t INTERVAL_TIME { 16666667 };
constexpr int32_t FRAMERATE { 30 };
constexpr int32_t SVG_WIDTH { 40 };
constexpr int32_t SVG_HEIGHT { 40 };
constexpr int32_t SIXTEEN { 16 };
constexpr int32_t SUCCESS_ANIMATION_DURATION { 300 };
constexpr int32_t VIEW_BOX_POS { 2 };
constexpr int32_t BACKGROUND_FILTER_INDEX { 0 };
constexpr int32_t PIXEL_MAP_INDEX { 1 };
constexpr int32_t DRAG_STYLE_INDEX { 2 };
constexpr int32_t MOUSE_ICON_INDEX { 3 };
constexpr size_t TOUCH_NODE_MIN_COUNT { 3 };
constexpr size_t MOUSE_NODE_MIN_COUNT { 4 };
constexpr double ONETHOUSAND { 1000.0 };
constexpr float DEFAULT_SCALING { 1.0f };
constexpr float BEGIN_ALPHA { 1.0f };
constexpr float END_ALPHA { 0.0f };
constexpr float BEGIN_SCALE { 1.0f };
constexpr float END_SCALE_FAIL { 1.2f };
constexpr float END_SCALE_SUCCESS { 0.1f };
constexpr float PIVOT_X { 0.5f };
constexpr float PIVOT_Y { 0.5f };
constexpr float SVG_ORIGINAL_SIZE { 40.0f };
constexpr float DEFAULT_POSITION_X { 0.0f };
constexpr float BLUR_SIGMA_SCALE { 0.57735f };
constexpr float RADIUS_VP { 23.0f };
constexpr float DEFAULT_SATURATION { 1.05f };
constexpr float DEFAULT_BRIGHTNESS { 1.05f };
constexpr float INCREASE_RATIO { 1.22f };
constexpr float DRAG_WINDOW_POSITION_Z { 6999.0f };
constexpr int32_t DEFAULT_MOUSE_SIZE { 1 };
constexpr int32_t DEFAULT_COLOR_VALUE { 0 };
constexpr int32_t INVALID_COLOR_VALUE { -1 };
constexpr int32_t GLOBAL_WINDOW_ID { -1 };
constexpr int32_t MOUSE_DRAG_CURSOR_CIRCLE_STYLE { 41 };
constexpr int32_t CURSOR_CIRCLE_MIDDLE { 2 };
const std::string DEVICE_TYPE_DEFAULT { "default" };
const std::string DEVICE_TYPE_PHONE { "phone" };
const std::string THREAD_NAME { "os_AnimationEventRunner" };
const std::string COPY_DRAG_PATH { "/system/etc/device_status/drag_icon/Copy_Drag.svg" };
const std::string COPY_ONE_DRAG_PATH { "/system/etc/device_status/drag_icon/Copy_One_Drag.svg" };
const std::string DEFAULT_DRAG_PATH { "/system/etc/device_status/drag_icon/Default_Drag.svg" };
const std::string FORBID_DRAG_PATH { "/system/etc/device_status/drag_icon/Forbid_Drag.svg" };
const std::string FORBID_ONE_DRAG_PATH { "/system/etc/device_status/drag_icon/Forbid_One_Drag.svg" };
const std::string MOUSE_DRAG_DEFAULT_PATH { "/system/etc/device_status/drag_icon/Mouse_Drag_Default.svg" };
const std::string MOUSE_DRAG_CURSOR_CIRCLE_PATH { "/system/etc/device_status/drag_icon/Mouse_Drag_Cursor_Circle.png" };
const std::string MOVE_DRAG_PATH { "/system/etc/device_status/drag_icon/Move_Drag.svg" };
#ifdef __aarch64__
const std::string DRAG_ANIMATION_EXTENSION_SO_PATH { "/system/lib64/drag_drop_ext/libdrag_drop_ext.z.so" };
#else
const std::string DRAG_ANIMATION_EXTENSION_SO_PATH { "/system/lib/drag_drop_ext/libdrag_drop_ext.z.so" };
#endif
const std::string BIG_FOLDER_LABEL { "scb_folder" };
struct DrawingInfo g_drawingInfo;

struct JsonParser {
    JsonParser() = default;
    ~JsonParser()
    {
        if (json != nullptr) {
            cJSON_Delete(json);
            json = nullptr;
        }
    }
    operator cJSON *()
    {
        return json;
    }
    cJSON *json = nullptr;
};

bool CheckNodesValid()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when mouse type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return false;
    }
    if ((g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) &&
        (g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid when touchscreen type, node size:%{public}zu", g_drawingInfo.nodes.size());
        return false;
    }
    return true;
}

float GetScaling()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.isExistScalingVallue) {
        FI_HILOGD("deviceDpi:%{public}f", g_drawingInfo.scalingValue);
        return g_drawingInfo.scalingValue;
    }
    sptr<Rosen::Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    if (display == nullptr) {
        FI_HILOGD("Get display info failed, display:%{public}d", g_drawingInfo.displayId);
        display = Rosen::DisplayManager::GetInstance().GetDisplayById(0);
        if (display == nullptr) {
            FI_HILOGE("Get display info failed, display is nullptr");
            return DEFAULT_SCALING;
        }
    }

    int32_t deviceDpi = display->GetDpi();
    FI_HILOGD("displayId:%{public}d, deviceDpi:%{public}d", g_drawingInfo.displayId, deviceDpi);
    if (deviceDpi < -std::numeric_limits<float>::epsilon()) {
        FI_HILOGE("Invalid deviceDpi:%{public}d", deviceDpi);
        return DEFAULT_SCALING;
    }
    g_drawingInfo.scalingValue = (1.0 * deviceDpi * DEVICE_INDEPENDENT_PIXEL / BASELINE_DENSITY) / SVG_ORIGINAL_SIZE;
    g_drawingInfo.isExistScalingVallue = true;
    return g_drawingInfo.scalingValue;
}
} // namespace

int32_t DragDrawing::Init(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    int32_t checkDragDataResult = CheckDragData(dragData);
    if (INIT_SUCCESS != checkDragDataResult) {
        return checkDragDataResult;
    }
    InitDrawingInfo(dragData);
    CreateWindow(dragData.displayX, dragData.displayY);
    CHKPR(g_drawingInfo.surfaceNode, INIT_FAIL);
    if (InitLayer() != RET_OK) {
        FI_HILOGE("Init layer failed");
        return INIT_FAIL;
    }
    DragAnimationData dragAnimationData;
    if (InitDragAnimationData(dragAnimationData) != RET_OK) {
        FI_HILOGE("Init drag animation data failed");
        return INIT_FAIL;
    }
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return INIT_FAIL;
    }
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPR(shadowNode, INIT_FAIL);
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPR(dragStyleNode, INIT_FAIL);
    dragExtHandle_ = dlopen(DRAG_ANIMATION_EXTENSION_SO_PATH.c_str(), RTLD_LAZY);
    if (dragExtHandle_ == nullptr) {
        FI_HILOGE("Failed to open drag extension library");
    }
    OnStartDrag(dragAnimationData, shadowNode, dragStyleNode);
    CHKPR(rsUiDirector_, INIT_FAIL);
    if (g_drawingInfo.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        rsUiDirector_->SendMessages();
        return INIT_SUCCESS;
    }
    if (DrawMouseIcon() != RET_OK) {
        FI_HILOGE("Draw mouse icon failed");
        return INIT_FAIL;
    }
    rsUiDirector_->SendMessages();
    return INIT_SUCCESS;
}

int32_t DragDrawing::CheckDragData(const DragData &dragData)
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.isRunning) {
        FI_HILOGE("Drag drawing is running, can not init again");
        return INIT_CANCEL;
    }
    CHKPR(dragData.shadowInfo.pixelMap, INIT_FAIL);
    if ((dragData.sourceType != MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
        (dragData.sourceType != MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN)) {
        FI_HILOGE("Invalid sourceType:%{public}d", dragData.sourceType);
        return INIT_FAIL;
    }
    if (dragData.dragNum < 0) {
        FI_HILOGE("Invalid dragNum:%{public}d", dragData.dragNum);
        return INIT_FAIL;
    }
    return INIT_SUCCESS;
}

void DragDrawing::Draw(int32_t displayId, int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    if (displayId < 0) {
        FI_HILOGE("Invalid displayId:%{public}d", displayId);
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
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    int32_t positionY = g_drawingInfo.displayY + g_drawingInfo.pixelMapY - adjustSize;
    int32_t positionX = g_drawingInfo.displayX + g_drawingInfo.pixelMapX;
    if (g_drawingInfo.surfaceNode != nullptr) {
        g_drawingInfo.surfaceNode->SetBounds(positionX, positionY,
            g_drawingInfo.surfaceNode->GetStagingProperties().GetBounds().z_,
            g_drawingInfo.surfaceNode->GetStagingProperties().GetBounds().w_);
        Rosen::RSTransaction::FlushImplicitTransaction();
        return;
    }
    CreateWindow(positionX, positionY);
    CHKPV(g_drawingInfo.surfaceNode);
}

int32_t DragDrawing::UpdateDragStyle(DragCursorStyle style)
{
    CALL_DEBUG_ENTER;
    FI_HILOGD("style:%{public}d", style);
    if ((style < DragCursorStyle::DEFAULT) || (style > DragCursorStyle::MOVE)) {
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if (g_drawingInfo.currentStyle == style) {
        FI_HILOGD("Not need update drag style");
        return RET_OK;
    }
    g_drawingInfo.currentStyle = style;
    std::string filePath;
    if (GetFilePath(filePath) != RET_OK) {
        FI_HILOGD("Get file path failed");
        return RET_ERR;
    }
    if (!IsValidSvgFile(filePath)) {
        FI_HILOGE("Svg file is invalid");
        return RET_ERR;
    }
    std::shared_ptr<Media::PixelMap> pixelMap = DecodeSvgToPixelMap(filePath);
    CHKPR(pixelMap, RET_ERR);
    bool isPreviousDefaultStyle = g_drawingInfo.isCurrentDefaultStyle;
    g_drawingInfo.isPreviousDefaultStyle = isPreviousDefaultStyle;
    g_drawingInfo.isCurrentDefaultStyle = (filePath == DEFAULT_DRAG_PATH);
    g_drawingInfo.stylePixelMap = pixelMap;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return RET_ERR;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPR(dragStyleNode, RET_ERR);
    OnDragStyle(dragStyleNode, pixelMap);
    CHKPR(rsUiDirector_, RET_ERR);
    rsUiDirector_->SendMessages();
    return RET_OK;
}

int32_t DragDrawing::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    CHKPR(shadowInfo.pixelMap, RET_ERR);
    g_drawingInfo.pixelMap = shadowInfo.pixelMap;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return RET_ERR;
    }
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPR(shadowNode, RET_ERR);
    DrawShadow(shadowNode);
    float scalingValue = GetScaling();
    if ((1.0 * INT32_MAX / (SVG_WIDTH + TWELVE_SIZE)) <= scalingValue) {
        FI_HILOGE("Invalid scalingValue:%{public}f", scalingValue);
        return RET_ERR;
    }
    int32_t adjustSize = (SVG_WIDTH + TWELVE_SIZE) * scalingValue;
    g_drawingInfo.rootNodeWidth = g_drawingInfo.pixelMap->GetWidth() + g_drawingInfo.mouseWidth + adjustSize;
    g_drawingInfo.rootNodeHeight = g_drawingInfo.pixelMap->GetHeight() + g_drawingInfo.mouseHeight + adjustSize;
    CHKPR(g_drawingInfo.rootNode, RET_ERR);
    g_drawingInfo.rootNode->SetBounds(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.rootNode->SetFrame(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    CHKPR(g_drawingInfo.surfaceNode, RET_ERR);
    g_drawingInfo.surfaceNode->SetBoundsWidth(g_drawingInfo.rootNodeWidth);
    g_drawingInfo.surfaceNode->SetBoundsHeight(g_drawingInfo.rootNodeHeight);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        g_drawingInfo.pixelMapX = shadowInfo.x;
        g_drawingInfo.pixelMapY = shadowInfo.y;
        DrawMouseIcon();
    }
    ProcessFilter();
    Draw(g_drawingInfo.displayId, g_drawingInfo.displayX + shadowInfo.x - g_drawingInfo.pixelMapX,
        g_drawingInfo.displayY + shadowInfo.y - g_drawingInfo.pixelMapY);
    Rosen::RSTransaction::FlushImplicitTransaction();
    CHKPR(rsUiDirector_, RET_ERR);
    rsUiDirector_->SendMessages();
    return RET_OK;
}

void DragDrawing::OnDragSuccess()
{
    CALL_INFO_TRACE;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPV(shadowNode);
    std::shared_ptr<Rosen::RSCanvasNode> styleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(styleNode);
    OnStopDragSuccess(shadowNode, styleNode);
}

void DragDrawing::OnDragFail()
{
    CALL_INFO_TRACE;
    std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode = g_drawingInfo.surfaceNode;
    CHKPV(surfaceNode);
    std::shared_ptr<Rosen::RSNode> rootNode = g_drawingInfo.rootNode;
    CHKPV(rootNode);
    OnStopDragFail(surfaceNode, rootNode);
}

void DragDrawing::EraseMouseIcon()
{
    CALL_INFO_TRACE;
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Nodes size invalid, node size:%{public}zu", g_drawingInfo.nodes.size());
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    if (drawMouseIconModifier_ != nullptr) {
        mouseIconNode->RemoveModifier(drawMouseIconModifier_);
        drawMouseIconModifier_ = nullptr;
    }
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->RemoveChild(mouseIconNode);
    CHKPV(rsUiDirector_);
    rsUiDirector_->SendMessages();
}

void DragDrawing::DestroyDragWindow()
{
    CALL_INFO_TRACE;
    startNum_ = START_TIME;
    g_drawingInfo.sourceType = -1;
    g_drawingInfo.currentDragNum = -1;
    g_drawingInfo.pixelMapX = -1;
    g_drawingInfo.pixelMapY = -1;
    g_drawingInfo.displayX = -1;
    g_drawingInfo.displayY = -1;
    g_drawingInfo.mouseWidth = 0;
    g_drawingInfo.mouseHeight = 0;
    g_drawingInfo.rootNodeWidth = -1;
    g_drawingInfo.rootNodeHeight = -1;
    g_drawingInfo.pixelMap = nullptr;
    g_drawingInfo.stylePixelMap = nullptr;
    g_drawingInfo.isPreviousDefaultStyle = false;
    g_drawingInfo.isCurrentDefaultStyle = false;
    g_drawingInfo.currentStyle = DragCursorStyle::DEFAULT;
    g_drawingInfo.filterInfo.clear();
    g_drawingInfo.extraInfo.clear();
    RemoveModifier();
    if (!g_drawingInfo.nodes.empty()) {
        g_drawingInfo.nodes.clear();
        g_drawingInfo.nodes.shrink_to_fit();
    }
    if (g_drawingInfo.rootNode != nullptr) {
        g_drawingInfo.rootNode->ClearChildren();
        g_drawingInfo.rootNode.reset();
        g_drawingInfo.rootNode = nullptr;
    }
    if (g_drawingInfo.surfaceNode != nullptr) {
        g_drawingInfo.surfaceNode->DetachToDisplay(g_drawingInfo.displayId);
        g_drawingInfo.surfaceNode = nullptr;
        g_drawingInfo.displayId = -1;
        Rosen::RSTransaction::FlushImplicitTransaction();
    }
    CHKPV(rsUiDirector_);
    rsUiDirector_->SetRoot(-1);
    rsUiDirector_->SendMessages();
}

void DragDrawing::UpdateDrawingState()
{
    CALL_DEBUG_ENTER;
    g_drawingInfo.isRunning = false;
}

void DragDrawing::UpdateDragWindowState(bool visible)
{
    CALL_DEBUG_ENTER;
    CHKPV(g_drawingInfo.surfaceNode);
    if (visible) {
        g_drawingInfo.surfaceNode->SetVisible(true);
        FI_HILOGI("Drag surfaceNode show success");
    } else {
        g_drawingInfo.surfaceNode->SetVisible(false);
        FI_HILOGI("Drag surfaceNode hide success");
    }
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DragDrawing::OnStartDrag(const DragAnimationData &dragAnimationData,
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode, std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode)
{
    CALL_DEBUG_ENTER;
    CHKPV(shadowNode);
    CHKPV(dragStyleNode);
    if (DrawShadow(shadowNode) != RET_OK) {
        FI_HILOGE("Draw shadow failed");
        return;
    }
    if (InitDrawStyle(dragStyleNode) != RET_OK) {
        FI_HILOGE("Init draw style failed");
        return;
    }
    if (dragExtHandle_ == nullptr) {
        FI_HILOGE("Failed to open drag extension library");
        return;
    }
    auto animationExtFunc = reinterpret_cast<DragExtFunc>(dlsym(dragExtHandle_, "OnStartDragExt"));
    if (animationExtFunc == nullptr) {
        FI_HILOGE("Failed to get drag extension func");
        dlclose(dragExtHandle_);
        dragExtHandle_ = nullptr;
        return;
    }
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(animationExtFunc, this, &g_drawingInfo))) {
        FI_HILOGE("Send animationExtFunc failed");
    }
}

void DragDrawing::OnDragStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode,
    std::shared_ptr<Media::PixelMap> stylePixelMap)
{
    CALL_DEBUG_ENTER;
    CHKPV(dragStyleNode);
    CHKPV(stylePixelMap);
    if (dragExtHandle_ == nullptr) {
        FI_HILOGE("Failed to open drag extension library");
        DrawStyle(dragStyleNode, stylePixelMap);
        return;
    }
    auto animationExtFunc = reinterpret_cast<DragExtFunc>(dlsym(dragExtHandle_, "OnDragStyleExt"));
    if (animationExtFunc == nullptr) {
        FI_HILOGE("Failed to get drag extension func");
        DrawStyle(dragStyleNode, stylePixelMap);
        dlclose(dragExtHandle_);
        dragExtHandle_ = nullptr;
        return;
    }
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (drawSVGModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawSVGModifier_);
        drawSVGModifier_ = nullptr;
    }
    if (!handler_->PostTask(std::bind(animationExtFunc, this, &g_drawingInfo))) {
        FI_HILOGE("Send animationExtFunc failed");
        DrawStyle(dragStyleNode, stylePixelMap);
    }
}

void DragDrawing::OnStopDragSuccess(std::shared_ptr<Rosen::RSCanvasNode> shadowNode,
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode)
{
    CALL_DEBUG_ENTER;
    if (dragExtHandle_ == nullptr) {
        FI_HILOGE("Failed to open drag extension library");
        RunAnimation(END_ALPHA, END_SCALE_SUCCESS);
        return;
    }
    auto animationExtFunc = reinterpret_cast<DragExtFunc>(dlsym(dragExtHandle_, "OnStopDragSuccessExt"));
    if (animationExtFunc == nullptr) {
        FI_HILOGE("Failed to get drag extension func");
        RunAnimation(END_ALPHA, END_SCALE_SUCCESS);
        dlclose(dragExtHandle_);
        dragExtHandle_ = nullptr;
        return;
    }
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(animationExtFunc, this, &g_drawingInfo))) {
        FI_HILOGE("Send animationExtFunc failed");
        RunAnimation(END_ALPHA, END_SCALE_SUCCESS);
    } else {
        StartVsync();
    }
}

void DragDrawing::OnStopDragFail(std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode,
    std::shared_ptr<Rosen::RSNode> rootNode)
{
    CALL_DEBUG_ENTER;
    if (dragExtHandle_ == nullptr) {
        FI_HILOGE("Failed to open drag extension library");
        RunAnimation(END_ALPHA, END_SCALE_FAIL);
        return;
    }
    auto animationExtFunc = reinterpret_cast<DragExtFunc>(dlsym(dragExtHandle_, "OnStopDragFailExt"));
    if (animationExtFunc == nullptr) {
        FI_HILOGE("Failed to get drag extension func");
        RunAnimation(END_ALPHA, END_SCALE_FAIL);
        dlclose(dragExtHandle_);
        dragExtHandle_ = nullptr;
        return;
    }
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(animationExtFunc, this, &g_drawingInfo))) {
        FI_HILOGE("Send animationExtFunc failed");
        RunAnimation(END_ALPHA, END_SCALE_FAIL);
    } else {
        StartVsync();
    }
}

void DragDrawing::OnStopAnimation()
{
    CALL_DEBUG_ENTER;
}

void DragDrawing::RunAnimation(float endAlpha, float endScale)
{
    CALL_DEBUG_ENTER;
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(&DragDrawing::InitVSync, this, endAlpha, endScale))) {
        FI_HILOGE("Send vsync event failed");
    }
}

int32_t DragDrawing::InitDrawStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode)
{
    CALL_DEBUG_ENTER;
    CHKPR(dragStyleNode, RET_ERR);
    std::string filePath;
    if (GetFilePath(filePath) != RET_OK) {
        FI_HILOGD("Get file path failed");
        return RET_ERR;
    }
    if (!IsValidSvgFile(filePath)) {
        FI_HILOGE("Svg file is invalid");
        return RET_ERR;
    }
    g_drawingInfo.isCurrentDefaultStyle = (filePath == DEFAULT_DRAG_PATH);
    std::shared_ptr<Media::PixelMap> pixelMap = DecodeSvgToPixelMap(filePath);
    CHKPR(pixelMap, RET_ERR);
    int32_t ret = DrawStyle(dragStyleNode, pixelMap);
    if (ret != RET_OK) {
        FI_HILOGE("Drag style failed");
    }
    return ret;
}

int32_t DragDrawing::DrawShadow(std::shared_ptr<Rosen::RSCanvasNode> shadowNode)
{
    CALL_DEBUG_ENTER;
    CHKPR(shadowNode, RET_ERR);
    if (drawPixelMapModifier_ != nullptr) {
        shadowNode->RemoveModifier(drawPixelMapModifier_);
        drawPixelMapModifier_ = nullptr;
    }
    drawPixelMapModifier_ = std::make_shared<DrawPixelMapModifier>();
    shadowNode->AddModifier(drawPixelMapModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.nodes.size() < MOUSE_NODE_MIN_COUNT) {
        FI_HILOGE("Nodes size invalid, node size:%{public}zu", g_drawingInfo.nodes.size());
        return RET_ERR;
    }
    std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPR(mouseIconNode, RET_ERR);
    if (drawMouseIconModifier_ != nullptr) {
        mouseIconNode->RemoveModifier(drawMouseIconModifier_);
        drawMouseIconModifier_ = nullptr;
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    mouseIconNode->AddModifier(drawMouseIconModifier_);
    return RET_OK;
}

int32_t DragDrawing::DrawStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode,
    std::shared_ptr<Media::PixelMap> stylePixelMap)
{
    CALL_DEBUG_ENTER;
    CHKPR(dragStyleNode, RET_ERR);
    CHKPR(stylePixelMap, RET_ERR);
    if (drawSVGModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawSVGModifier_);
        drawSVGModifier_ = nullptr;
    }
    drawSVGModifier_ = std::make_shared<DrawSVGModifier>(stylePixelMap);
    dragStyleNode->AddModifier(drawSVGModifier_);
    return RET_OK;
}

int32_t DragDrawing::InitVSync(float endAlpha, float endScale)
{
    CALL_DEBUG_ENTER;
    startNum_ = START_TIME;
    CHKPR(g_drawingInfo.rootNode, RET_ERR);
    if (drawDynamicEffectModifier_ != nullptr) {
        g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
        drawDynamicEffectModifier_ = nullptr;
    }
    drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    g_drawingInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(BEGIN_ALPHA);
    drawDynamicEffectModifier_->SetScale(BEGIN_SCALE);

    Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(SUCCESS_ANIMATION_DURATION);
    Rosen::RSNode::Animate(protocol, Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(endAlpha);
        drawDynamicEffectModifier_->SetScale(endScale);
    });
    return StartVsync();
}

int32_t DragDrawing::StartVsync()
{
    CHKPR(g_drawingInfo.surfaceNode, RET_ERR);
    g_drawingInfo.surfaceNode->SetPivot(PIVOT_X, PIVOT_Y);
    Rosen::RSTransaction::FlushImplicitTransaction();
    if (receiver_ == nullptr) {
        CHKPR(handler_, RET_ERR);
        receiver_ = Rosen::RSInterfaces::GetInstance().CreateVSyncReceiver("DragDrawing", handler_);
        CHKPR(receiver_, RET_ERR);
    }
    int32_t ret = receiver_->Init();
    if (ret != RET_OK) {
        FI_HILOGE("Receiver init failed");
        return RET_ERR;
    }
    Rosen::VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = std::bind(&DragDrawing::OnVsync, this)
    };
    int32_t changeFreq = static_cast<int32_t>((ONETHOUSAND / FRAMERATE) / SIXTEEN);
    ret = receiver_->SetVSyncRate(fcb, changeFreq);
    if (ret != RET_OK) {
        FI_HILOGE("Set vsync rate failed");
    }
    return ret;
}

void DragDrawing::OnVsync()
{
    CALL_DEBUG_ENTER;
    CHKPV(rsUiDirector_);
    bool hasRunningAnimation = rsUiDirector_->RunningCustomAnimation(startNum_);
    if (!hasRunningAnimation) {
        FI_HILOGD("Stop runner, hasRunningAnimation:%{public}d", hasRunningAnimation);
        CHKPV(handler_);
        handler_->RemoveAllEvents();
        handler_->RemoveAllFileDescriptorListeners();
        handler_ = nullptr;
        g_drawingInfo.isRunning = false;
        receiver_ = nullptr;
        CHKPV(g_drawingInfo.rootNode);
        if (drawDynamicEffectModifier_ != nullptr) {
            g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
            drawDynamicEffectModifier_ = nullptr;
        }
        DestroyDragWindow();
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
    g_drawingInfo.displayX = dragData.displayX;
    g_drawingInfo.displayY = dragData.displayY;
    g_drawingInfo.pixelMap = dragData.shadowInfo.pixelMap;
    g_drawingInfo.pixelMapX = dragData.shadowInfo.x;
    g_drawingInfo.pixelMapY = dragData.shadowInfo.y;
    g_drawingInfo.filterInfo = dragData.filterInfo;
    g_drawingInfo.extraInfo = dragData.extraInfo;
}

int32_t DragDrawing::InitDragAnimationData(DragAnimationData &dragAnimationData)
{
    CALL_DEBUG_ENTER;
    CHKPR(g_drawingInfo.pixelMap, RET_ERR);
    dragAnimationData.pixelMap = g_drawingInfo.pixelMap;
    dragAnimationData.displayX = g_drawingInfo.displayX;
    dragAnimationData.displayY = g_drawingInfo.displayY;
    dragAnimationData.offsetX = g_drawingInfo.pixelMapX;
    dragAnimationData.offsetY = g_drawingInfo.pixelMapY;
    return RET_OK;
}

int32_t DragDrawing::InitLayer()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.surfaceNode == nullptr) {
        FI_HILOGE("Init layer failed, surfaceNode is nullptr");
        return RET_ERR;
    }
    auto surface = g_drawingInfo.surfaceNode->GetSurface();
    if (surface == nullptr) {
        g_drawingInfo.surfaceNode->DetachToDisplay(g_drawingInfo.displayId);
        g_drawingInfo.surfaceNode = nullptr;
        FI_HILOGE("Init layer failed, surface is nullptr");
        Rosen::RSTransaction::FlushImplicitTransaction();
        return RET_ERR;
    }
    if (g_drawingInfo.isInitUiDirector) {
        g_drawingInfo.isInitUiDirector = false;
        rsUiDirector_ = Rosen::RSUIDirector::Create();
        CHKPR(rsUiDirector_, RET_ERR);
        rsUiDirector_->Init();
    }
    rsUiDirector_->SetRSSurfaceNode(g_drawingInfo.surfaceNode);
    InitCanvas(IMAGE_WIDTH, IMAGE_HEIGHT);
    Rosen::RSTransaction::FlushImplicitTransaction();
    return RET_OK;
}

void DragDrawing::InitCanvas(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.rootNode == nullptr) {
        g_drawingInfo.rootNode = Rosen::RSRootNode::Create();
        CHKPV(g_drawingInfo.rootNode);
    }
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    g_drawingInfo.rootNode->SetBounds(g_drawingInfo.displayX, g_drawingInfo.displayY - adjustSize, width, height);
    g_drawingInfo.rootNode->SetFrame(g_drawingInfo.displayX, g_drawingInfo.displayY - adjustSize, width, height);
    g_drawingInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);
    std::shared_ptr<Rosen::RSCanvasNode> filterNode = Rosen::RSCanvasNode::Create();
    CHKPV(filterNode);
    g_drawingInfo.nodes.emplace_back(filterNode);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        ProcessFilter();
    }
    std::shared_ptr<Rosen::RSCanvasNode> pixelMapNode = Rosen::RSCanvasNode::Create();
    CHKPV(pixelMapNode);
    CHKPV(g_drawingInfo.pixelMap);
    pixelMapNode->SetBounds(DEFAULT_POSITION_X, adjustSize, g_drawingInfo.pixelMap->GetWidth(),
        g_drawingInfo.pixelMap->GetHeight());
    pixelMapNode->SetFrame(DEFAULT_POSITION_X, adjustSize, g_drawingInfo.pixelMap->GetWidth(),
        g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.nodes.emplace_back(pixelMapNode);
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = Rosen::RSCanvasNode::Create();
    CHKPV(dragStyleNode);
    dragStyleNode->SetBounds(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    dragStyleNode->SetFrame(0, 0, SVG_HEIGHT, SVG_HEIGHT);
    g_drawingInfo.nodes.emplace_back(dragStyleNode);
    CHKPV(rsUiDirector_);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = Rosen::RSCanvasNode::Create();
        CHKPV(mouseIconNode);
        mouseIconNode->SetBounds(-g_drawingInfo.pixelMapX, -g_drawingInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        mouseIconNode->SetFrame(-g_drawingInfo.pixelMapX, -g_drawingInfo.pixelMapY, SVG_HEIGHT, SVG_HEIGHT);
        g_drawingInfo.nodes.emplace_back(mouseIconNode);
        g_drawingInfo.rootNode->AddChild(filterNode);
        g_drawingInfo.rootNode->AddChild(pixelMapNode);
        g_drawingInfo.rootNode->AddChild(dragStyleNode);
        g_drawingInfo.rootNode->AddChild(mouseIconNode);
        rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
        return;
    }
    g_drawingInfo.rootNode->AddChild(filterNode);
    g_drawingInfo.rootNode->AddChild(pixelMapNode);
    g_drawingInfo.rootNode->AddChild(dragStyleNode);
    rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
}

void DragDrawing::CreateWindow(int32_t displayX, int32_t displayY)
{
    CALL_DEBUG_ENTER;
    Rosen::RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "drag window";
    Rosen::RSSurfaceNodeType surfaceNodeType = Rosen::RSSurfaceNodeType::SELF_DRAWING_WINDOW_NODE;
    g_drawingInfo.surfaceNode = Rosen::RSSurfaceNode::Create(surfaceNodeConfig, surfaceNodeType);
    CHKPV(g_drawingInfo.surfaceNode);
    g_drawingInfo.surfaceNode->SetFrameGravity(Rosen::Gravity::RESIZE_ASPECT_FILL);
    g_drawingInfo.surfaceNode->SetPositionZ(DRAG_WINDOW_POSITION_Z);
    g_drawingInfo.surfaceNode->SetBounds(displayX, displayY, IMAGE_WIDTH, IMAGE_HEIGHT);
    g_drawingInfo.surfaceNode->SetBackgroundColor(SK_ColorTRANSPARENT);
    g_drawingInfo.surfaceNode->AttachToDisplay(g_drawingInfo.displayId);
    g_drawingInfo.surfaceNode->SetVisible(false);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DragDrawing::RemoveModifier()
{
    CALL_DEBUG_ENTER;
    if ((g_drawingInfo.nodes.size() < TOUCH_NODE_MIN_COUNT)) {
        FI_HILOGE("Nodes size invalid, node size:%{public}zu", g_drawingInfo.nodes.size());
        return;
    }

    std::shared_ptr<Rosen::RSCanvasNode> pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPV(pixelMapNode);
    if (drawPixelMapModifier_ != nullptr) {
        pixelMapNode->RemoveModifier(drawPixelMapModifier_);
        drawPixelMapModifier_ = nullptr;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    if (drawSVGModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawSVGModifier_);
        drawSVGModifier_ = nullptr;
    }
}

int32_t DragDrawing::UpdateSvgNodeInfo(xmlNodePtr curNode, int32_t extendSvgWidth)
{
    CALL_DEBUG_ENTER;
    if (xmlStrcmp(curNode->name, BAD_CAST "svg")) {
        FI_HILOGE("Svg format invalid");
        return RET_ERR;
    }
    std::ostringstream oStrStream;
    oStrStream << xmlGetProp(curNode, BAD_CAST "width");
    std::string srcSvgWidth = oStrStream.str();
    if (srcSvgWidth.length() < STRING_PX_LENGTH) {
        FI_HILOGE("Svg width invalid, srcSvgWidth:%{public}s", srcSvgWidth.c_str());
        return RET_ERR;
    }
    srcSvgWidth = srcSvgWidth.substr(0, srcSvgWidth.length() - STRING_PX_LENGTH);
    if (!IsNum(srcSvgWidth)) {
        FI_HILOGE("srcSvgWidth is not digital, srcSvgWidth:%{public}s", srcSvgWidth.c_str());
        return RET_ERR;
    }
    int32_t number = std::stoi(srcSvgWidth) + extendSvgWidth;
    std::string tgtSvgWidth = std::to_string(number);
    tgtSvgWidth.append("px");
    xmlSetProp(curNode, BAD_CAST "width", BAD_CAST tgtSvgWidth.c_str());
    oStrStream.str("");
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

xmlNodePtr DragDrawing::GetRectNode(xmlNodePtr curNode)
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

xmlNodePtr DragDrawing::UpdateRectNode(int32_t extendSvgWidth, xmlNodePtr curNode)
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
    FI_HILOGE("Empty node of XML");
    return nullptr;
}

void DragDrawing::UpdateTspanNode(xmlNodePtr curNode)
{
    CALL_DEBUG_ENTER;
    while (curNode != nullptr) {
        if (!xmlStrcmp(curNode->name, BAD_CAST "tspan")) {
            xmlNodeSetContent(curNode, BAD_CAST std::to_string(g_drawingInfo.currentDragNum).c_str());
        }
        curNode = curNode->next;
    }
}

int32_t DragDrawing::ParseAndAdjustSvgInfo(xmlNodePtr curNode)
{
    CALL_DEBUG_ENTER;
    CHKPR(curNode, RET_ERR);
    std::string strStyle = std::to_string(g_drawingInfo.currentDragNum);
    if (strStyle.empty()) {
        FI_HILOGE("strStyle size:%{public}zu invalid", strStyle.size());
        return RET_ERR;
    }
    int32_t extendSvgWidth = (static_cast<int32_t>(strStyle.size()) - 1) * EIGHT_SIZE;
    xmlKeepBlanksDefault(0);
    int32_t ret = UpdateSvgNodeInfo(curNode, extendSvgWidth);
    if (ret != RET_OK) {
        FI_HILOGE("Update svg node info failed, ret:%{public}d", ret);
        return RET_ERR;
    }
    curNode = GetRectNode(curNode);
    CHKPR(curNode, RET_ERR);
    curNode = UpdateRectNode(extendSvgWidth, curNode);
    CHKPR(curNode, RET_ERR);
    UpdateTspanNode(curNode);
    return RET_OK;
}

std::shared_ptr<Media::PixelMap> DragDrawing::DecodeSvgToPixelMap(
    const std::string &filePath)
{
    CALL_DEBUG_ENTER;
    xmlDocPtr xmlDoc = xmlReadFile(filePath.c_str(), 0, XML_PARSE_NOBLANKS);
    if (NeedAdjustSvgInfo()) {
        xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
        CHKPP(node);
        int32_t ret = ParseAndAdjustSvgInfo(node);
        if (ret != RET_OK) {
            FI_HILOGE("Parse and adjust svg info failed, ret:%{public}d", ret);
            return nullptr;
        }
    }
    xmlChar *xmlbuff = nullptr;
    int32_t buffersize = 0;
    xmlDocDumpFormatMemory(xmlDoc, &xmlbuff, &buffersize, 1);
    std::ostringstream oStrStream;
    oStrStream << xmlbuff;
    std::string content = oStrStream.str();
    xmlFree(xmlbuff);
    xmlFreeDoc(xmlDoc);
    Media::SourceOptions opts;
    opts.formatHint = "image/svg+xml";
    uint32_t errCode = 0;
    auto imageSource = Media::ImageSource::CreateImageSource(reinterpret_cast<const uint8_t*>(content.c_str()),
        content.size(), opts, errCode);
    CHKPP(imageSource);
    Media::DecodeOptions decodeOpts;
    SetDecodeOptions(decodeOpts);
    std::shared_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    return pixelMap;
}

bool DragDrawing::NeedAdjustSvgInfo()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.currentStyle == DragCursorStyle::DEFAULT) {
        return false;
    }
    if ((g_drawingInfo.currentStyle == DragCursorStyle::COPY) &&
        (g_drawingInfo.currentDragNum == DRAG_NUM_ONE)) {
        return false;
    }
    if ((g_drawingInfo.currentStyle == DragCursorStyle::MOVE) &&
        (g_drawingInfo.currentDragNum == DRAG_NUM_ONE)) {
        return false;
    }
    if ((g_drawingInfo.currentStyle == DragCursorStyle::FORBIDDEN) &&
        (g_drawingInfo.currentDragNum == DRAG_NUM_ONE)) {
        return false;
    }
    return true;
}

int32_t DragDrawing::GetFilePath(std::string &filePath)
{
    CALL_DEBUG_ENTER;
    switch (g_drawingInfo.currentStyle) {
        case DragCursorStyle::COPY: {
            if (g_drawingInfo.currentDragNum == DRAG_NUM_ONE) {
                filePath = COPY_ONE_DRAG_PATH;
            } else {
                filePath = COPY_DRAG_PATH;
            }
            break;
        }
        case DragCursorStyle::MOVE: {
            if (g_drawingInfo.currentDragNum == DRAG_NUM_ONE) {
                FI_HILOGD("Not need draw svg style, current drag number is one");
                filePath = DEFAULT_DRAG_PATH;
            } else {
                filePath = MOVE_DRAG_PATH;
            }
            break;
        }
        case DragCursorStyle::FORBIDDEN: {
            if (g_drawingInfo.currentDragNum == DRAG_NUM_ONE) {
                filePath = FORBID_ONE_DRAG_PATH;
            } else {
                filePath = FORBID_DRAG_PATH;
            }
            break;
        }
        case DragCursorStyle::DEFAULT:
        default: {
            FI_HILOGW("Not need draw svg style, DragCursorStyle:%{public}d", g_drawingInfo.currentStyle);
            filePath = DEFAULT_DRAG_PATH;
            break;
        }
    }
    return RET_OK;
}

void DragDrawing::SetDecodeOptions(Media::DecodeOptions &decodeOpts)
{
    CALL_DEBUG_ENTER;
    std::string strStyle = std::to_string(g_drawingInfo.currentDragNum);
    if (strStyle.empty()) {
        FI_HILOGE("strStyle size:%{public}zu invalid", strStyle.size());
        return;
    }
    int32_t extendSvgWidth = (static_cast<int32_t>(strStyle.size()) - 1) * EIGHT_SIZE;
    std::string deviceType = system::GetDeviceType();
    if ((g_drawingInfo.currentStyle == DragCursorStyle::COPY) && (g_drawingInfo.currentDragNum == DRAG_NUM_ONE)) {
        decodeOpts.desiredSize = {
            .width = DEVICE_INDEPENDENT_PIXEL * GetScaling(),
            .height = DEVICE_INDEPENDENT_PIXEL * GetScaling()
        };
    } else if (((deviceType.compare(0, DEVICE_TYPE_DEFAULT.size(), DEVICE_TYPE_DEFAULT) == 0) ||
        (deviceType.compare(0, DEVICE_TYPE_PHONE.size(), DEVICE_TYPE_PHONE) == 0)) &&
        (g_drawingInfo.currentStyle == DragCursorStyle::MOVE) && (g_drawingInfo.currentDragNum == DRAG_NUM_ONE)) {
        decodeOpts.desiredSize = {
            .width = DEVICE_INDEPENDENT_PIXEL * GetScaling(),
            .height = DEVICE_INDEPENDENT_PIXEL * GetScaling()
        };
    } else {
        decodeOpts.desiredSize = {
            .width = (DEVICE_INDEPENDENT_PIXEL + extendSvgWidth) * GetScaling(),
            .height = DEVICE_INDEPENDENT_PIXEL * GetScaling()
        };
    }
}

bool DragDrawing::ParserFilterInfo(FilterInfo& filterInfo)
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.filterInfo.empty()) {
        FI_HILOGD("FilterInfo is empty");
        return false;
    }
    JsonParser filterParser;
    filterParser.json = cJSON_Parse(g_drawingInfo.filterInfo.c_str());
    FI_HILOGD("FilterInfo size:%{public}zu, filterInfo:%{public}s",
        g_drawingInfo.filterInfo.size(), g_drawingInfo.filterInfo.c_str());
    if (!cJSON_IsObject(filterParser.json)) {
        FI_HILOGE("FilterInfo is not json object");
        return false;
    }
    cJSON *componentType = cJSON_GetObjectItemCaseSensitive(filterParser.json, "drag_data_type");
    if (!cJSON_IsString(componentType)) {
        FI_HILOGE("Parser componentType failed");
        return false;
    }
    cJSON *blurStyle = cJSON_GetObjectItemCaseSensitive(filterParser.json, "drag_blur_style");
    if (!cJSON_IsNumber(blurStyle)) {
        FI_HILOGE("Parser blurStyle failed");
        return false;
    }
    cJSON *cornerRadius = cJSON_GetObjectItemCaseSensitive(filterParser.json, "drag_corner_radius");
    if (!cJSON_IsNumber(cornerRadius)) {
        FI_HILOGE("Parser cornerRadius failed");
        return false;
    }
    if (g_drawingInfo.extraInfo.empty()) {
        FI_HILOGD("ExtraInfo is empty");
        return false;
    }
    JsonParser extraInfoParser;
    extraInfoParser.json = cJSON_Parse(g_drawingInfo.extraInfo.c_str());
    FI_HILOGD("ExtraInfo size:%{public}zu, extraInfo:%{public}s",
        g_drawingInfo.extraInfo.size(), g_drawingInfo.extraInfo.c_str());
    if (!cJSON_IsObject(extraInfoParser.json)) {
        FI_HILOGE("ExtraInfo is not json object");
        return false;
    }
    cJSON *dipScale = cJSON_GetObjectItemCaseSensitive(extraInfoParser.json, "dip_scale");
    if (!cJSON_IsNumber(dipScale)) {
        FI_HILOGE("Parser dipScale failed");
        return false;
    }
    filterInfo = { componentType->valuestring, blurStyle->valueint, cornerRadius->valueint, dipScale->valuedouble };
    return true;
}

void DragDrawing::ProcessFilter()
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<Rosen::RSCanvasNode> filterNode = g_drawingInfo.nodes[BACKGROUND_FILTER_INDEX];
    CHKPV(filterNode);
    CHKPV(g_drawingInfo.pixelMap);
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    if (FilterInfo filterInfo; ParserFilterInfo(filterInfo) && filterInfo.componentType == BIG_FOLDER_LABEL) {
        std::shared_ptr<Rosen::RSFilter> backFilter = Rosen::RSFilter::CreateMaterialFilter(
            RadiusVp2Sigma(RADIUS_VP, filterInfo.dipScale),
            DEFAULT_SATURATION, DEFAULT_BRIGHTNESS, DEFAULT_COLOR_VALUE);
        if (backFilter == nullptr) {
            FI_HILOGE("Create backgroundFilter failed");
            return;
        }
        filterNode->SetBackgroundFilter(backFilter);
        filterNode->SetBounds(DEFAULT_POSITION_X, adjustSize, g_drawingInfo.pixelMap->GetWidth(),
            g_drawingInfo.pixelMap->GetHeight());
        filterNode->SetFrame(DEFAULT_POSITION_X, adjustSize, g_drawingInfo.pixelMap->GetWidth(),
            g_drawingInfo.pixelMap->GetHeight());
        filterNode->SetCornerRadius(filterInfo.cornerRadius * filterInfo.dipScale);
        FI_HILOGD("Add filter successfully");
    }
}

int32_t DragDrawing::EnterTextEditorArea(bool enable)
{
    CALL_DEBUG_ENTER;
    if(enable){
        return RET_OK;
    }
    return RET_ERR;
}

float DragDrawing::RadiusVp2Sigma(float radiusVp, float dipScale)
{
    float radiusPx = radiusVp * dipScale;
    return radiusPx > 0.0f ? BLUR_SIGMA_SCALE * radiusPx + 0.5f : 0.0f;
}

DragDrawing::~DragDrawing()
{
    if (dragExtHandle_ != nullptr) {
        dlclose(dragExtHandle_);
        dragExtHandle_ = nullptr;
    }
}

void DrawSVGModifier::Draw(Rosen::RSDrawingContext& context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(stylePixelMap_);
    CHKPV(g_drawingInfo.pixelMap);
    float scalingValue = GetScaling();
    if ((1.0 * INT_MAX / (SVG_WIDTH + EIGHT_SIZE)) <= scalingValue) {
        FI_HILOGE("Invalid scalingValue:%{public}f", scalingValue);
        return;
    }
    int32_t adjustSize = EIGHT_SIZE * scalingValue;
    int32_t svgTouchPositionX = 0;
    if ((g_drawingInfo.pixelMap->GetWidth() + adjustSize) > stylePixelMap_->GetWidth()) {
        svgTouchPositionX = g_drawingInfo.pixelMap->GetWidth() + adjustSize - stylePixelMap_->GetWidth();
    }
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    dragStyleNode->SetBounds(svgTouchPositionX, (TWELVE_SIZE - EIGHT_SIZE) * scalingValue, stylePixelMap_->GetWidth(),
        stylePixelMap_->GetHeight());
    dragStyleNode->SetFrame(svgTouchPositionX, (TWELVE_SIZE - EIGHT_SIZE) * scalingValue, stylePixelMap_->GetWidth(),
        stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImageWidth(stylePixelMap_->GetWidth());
    dragStyleNode->SetBgImageHeight(stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImagePositionX(0);
    dragStyleNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(stylePixelMap_);
    rosenImage->SetImageRepeat(0);
    dragStyleNode->SetBgImage(rosenImage);
    adjustSize = (SVG_WIDTH + TWELVE_SIZE) * scalingValue;
    g_drawingInfo.rootNodeHeight = g_drawingInfo.pixelMap->GetHeight() + g_drawingInfo.mouseHeight + adjustSize;
    g_drawingInfo.rootNodeWidth = g_drawingInfo.pixelMap->GetWidth() + g_drawingInfo.mouseWidth + adjustSize;
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->SetBounds(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.rootNode->SetFrame(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    CHKPV(g_drawingInfo.surfaceNode);
    g_drawingInfo.surfaceNode->SetBoundsWidth(g_drawingInfo.rootNodeWidth);
    g_drawingInfo.surfaceNode->SetBoundsHeight(g_drawingInfo.rootNodeHeight);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawPixelMapModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(g_drawingInfo.pixelMap);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(g_drawingInfo.pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t pixelMapWidth = g_drawingInfo.pixelMap->GetWidth();
    int32_t pixelMapHeight = g_drawingInfo.pixelMap->GetHeight();
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPV(pixelMapNode);
    pixelMapNode->SetBoundsWidth(pixelMapWidth);
    pixelMapNode->SetBoundsHeight(pixelMapHeight);
    pixelMapNode->SetBgImageWidth(pixelMapWidth);
    pixelMapNode->SetBgImageHeight(pixelMapHeight);
    pixelMapNode->SetBgImagePositionX(0);
    pixelMapNode->SetBgImagePositionY(0);
    pixelMapNode->SetBgImage(rosenImage);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawMouseIconModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    std::string imagePath;
    MMI::PointerStyle pointerStyle;
    int32_t ret = MMI::InputManager::GetInstance()->GetPointerStyle(GLOBAL_WINDOW_ID, pointerStyle);
    if (ret != RET_OK) {
        FI_HILOGE("Get pointer style failed, ret:%{public}d", ret);
        return;
    }
    int32_t pointerStyleId = pointerStyle.id;
    if (pointerStyleId == MOUSE_DRAG_CURSOR_CIRCLE_STYLE) {
        imagePath = MOUSE_DRAG_CURSOR_CIRCLE_PATH;
    } else {
        imagePath = MOUSE_DRAG_DEFAULT_PATH;
    }
    Media::SourceOptions opts;
    opts.formatHint = "image/svg+xml";
    uint32_t errCode = 0;
    auto imageSource = Media::ImageSource::CreateImageSource(imagePath, opts, errCode);
    CHKPV(imageSource);
    int32_t pointerSize = pointerStyle.size;
    if (pointerSize < DEFAULT_MOUSE_SIZE) {
        FI_HILOGD("Invalid pointerSize:%{public}d", pointerSize);
        pointerSize = DEFAULT_MOUSE_SIZE;
    }
    Media::DecodeOptions decodeOpts;
    decodeOpts.desiredSize = {
        .width = pow(INCREASE_RATIO, pointerSize - 1) * DEVICE_INDEPENDENT_PIXEL * GetScaling(),
        .height = pow(INCREASE_RATIO, pointerSize - 1) * DEVICE_INDEPENDENT_PIXEL * GetScaling()
    };
    int32_t pointerColor = pointerStyle.color;
    if (pointerColor != INVALID_COLOR_VALUE) {
        decodeOpts.SVGOpts.fillColor = {.isValidColor = true, .color = pointerColor};
    }
    std::shared_ptr<Media::PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errCode);
    CHKPV(pixelMap);
    OnDraw(pixelMap, pointerStyleId);
}

void DrawMouseIconModifier::OnDraw(std::shared_ptr<Media::PixelMap> pixelMap, int32_t pointerStyleId) const
{
    CALL_DEBUG_ENTER;
    CHKPV(pixelMap);
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    if (pointerStyleId == MOUSE_DRAG_CURSOR_CIRCLE_STYLE) {
        int32_t positionX = -g_drawingInfo.pixelMapX - (pixelMap->GetWidth() / CURSOR_CIRCLE_MIDDLE);
        int32_t positionY = -g_drawingInfo.pixelMapY + adjustSize - (pixelMap->GetHeight() / CURSOR_CIRCLE_MIDDLE);
        mouseIconNode->SetBounds(positionX, positionY, pixelMap->GetWidth(), pixelMap->GetHeight());
        mouseIconNode->SetFrame(positionX, positionY, pixelMap->GetWidth(), pixelMap->GetHeight());
    } else {
        mouseIconNode->SetBounds(-g_drawingInfo.pixelMapX, -g_drawingInfo.pixelMapY + adjustSize,
            pixelMap->GetWidth(), pixelMap->GetHeight());
        mouseIconNode->SetFrame(-g_drawingInfo.pixelMapX, -g_drawingInfo.pixelMapY + adjustSize,
            pixelMap->GetWidth(), pixelMap->GetHeight());
    }
    mouseIconNode->SetBgImageWidth(pixelMap->GetWidth());
    mouseIconNode->SetBgImageHeight(pixelMap->GetHeight());
    mouseIconNode->SetBgImagePositionX(0);
    mouseIconNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    mouseIconNode->SetBgImage(rosenImage);
    g_drawingInfo.mouseWidth = pixelMap->GetWidth();
    g_drawingInfo.mouseHeight = pixelMap->GetHeight();
    adjustSize = (SVG_WIDTH + TWELVE_SIZE) * GetScaling();
    g_drawingInfo.rootNodeWidth = g_drawingInfo.pixelMap->GetWidth() + pixelMap->GetWidth() + adjustSize;
    g_drawingInfo.rootNodeHeight = g_drawingInfo.pixelMap->GetHeight() + pixelMap->GetHeight() + adjustSize;
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->SetBounds(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    g_drawingInfo.rootNode->SetFrame(0, 0, g_drawingInfo.rootNodeWidth, g_drawingInfo.rootNodeHeight);
    CHKPV(g_drawingInfo.surfaceNode);
    g_drawingInfo.surfaceNode->SetBoundsWidth(g_drawingInfo.rootNodeWidth);
    g_drawingInfo.surfaceNode->SetBoundsHeight(g_drawingInfo.rootNodeHeight);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawDynamicEffectModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(g_drawingInfo.rootNode);
    g_drawingInfo.rootNode->SetAlpha(alpha_->Get());
    CHKPV(scale_);
    CHKPV(g_drawingInfo.surfaceNode);
    g_drawingInfo.surfaceNode->SetScale(scale_->Get(), scale_->Get());
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawDynamicEffectModifier::SetAlpha(float alpha)
{
    CALL_DEBUG_ENTER;
    if (alpha_ == nullptr) {
        alpha_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(alpha);
        Rosen::RSModifier::AttachProperty(alpha_);
        return;
    }
    alpha_->Set(alpha);
}

void DrawDynamicEffectModifier::SetScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (scale_ == nullptr) {
        scale_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(scale);
        Rosen::RSModifier::AttachProperty(scale_);
        return;
    }
    scale_->Set(scale);
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS