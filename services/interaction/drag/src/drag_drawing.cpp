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
#include <limits>
#include <string>

#include <dlfcn.h>

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

#include "animation_curve.h"
#include "devicestatus_define.h"
#include "drag_data_manager.h"
#include "drag_hisysevent.h"
#include "json_parser.h"
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
constexpr int64_t START_TIME { 181154000809 };
constexpr int64_t INTERVAL_TIME { 16666667 };
constexpr int32_t SVG_WIDTH { 40 };
constexpr float SCALE_THRESHOLD_EIGHT { 1.0F * INT32_MAX / (SVG_WIDTH + EIGHT_SIZE) };
constexpr float SCALE_THRESHOLD_TWELVE { 1.0F * INT32_MAX / (SVG_WIDTH + TWELVE_SIZE) };
constexpr int32_t SUCCESS_ANIMATION_DURATION { 300 };
constexpr int32_t VIEW_BOX_POS { 2 };
constexpr int32_t BACKGROUND_FILTER_INDEX { 0 };
constexpr int32_t PIXEL_MAP_INDEX { 1 };
constexpr int32_t DRAG_STYLE_INDEX { 2 };
constexpr int32_t MOUSE_ICON_INDEX { 3 };
constexpr int32_t SHORT_DURATION { 55 };
constexpr int32_t LONG_DURATION { 90 };
constexpr int32_t FIRST_PIXELMAP_INDEX { 0 };
constexpr int32_t SECOND_PIXELMAP_INDEX { 1 };
constexpr size_t TOUCH_NODE_MIN_COUNT { 3 };
constexpr size_t MOUSE_NODE_MIN_COUNT { 4 };
constexpr float DEFAULT_SCALING { 1.0f };
constexpr float BEGIN_ALPHA { 1.0f };
constexpr float END_ALPHA { 0.0f };
constexpr float START_STYLE_ALPHA { 1.0f };
constexpr float END_STYLE_ALPHA { 0.0f };
constexpr float BEGIN_SCALE { 1.0f };
constexpr float END_SCALE_FAIL { 1.2f };
constexpr float END_SCALE_SUCCESS { 0.0f };
constexpr float START_STYLE_SCALE { 1.0f };
constexpr float STYLE_CHANGE_SCALE { 1.1f };
constexpr float STYLE_MAX_SCALE { 1.2f };
constexpr float STYLE_END_SCALE { 1.0f };
constexpr float DEFAULT_PIVOT { 0.0f };
constexpr float SVG_ORIGINAL_SIZE { 40.0f };
constexpr float DEFAULT_POSITION_X { 0.0f };
constexpr float BLUR_SIGMA_SCALE { 0.57735f };
constexpr float RADIUS_VP { 23.0f };
constexpr float DEFAULT_SATURATION { 1.05f };
constexpr float DEFAULT_BRIGHTNESS { 1.05f };
constexpr float INCREASE_RATIO { 1.22f };
constexpr float DRAG_WINDOW_POSITION_Z { 6999.0f };
constexpr float DEFAULT_ANGLE { 0.0f };
constexpr float POSITIVE_ANGLE { 8.0f };
constexpr float NEGATIVE_ANGLE { -8.0f };
constexpr float DEFAULT_ALPHA { 1.0f };
constexpr float FIRST_PIXELMAP_ALPHA { 0.6f };
constexpr float SECOND_PIXELMAP_ALPHA { 0.3f };
constexpr float HALF_RATIO { 0.5f };
constexpr float ROTATION_DEFAULT { 0.0f };
constexpr float ROTATION_FIRST_ORDER { -90.0f };
constexpr float ROTATION_SECOND_ORDER { -180.0f };
constexpr float ROTATION_THIRD_ORDER { -270.0f };
constexpr uint32_t TRANSPARENT_COLOR_ARGB { 0x00000000 };
constexpr int32_t DEFAULT_MOUSE_SIZE { 1 };
constexpr int32_t DEFAULT_COLOR_VALUE { 0 };
constexpr int32_t INVALID_COLOR_VALUE { -1 };
constexpr int32_t GLOBAL_WINDOW_ID { -1 };
constexpr int32_t MOUSE_DRAG_CURSOR_CIRCLE_STYLE { 41 };
constexpr int32_t CURSOR_CIRCLE_MIDDLE { 2 };
constexpr size_t EXTRA_INFO_MAX_SIZE { 200 };
constexpr int32_t HEX_FF { 0xFF };
const Rosen::RSAnimationTimingCurve SPRING = Rosen::RSAnimationTimingCurve::CreateSpring(0.347f, 0.99f, 0.0f);
const std::string DEVICE_TYPE_DEFAULT { "default" };
const std::string DEVICE_TYPE_PHONE { "phone" };
constexpr float BEZIER_000 { 0.00f };
constexpr float BEZIER_020 { 0.20f };
constexpr float BEZIER_030 { 0.30f };
constexpr float BEZIER_033 { 0.33f };
constexpr float BEZIER_040 { 0.40f };
constexpr float BEZIER_060 { 0.60f };
constexpr float BEZIER_067 { 0.67f };
constexpr float BEZIER_100 { 1.00f };
constexpr int32_t TIME_DRAG_CHANGE_STYLE { 50 };
constexpr int32_t TIME_DRAG_STYLE { 100 };
constexpr int32_t TIME_STOP_FAIL_WINDOW { 125 };
constexpr int32_t TIME_STOP_SUCCESS_WINDOW { 250 };
constexpr int32_t TIME_STOP_SUCCESS_STYLE { 150 };
constexpr int32_t TIME_STOP { 0 };
constexpr int64_t TIME_SLEEP { 30000 };
constexpr int32_t INTERRUPT_SCALE { 15 };
const std::string THREAD_NAME { "os_AnimationEventRunner" };
const std::string COPY_DRAG_PATH { "/system/etc/device_status/drag_icon/Copy_Drag.svg" };
const std::string COPY_ONE_DRAG_PATH { "/system/etc/device_status/drag_icon/Copy_One_Drag.svg" };
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

bool CheckNodesValid()
{
    CALL_DEBUG_ENTER;
    if (g_drawingInfo.nodes.empty() || g_drawingInfo.nodes[DRAG_STYLE_INDEX] == nullptr) {
        FI_HILOGE("Nodes invalid");
        return false;
    }
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
    if (g_drawingInfo.isExistScalingValue) {
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
    g_drawingInfo.isExistScalingValue = true;
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
    CreateWindow();
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
    OnStartDrag(dragAnimationData, shadowNode, dragStyleNode);
    if (!g_drawingInfo.multiSelectedNodes.empty()) {
        g_drawingInfo.isCurrentDefaultStyle = true;
        UpdateDragStyle(DragCursorStyle::MOVE);
    }
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
    if (g_drawingInfo.isRunning) {
        FI_HILOGE("Drag drawing is running, can not init again");
        return INIT_CANCEL;
    }
    if (dragData.shadowInfos.empty()) {
        FI_HILOGE("ShadowInfos is empty");
        return INIT_FAIL;
    }
    CHKPR(dragData.shadowInfos.front().pixelMap, INIT_FAIL);
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

void DragDrawing::Draw(int32_t displayId, int32_t displayX, int32_t displayY, bool isNeedAdjustDisplayXY)
{
    if (displayId < 0) {
        FI_HILOGE("Invalid displayId:%{public}d", displayId);
        return;
    }
    if (isNeedAdjustDisplayXY) {
        RotateDisplayXY(displayX, displayY);
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
    int32_t positionX = g_drawingInfo.displayX + g_drawingInfo.pixelMapX;
    int32_t positionY = g_drawingInfo.displayY + g_drawingInfo.pixelMapY - adjustSize;
    CHKPV(g_drawingInfo.parentNode);
    CHKPV(g_drawingInfo.pixelMap);
    g_drawingInfo.parentNode->SetBounds(positionX, positionY, g_drawingInfo.pixelMap->GetWidth(),
        g_drawingInfo.pixelMap->GetHeight());
    g_drawingInfo.parentNode->SetFrame(positionX, positionY, g_drawingInfo.pixelMap->GetWidth(),
        g_drawingInfo.pixelMap->GetHeight());
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        DoDrawMouse();
    }
    if (!g_drawingInfo.multiSelectedNodes.empty() && !g_drawingInfo.multiSelectedPixelMaps.empty()) {
        MultiSelectedAnimation(positionX, positionY, adjustSize);
    }
    Rosen::RSTransaction::FlushImplicitTransaction();
}

int32_t DragDrawing::UpdateDragStyle(DragCursorStyle style)
{
    FI_HILOGD("style:%{public}d", style);
    if ((style < DragCursorStyle::DEFAULT) || (style > DragCursorStyle::MOVE)) {
        DragDFX::WriteUpdateDragStyle(style, OHOS::HiviewDFX::HiSysEvent::EventType::FAULT);
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if ((style == DragCursorStyle::DEFAULT) ||
        ((style == DragCursorStyle::MOVE) && (g_drawingInfo.currentDragNum == DRAG_NUM_ONE))) {
        return UpdateDefaultDragStyle(style);
    }
    return UpdateValidDragStyle(style);
}

int32_t DragDrawing::UpdateShadowPic(const ShadowInfo &shadowInfo)
{
    CALL_DEBUG_ENTER;
    CHKPR(shadowInfo.pixelMap, RET_ERR);
    g_drawingInfo.pixelMap = shadowInfo.pixelMap;
    g_drawingInfo.pixelMapX = shadowInfo.x;
    g_drawingInfo.pixelMapY = shadowInfo.y;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return RET_ERR;
    }
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPR(shadowNode, RET_ERR);
    DrawShadow(shadowNode);
    float scalingValue = GetScaling();
    if (SCALE_THRESHOLD_TWELVE < scalingValue || fabsf(SCALE_THRESHOLD_TWELVE - scalingValue) < EPSILON) {
        FI_HILOGE("Invalid scalingValue:%{public}f", scalingValue);
        return RET_ERR;
    }
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        DrawMouseIcon();
    }
    ProcessFilter();
    Draw(g_drawingInfo.displayId, g_drawingInfo.displayX, g_drawingInfo.displayY, false);
    RotateDragWindow(rotation_);
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
    CALL_DEBUG_ENTER;
    startNum_ = START_TIME;
    needDestroyDragWindow_ = false;
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
    ClearMultiSelectedData();
    if (!g_drawingInfo.nodes.empty()) {
        g_drawingInfo.nodes.clear();
        g_drawingInfo.nodes.shrink_to_fit();
    }
    if (g_drawingInfo.parentNode != nullptr) {
        g_drawingInfo.parentNode->ClearChildren();
        g_drawingInfo.parentNode.reset();
        g_drawingInfo.parentNode = nullptr;
    }
    if (g_drawingInfo.rootNode != nullptr) {
        g_drawingInfo.rootNode->ClearChildren();
        g_drawingInfo.rootNode.reset();
        g_drawingInfo.rootNode = nullptr;
    }
    if (g_drawingInfo.surfaceNode != nullptr) {
        g_drawingInfo.surfaceNode->DetachToDisplay(screenId_);
        screenId_ = 0;
        g_drawingInfo.displayId = -1;
        g_drawingInfo.surfaceNode = nullptr;
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

void DragDrawing::OnStartStyleAnimation()
{
    CALL_DEBUG_ENTER;
    hasRunningStopAnimation_ = false;
}

void DragDrawing::OnStartDrag(const DragAnimationData &dragAnimationData,
    std::shared_ptr<Rosen::RSCanvasNode> shadowNode, std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode)
{
    CALL_DEBUG_ENTER;
    CHKPV(shadowNode);
    if (DrawShadow(shadowNode) != RET_OK) {
        FI_HILOGE("Draw shadow failed");
        return;
    }
    g_drawingInfo.isCurrentDefaultStyle = true;
#ifdef OHOS_DRAG_ENABLE_ANIMATION
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(&DragDrawing::OnStartStyleAnimation, this))) {
        FI_HILOGE("Start style animation failed");
    }
#endif // OHOS_DRAG_ENABLE_ANIMATION
}

void DragDrawing::CheckStyleNodeModifier(std::shared_ptr<Rosen::RSCanvasNode> styleNode)
{
    CALL_DEBUG_ENTER;
    CHKPV(styleNode);
    if (drawStyleChangeModifier_ != nullptr) {
        styleNode->RemoveModifier(drawStyleChangeModifier_);
        drawStyleChangeModifier_ = nullptr;
    }
    if (drawStyleScaleModifier_ != nullptr && hasRunningScaleAnimation_) {
        needBreakStyleScaleAnimation_ = true;
    }
    styleNode->RemoveAllAnimations();
}

void DragDrawing::RemoveStyleNodeModifier(std::shared_ptr<Rosen::RSCanvasNode> styleNode)
{
    CALL_DEBUG_ENTER;
    CHKPV(styleNode);
    if (drawStyleChangeModifier_ != nullptr) {
        styleNode->RemoveModifier(drawStyleChangeModifier_);
        drawStyleChangeModifier_ = nullptr;
    }
    if (drawStyleScaleModifier_ != nullptr) {
        styleNode->RemoveModifier(drawStyleScaleModifier_);
        drawStyleScaleModifier_ = nullptr;
    }
}

void DragDrawing::UpdateAnimationProtocol(Rosen::RSAnimationTimingProtocol protocol)
{
    CALL_DEBUG_ENTER;
    startNum_ = START_TIME;
    interruptNum_ = START_TIME * INTERRUPT_SCALE;
    hasRunningAnimation_ = true;
    bool stopSignal = true;
    CHKPV(rsUiDirector_);
    while (hasRunningAnimation_) {
        hasRunningAnimation_ = rsUiDirector_->FlushAnimation(startNum_);
        rsUiDirector_->FlushModifier();
        rsUiDirector_->SendMessages();
        if ((startNum_ >= interruptNum_) && stopSignal) {
            protocol.SetDuration(TIME_STOP);
            stopSignal = false;
        }
        startNum_ += INTERVAL_TIME;
        usleep(TIME_SLEEP);
    }
}

void DragDrawing::StartStyleAnimation(float startScale, float endScale, int32_t duration)
{
    FI_HILOGI("StartStyleAnimation, startScale is %{public}lf", startScale);
    if (!CheckNodesValid() || needBreakStyleScaleAnimation_ || hasRunningStopAnimation_) {
        FI_HILOGE("needBreakStyleScaleAnimation_ or hasRunningStopAnimation_, return");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    RemoveStyleNodeModifier(dragStyleNode);
    drawStyleScaleModifier_ = std::make_shared<DrawStyleScaleModifier>();
    dragStyleNode->AddModifier(drawStyleScaleModifier_);
    CHKPV(drawStyleScaleModifier_);
    drawStyleScaleModifier_->SetScale(startScale);
    Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(duration);
    auto springCurveStyle = endScale == STYLE_END_SCALE
        ? Rosen::RSAnimationTimingCurve::CreateCubicCurve(BEZIER_030, BEZIER_000, BEZIER_040, BEZIER_100)
        : Rosen::RSAnimationTimingCurve::CreateCubicCurve(BEZIER_020, BEZIER_000, BEZIER_060, BEZIER_100);
    CHKPV(drawStyleScaleModifier_);
    Rosen::RSNode::Animate(protocol, springCurveStyle, [&]() {
        drawStyleScaleModifier_->SetScale(endScale);
    });
    UpdateAnimationProtocol(protocol);
    if (endScale == STYLE_CHANGE_SCALE) {
        if (drawStyleChangeModifier_ != nullptr) {
            dragStyleNode->RemoveModifier(drawStyleChangeModifier_);
            drawStyleChangeModifier_ = nullptr;
        }
        if (drawStyleScaleModifier_ != nullptr) {
            dragStyleNode->RemoveModifier(drawStyleScaleModifier_);
            drawStyleScaleModifier_ = nullptr;
        }
        drawStyleChangeModifier_ = std::make_shared<DrawStyleChangeModifier>(g_drawingInfo.stylePixelMap);
        dragStyleNode->AddModifier(drawStyleChangeModifier_);
    }
    if (endScale == STYLE_END_SCALE && drawStyleScaleModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawStyleScaleModifier_);
        drawStyleScaleModifier_ = nullptr;
    }
}

void DragDrawing::ChangeStyleAnimation()
{
    CALL_DEBUG_ENTER;
    hasRunningScaleAnimation_ = true;
    StartStyleAnimation(START_STYLE_SCALE, STYLE_CHANGE_SCALE, TIME_DRAG_CHANGE_STYLE);
    StartStyleAnimation(STYLE_CHANGE_SCALE, STYLE_MAX_SCALE, TIME_DRAG_CHANGE_STYLE);
    StartStyleAnimation(STYLE_MAX_SCALE, STYLE_END_SCALE, TIME_DRAG_STYLE);
    needBreakStyleScaleAnimation_ = false;
    hasRunningScaleAnimation_ = false;
}

void DragDrawing::OnDragStyleAnimation()
{
    CALL_DEBUG_ENTER;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    needBreakStyleScaleAnimation_ = false;
    if (g_drawingInfo.isPreviousDefaultStyle == true || g_drawingInfo.isCurrentDefaultStyle == true) {
        FI_HILOGE("Has DefaultStyle, change style and return");
        CheckStyleNodeModifier(dragStyleNode);
        drawStyleChangeModifier_ = std::make_shared<DrawStyleChangeModifier>(g_drawingInfo.stylePixelMap);
        dragStyleNode->AddModifier(drawStyleChangeModifier_);
        return;
    }
    rsUiDirector_ = Rosen::RSUIDirector::Create();
    CHKPV(rsUiDirector_);
    rsUiDirector_->Init();
    rsUiDirector_->SetRSSurfaceNode(g_drawingInfo.surfaceNode);
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    CheckStyleNodeModifier(dragStyleNode);
    handler_->PostTask(std::bind(&DragDrawing::ChangeStyleAnimation, this));
}

void DragDrawing::OnDragStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode,
    std::shared_ptr<Media::PixelMap> stylePixelMap)
{
    CALL_DEBUG_ENTER;
    CHKPV(dragStyleNode);
    CHKPV(stylePixelMap);
#ifdef OHOS_DRAG_ENABLE_ANIMATION
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (drawSVGModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawSVGModifier_);
        drawSVGModifier_ = nullptr;
    }
    if (!handler_->PostTask(std::bind(&DragDrawing::OnDragStyleAnimation, this))) {
        FI_HILOGE("Drag style animation failed");
        DrawStyle(dragStyleNode, stylePixelMap);
    }
#else // OHOS_DRAG_ENABLE_ANIMATION
    DrawStyle(dragStyleNode, stylePixelMap);
#endif // OHOS_DRAG_ENABLE_ANIMATION
}

void DragDrawing::OnStopAnimationSuccess()
{
    CALL_DEBUG_ENTER;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    if (dragStyleNode != nullptr && drawStyleScaleModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawStyleScaleModifier_);
        dragStyleNode->RemoveAllAnimations();
        drawStyleScaleModifier_ = nullptr;
        needBreakStyleScaleAnimation_ = true;
    }
    CHKPV(g_drawingInfo.rootNode);
    hasRunningStopAnimation_ = true;
    if (drawDragStopModifier_ != nullptr) {
        g_drawingInfo.rootNode->RemoveModifier(drawDragStopModifier_);
        drawDragStopModifier_ = nullptr;
    }
    drawDragStopModifier_ = std::make_shared<DrawDragStopModifier>();
    g_drawingInfo.rootNode->AddModifier(drawDragStopModifier_);
    drawDragStopModifier_->SetAlpha(BEGIN_ALPHA);
    drawDragStopModifier_->SetScale(BEGIN_SCALE);
    drawDragStopModifier_->SetStyleScale(START_STYLE_SCALE);
    drawDragStopModifier_->SetStyleAlpha(START_STYLE_ALPHA);
    Rosen::RSAnimationTimingProtocol windowProtocol;
    Rosen::RSAnimationTimingProtocol styleProtocol;
    windowProtocol.SetDuration(TIME_STOP_SUCCESS_WINDOW);
    styleProtocol.SetDuration(TIME_STOP_SUCCESS_STYLE);
    auto springCurveSuccessWindow = Rosen::RSAnimationTimingCurve::CreateCubicCurve(BEZIER_040, BEZIER_000,
        BEZIER_100, BEZIER_100);
    auto springCurveSuccessStyle = Rosen::RSAnimationTimingCurve::CreateCubicCurve(BEZIER_000, BEZIER_000,
        BEZIER_100, BEZIER_100);
    Rosen::RSNode::Animate(windowProtocol, springCurveSuccessWindow, [&]() {
        drawDragStopModifier_->SetAlpha(BEGIN_ALPHA);
        drawDragStopModifier_->SetScale(END_SCALE_SUCCESS);
        Rosen::RSNode::Animate(styleProtocol, springCurveSuccessStyle, [&]() {
            drawDragStopModifier_->SetStyleAlpha(END_STYLE_ALPHA);
            drawDragStopModifier_->SetStyleScale(START_STYLE_SCALE);
        });
    });
}

void DragDrawing::OnStopDragSuccess(std::shared_ptr<Rosen::RSCanvasNode> shadowNode,
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode)
{
    CALL_DEBUG_ENTER;
    auto animateCb = std::bind(&DragDrawing::InitVSync, this, END_ALPHA, END_SCALE_SUCCESS);
#ifdef OHOS_DRAG_ENABLE_ANIMATION
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(&DragDrawing::OnStopAnimationSuccess, this))) {
        FI_HILOGE("Failed to stop style animation");
        RunAnimation(animateCb);
    } else {
        startNum_ = START_TIME;
        needDestroyDragWindow_ = true;
        StartVsync();
    }
#else // OHOS_DRAG_ENABLE_ANIMATION
    RunAnimation(animateCb);
#endif // OHOS_DRAG_ENABLE_ANIMATION
}

void DragDrawing::OnStopAnimationFail()
{
    CALL_DEBUG_ENTER;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    if (dragStyleNode != nullptr && drawStyleScaleModifier_ != nullptr) {
        dragStyleNode->RemoveModifier(drawStyleScaleModifier_);
        dragStyleNode->RemoveAllAnimations();
        drawStyleScaleModifier_ = nullptr;
        needBreakStyleScaleAnimation_ = true;
    }
    CHKPV(g_drawingInfo.rootNode);
    if (drawDragStopModifier_ != nullptr) {
        g_drawingInfo.rootNode->RemoveModifier(drawDragStopModifier_);
        drawDragStopModifier_ = nullptr;
    }
    drawDragStopModifier_ = std::make_shared<DrawDragStopModifier>();
    hasRunningStopAnimation_ = true;
    g_drawingInfo.rootNode->AddModifier(drawDragStopModifier_);
    drawDragStopModifier_->SetAlpha(BEGIN_ALPHA);
    drawDragStopModifier_->SetScale(BEGIN_SCALE);
    drawDragStopModifier_->SetStyleScale(START_STYLE_SCALE);
    drawDragStopModifier_->SetStyleAlpha(START_STYLE_ALPHA);
    Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(TIME_STOP_FAIL_WINDOW);
    auto springCurveFail = Rosen::RSAnimationTimingCurve::CreateCubicCurve(BEZIER_033, BEZIER_000,
        BEZIER_067, BEZIER_100);
    Rosen::RSNode::Animate(protocol, springCurveFail, [&]() {
        drawDragStopModifier_->SetAlpha(END_ALPHA);
        drawDragStopModifier_->SetScale(END_SCALE_FAIL);
        drawDragStopModifier_->SetStyleScale(START_STYLE_SCALE);
        drawDragStopModifier_->SetStyleAlpha(END_STYLE_ALPHA);
    });
}

void DragDrawing::OnStopDragFail(std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode,
    std::shared_ptr<Rosen::RSNode> rootNode)
{
    CALL_DEBUG_ENTER;
    auto animateCb = std::bind(&DragDrawing::InitVSync, this, END_ALPHA, END_SCALE_FAIL);
#ifdef OHOS_DRAG_ENABLE_ANIMATION
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPV(runner);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(std::bind(&DragDrawing::OnStopAnimationFail, this))) {
        FI_HILOGE("Failed to stop style animation");
        RunAnimation(animateCb);
    } else {
        startNum_ = START_TIME;
        needDestroyDragWindow_ = true;
        StartVsync();
    }
#else // OHOS_DRAG_ENABLE_ANIMATION
    RunAnimation(animateCb);
#endif // OHOS_DRAG_ENABLE_ANIMATION
}

void DragDrawing::OnStopAnimation()
{
    CALL_DEBUG_ENTER;
}

int32_t DragDrawing::RunAnimation(std::function<int32_t()> cb)
{
    CALL_DEBUG_ENTER;
    if (handler_ == nullptr) {
        auto runner = AppExecFwk::EventRunner::Create(THREAD_NAME);
        CHKPR(runner, RET_ERR);
        handler_ = std::make_shared<AppExecFwk::EventHandler>(std::move(runner));
    }
    if (!handler_->PostTask(cb)) {
        FI_HILOGE("Send vsync event failed");
        return RET_ERR;
    }
    return RET_OK;
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
    CHKPR(g_drawingInfo.parentNode, RET_ERR);
    Rosen::RSTransaction::FlushImplicitTransaction();
    startNum_ = START_TIME;
    needDestroyDragWindow_ = true;
    return StartVsync();
}

int32_t DragDrawing::StartVsync()
{
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
    ret = receiver_->RequestNextVSync(fcb);
    if (ret != RET_OK) {
        FI_HILOGE("Request next vsync failed");
    }
    return ret;
}

void DragDrawing::OnVsync()
{
    CALL_DEBUG_ENTER;
    CHKPV(rsUiDirector_);
    bool hasRunningAnimation = rsUiDirector_->FlushAnimation(startNum_);
    rsUiDirector_->FlushModifier();
    if (!hasRunningAnimation) {
        FI_HILOGD("Stop runner, hasRunningAnimation:%{public}d", hasRunningAnimation);
        CHKPV(handler_);
        handler_->RemoveAllEvents();
        handler_->RemoveAllFileDescriptorListeners();
        handler_ = nullptr;
        receiver_ = nullptr;
        if (needDestroyDragWindow_) {
            CHKPV(g_drawingInfo.rootNode);
            if (drawDynamicEffectModifier_ != nullptr) {
                g_drawingInfo.rootNode->RemoveModifier(drawDynamicEffectModifier_);
                drawDynamicEffectModifier_ = nullptr;
            }
            DestroyDragWindow();
            g_drawingInfo.isRunning = false;
        }
        return;
    }
    Rosen::VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = std::bind(&DragDrawing::OnVsync, this)
    };
    int32_t ret = receiver_->RequestNextVSync(fcb);
    if (ret != RET_OK) {
        FI_HILOGE("Request next vsync failed");
    }
    rsUiDirector_->SendMessages();
    startNum_ += INTERVAL_TIME;
}

void DragDrawing::InitDrawingInfo(const DragData &dragData)
{
    g_drawingInfo.isRunning = true;
    if (dragData.shadowInfos.empty()) {
        FI_HILOGE("ShadowInfos is empty");
        return;
    }
    size_t shadowInfosSize = dragData.shadowInfos.size();
    for (size_t i = 1; i < shadowInfosSize; ++i) {
        g_drawingInfo.multiSelectedPixelMaps.emplace_back(dragData.shadowInfos[i].pixelMap);
    }
    g_drawingInfo.pixelMap = dragData.shadowInfos.front().pixelMap;
    g_drawingInfo.pixelMapX = dragData.shadowInfos.front().x;
    g_drawingInfo.pixelMapY = dragData.shadowInfos.front().y;
    g_drawingInfo.currentDragNum = dragData.dragNum;
    g_drawingInfo.sourceType = dragData.sourceType;
    g_drawingInfo.displayId = dragData.displayId;
    g_drawingInfo.displayX = dragData.displayX;
    g_drawingInfo.displayY = dragData.displayY;
    RotateDisplayXY(g_drawingInfo.displayX, g_drawingInfo.displayY);
    g_drawingInfo.extraInfo = dragData.extraInfo;
    g_drawingInfo.filterInfo = dragData.filterInfo;
}

int32_t DragDrawing::InitDragAnimationData(DragAnimationData &dragAnimationData)
{
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
    sptr<Rosen::Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    if (display == nullptr) {
        FI_HILOGD("Get display info failed, display:%{public}d", g_drawingInfo.displayId);
        display = Rosen::DisplayManager::GetInstance().GetDisplayById(0);
        if (display == nullptr) {
            FI_HILOGE("Get display info failed, display is nullptr");
            return RET_ERR;
        }
    }
    int32_t rootNodeSize = std::max(display->GetWidth(), display->GetHeight());
    InitCanvas(rootNodeSize, rootNodeSize);
    if (rotation_ != Rosen::Rotation::ROTATION_0) {
        RotateDragWindow(rotation_);
    }
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
    g_drawingInfo.rootNode->SetBounds(0, 0, width, height);
    g_drawingInfo.rootNode->SetFrame(0, 0, width, height);
    g_drawingInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);
    std::shared_ptr<Rosen::RSCanvasNode> filterNode = Rosen::RSCanvasNode::Create();
    CHKPV(filterNode);
    g_drawingInfo.nodes.emplace_back(filterNode);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN) {
        ProcessFilter();
    }
    std::shared_ptr<Rosen::RSCanvasNode> pixelMapNode = Rosen::RSCanvasNode::Create();
    CHKPV(pixelMapNode);
    pixelMapNode->SetForegroundColor(TRANSPARENT_COLOR_ARGB);
    g_drawingInfo.nodes.emplace_back(pixelMapNode);
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = Rosen::RSCanvasNode::Create();
    CHKPV(dragStyleNode);
    g_drawingInfo.nodes.emplace_back(dragStyleNode);
    if (g_drawingInfo.parentNode == nullptr) {
        g_drawingInfo.parentNode = Rosen::RSCanvasNode::Create();
        CHKPV(g_drawingInfo.parentNode);
    }
    g_drawingInfo.parentNode->AddChild(filterNode);
    g_drawingInfo.parentNode->AddChild(pixelMapNode);
    if (!g_drawingInfo.multiSelectedPixelMaps.empty()) {
        InitMultiSelectedNodes();
        if (!g_drawingInfo.multiSelectedNodes.empty()) {
            size_t multiSelectedNodesSize = g_drawingInfo.multiSelectedNodes.size();
            for (size_t i = 0; i < multiSelectedNodesSize; ++i) {
                g_drawingInfo.rootNode->AddChild(g_drawingInfo.multiSelectedNodes[i]);
            }
        }
    }
    g_drawingInfo.rootNode->AddChild(g_drawingInfo.parentNode);
    CHKPV(rsUiDirector_);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = Rosen::RSCanvasNode::Create();
        CHKPV(mouseIconNode);
        g_drawingInfo.nodes.emplace_back(mouseIconNode);
        g_drawingInfo.rootNode->AddChild(mouseIconNode);
        rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
        return;
    }
    rsUiDirector_->SetRoot(g_drawingInfo.rootNode->GetId());
}

void DragDrawing::CreateWindow()
{
    FI_HILOGD("Parameter screen number:%{public}llu", static_cast<unsigned long long>(screenId_));
    Rosen::RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "drag window";
    Rosen::RSSurfaceNodeType surfaceNodeType = Rosen::RSSurfaceNodeType::SELF_DRAWING_WINDOW_NODE;
    g_drawingInfo.surfaceNode = Rosen::RSSurfaceNode::Create(surfaceNodeConfig, surfaceNodeType);
    CHKPV(g_drawingInfo.surfaceNode);
    sptr<Rosen::Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    if (display == nullptr) {
        FI_HILOGD("Get display info failed, display:%{public}d", g_drawingInfo.displayId);
        display = Rosen::DisplayManager::GetInstance().GetDisplayById(0);
        if (display == nullptr) {
            FI_HILOGE("Get display info failed, display is nullptr");
            return;
        }
    }
    int32_t surfaceNodeSize = std::max(display->GetWidth(), display->GetHeight());
    g_drawingInfo.surfaceNode->SetBounds(0, 0, surfaceNodeSize, surfaceNodeSize);
    g_drawingInfo.surfaceNode->SetFrameGravity(Rosen::Gravity::RESIZE_ASPECT_FILL);
    g_drawingInfo.surfaceNode->SetPositionZ(DRAG_WINDOW_POSITION_Z);
    g_drawingInfo.surfaceNode->SetBackgroundColor(SK_ColorTRANSPARENT);
    g_drawingInfo.surfaceNode->AttachToDisplay(screenId_);
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
            filePath = MOVE_DRAG_PATH;
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

bool DragDrawing::ParserFilterInfo(FilterInfo &filterInfo)
{
    FI_HILOGD("ExtraInfo size:%{public}zu, extraInfo:%{public}s, filterInfo size:%{public}zu, filterInfo:%{public}s",
        g_drawingInfo.extraInfo.size(), g_drawingInfo.extraInfo.c_str(), g_drawingInfo.filterInfo.size(),
        g_drawingInfo.filterInfo.c_str());
    if (g_drawingInfo.extraInfo.empty() || g_drawingInfo.filterInfo.empty()) {
        FI_HILOGD("ExtraInfo or filterInfo is empty");
        return false;
    }
    JsonParser extraInfoParser;
    extraInfoParser.json = cJSON_Parse(g_drawingInfo.extraInfo.c_str());
    if (!cJSON_IsObject(extraInfoParser.json)) {
        FI_HILOGE("ExtraInfo is not json object");
        return false;
    }
    cJSON *componentType = cJSON_GetObjectItemCaseSensitive(extraInfoParser.json, "drag_data_type");
    if (!cJSON_IsString(componentType)) {
        FI_HILOGE("Parser componentType failed");
        return false;
    }
    cJSON *blurStyle = cJSON_GetObjectItemCaseSensitive(extraInfoParser.json, "drag_blur_style");
    if (!cJSON_IsNumber(blurStyle)) {
        FI_HILOGE("Parser blurStyle failed");
        return false;
    }
    cJSON *cornerRadius = cJSON_GetObjectItemCaseSensitive(extraInfoParser.json, "drag_corner_radius");
    if (!cJSON_IsNumber(cornerRadius)) {
        FI_HILOGE("Parser cornerRadius failed");
        return false;
    }

    JsonParser filterInfoParser;
    filterInfoParser.json = cJSON_Parse(g_drawingInfo.filterInfo.c_str());
    if (!cJSON_IsObject(filterInfoParser.json)) {
        FI_HILOGE("FilterInfo is not json object");
        return false;
    }
    cJSON *dipScale = cJSON_GetObjectItemCaseSensitive(filterInfoParser.json, "dip_scale");
    if (!cJSON_IsNumber(dipScale)) {
        FI_HILOGE("Parser dipScale failed");
        return false;
    }
    filterInfo = { componentType->valuestring, blurStyle->valueint, cornerRadius->valuedouble, dipScale->valuedouble };
    return true;
}

bool DragDrawing::GetAllowDragState()
{
    if (g_drawingInfo.extraInfo.empty()) {
        FI_HILOGE("The extraInfo is empty");
        return true;
    }
    if (g_drawingInfo.extraInfo.size() > EXTRA_INFO_MAX_SIZE) {
        FI_HILOGE("The extraInfo is greater than the length limit");
        return true;
    }
    JsonParser extraInfoParser;
    extraInfoParser.json = cJSON_Parse(g_drawingInfo.extraInfo.c_str());
    if (!cJSON_IsObject(extraInfoParser.json)) {
        FI_HILOGE("The extraInfo is not a json object");
        return true;
    }
    bool ret = static_cast<bool>(cJSON_HasObjectItem(extraInfoParser.json, "drag_allow_distributed"));
    if (!ret) {
        FI_HILOGE("Can't found \'drag_allow_distributed\' in extraInfo");
        return true;
    }
    cJSON *json = cJSON_GetObjectItem(extraInfoParser.json, "drag_allow_distributed");
    if (json == nullptr || !cJSON_IsBool(json)) {
        FI_HILOGE("Can't parse \'drag_allow_distributed\' in extraInfo");
        return true;
    }
    bool isAllowDrag = cJSON_IsTrue(json) ? true : false;
    FI_HILOGD("Parse extraInfo, the \'drag_allow_distributed\' is %{public}s", isAllowDrag ? "true" : "false");

    return isAllowDrag;
}

void DragDrawing::SetScreenId(uint64_t screenId)
{
    CALL_DEBUG_ENTER;
    screenId_ = screenId;
}

int32_t DragDrawing::RotateDragWindow(Rosen::Rotation rotation)
{
    switch (rotation) {
        case Rosen::Rotation::ROTATION_0: {
            return DoRotateDragWindow(ROTATION_DEFAULT);
        }
        case Rosen::Rotation::ROTATION_90: {
            return DoRotateDragWindow(ROTATION_FIRST_ORDER);
        }
        case Rosen::Rotation::ROTATION_180: {
            return DoRotateDragWindow(ROTATION_SECOND_ORDER);
        }
        case Rosen::Rotation::ROTATION_270: {
            return DoRotateDragWindow(ROTATION_THIRD_ORDER);
        }
        default: {
            FI_HILOGE("Invalid parameter, rotation:%{public}d", static_cast<int32_t>(rotation));
            return RET_ERR;
        }
    }
    return RET_OK;
}

void DragDrawing::SetRotation(Rosen::Rotation rotation)
{
    rotation_ = rotation;
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
        if (filterInfo.cornerRadius < 0 || filterInfo.dipScale < 0 || fabs(filterInfo.dipScale) < EPSILON ||
            std::numeric_limits<float>::max() / filterInfo.dipScale < filterInfo.cornerRadius) {
            FI_HILOGE("Invalid parameters, cornerRadius:%{public}f, dipScale:%{public}f",
                filterInfo.cornerRadius, filterInfo.dipScale);
            return;
        }
        filterNode->SetCornerRadius(filterInfo.cornerRadius * filterInfo.dipScale);
        FI_HILOGD("Add filter successfully");
    }
}

int32_t DragDrawing::SetNodesLocation(int32_t positionX, int32_t positionY)
{
    CALL_DEBUG_ENTER;
    Rosen::RSAnimationTimingProtocol protocol;
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    CHKPR(g_drawingInfo.parentNode, RET_ERR);
    CHKPR(g_drawingInfo.pixelMap, RET_ERR);
    Rosen::RSNode::Animate(protocol, SPRING, [&]() {
        g_drawingInfo.parentNode->SetBounds(positionX, positionY, g_drawingInfo.pixelMap->GetWidth() + adjustSize,
            g_drawingInfo.pixelMap->GetHeight() + adjustSize);
        g_drawingInfo.parentNode->SetFrame(positionX, positionY, g_drawingInfo.pixelMap->GetWidth() + adjustSize,
            g_drawingInfo.pixelMap->GetHeight() + adjustSize);
    });
    startNum_ = START_TIME;
    needDestroyDragWindow_ = false;
    StartVsync();
    return RET_OK;
}


int32_t DragDrawing::EnterTextEditorArea(bool enable)
{
    CALL_DEBUG_ENTER;
    if (enable) {
        DRAG_DATA_MGR.SetInitialPixelMapLocation({ g_drawingInfo.pixelMapX, g_drawingInfo.pixelMapY });
        g_drawingInfo.pixelMapX = -(HALF_RATIO * g_drawingInfo.pixelMap->GetWidth());
        g_drawingInfo.pixelMapY = -(EIGHT_SIZE * GetScaling());
    } else {
        auto initialPixelMapLocation = DRAG_DATA_MGR.GetInitialPixelMapLocation();
        g_drawingInfo.pixelMapX = initialPixelMapLocation.first;
        g_drawingInfo.pixelMapY = initialPixelMapLocation.second;
    }
    DRAG_DATA_MGR.SetPixelMapLocation({ g_drawingInfo.pixelMapX, g_drawingInfo.pixelMapY });
    int32_t positionX = g_drawingInfo.displayX + g_drawingInfo.pixelMapX;
    int32_t positionY = g_drawingInfo.displayY + g_drawingInfo.pixelMapY - TWELVE_SIZE * GetScaling();
    if (RunAnimation(std::bind(&DragDrawing::SetNodesLocation, this, positionX, positionY)) != RET_OK) {
        FI_HILOGE("RunAnimation to SetNodesLocation failed");
        return RET_ERR;
    }
    DRAG_DATA_MGR.SetTextEditorAreaFlag(enable);
    FI_HILOGI("EnterTextEditorArea %{public}s successfully", (enable ? "true" : "false"));
    return RET_OK;
}

float DragDrawing::RadiusVp2Sigma(float radiusVp, float dipScale)
{
    float radiusPx = radiusVp * dipScale;
    return radiusPx > 0.0f ? BLUR_SIGMA_SCALE * radiusPx + 0.5f : 0.0f;
}

int32_t DragDrawing::UpdatePreviewStyle(const PreviewStyle &previewStyle)
{
    CALL_DEBUG_ENTER;
    if (ModifyPreviewStyle(g_drawingInfo.nodes[PIXEL_MAP_INDEX], previewStyle) != RET_OK) {
        FI_HILOGE("ModifyPreviewStyle failed");
        return RET_ERR;
    }
    if (ModifyMultiPreviewStyle(std::vector<PreviewStyle>(g_drawingInfo.multiSelectedNodes.size(), previewStyle)) !=
        RET_OK) {
        FI_HILOGE("ModifyPreviewStyle failed");
        return RET_ERR;
    }
    Rosen::RSTransaction::FlushImplicitTransaction();
    return RET_OK;
}

int32_t DragDrawing::UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle,
    const PreviewAnimation &animation)
{
    CALL_DEBUG_ENTER;
    std::shared_ptr<Rosen::RSCanvasNode> pixelMapNode = g_drawingInfo.nodes[PIXEL_MAP_INDEX];
    CHKPR(pixelMapNode, RET_ERR);
    PreviewStyle originStyle;
    originStyle.types = previewStyle.types;
    if (auto color = pixelMapNode->GetShowingProperties().GetForegroundColor(); color.has_value()) {
        originStyle.foregroundColor = color->AsArgbInt();
        originStyle.radius = previewStyle.radius;
    }
    size_t multiSelectedNodesSize = g_drawingInfo.multiSelectedNodes.size();
    std::vector<PreviewStyle> multiOriginStyles;
    for (size_t i = 0; i < multiSelectedNodesSize; ++i) {
        if (auto color = g_drawingInfo.multiSelectedNodes[i]->GetShowingProperties().GetForegroundColor();
            color.has_value()) {
            PreviewStyle currentStyle;
            currentStyle.types = { PreviewType::FOREGROUND_COLOR, PreviewType::RADIUS };
            currentStyle.foregroundColor = color->AsArgbInt();
            currentStyle.radius = previewStyle.radius;
            multiOriginStyles.push_back(currentStyle);
        }
    }
    if (ModifyPreviewStyle(pixelMapNode, originStyle) != RET_OK) {
        FI_HILOGE("ModifyPreviewStyle failed");
        return RET_ERR;
    }
    if (ModifyMultiPreviewStyle(multiOriginStyles) != RET_OK) {
        FI_HILOGE("ModifyMultiPreviewStyle failed");
        return RET_ERR;
    }
    Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(animation.duration);
    auto curve = AnimationCurve::CreateCurve(animation.curveName, animation.curve);
    Rosen::RSNode::Animate(protocol, curve, [&]() {
        if (ModifyPreviewStyle(pixelMapNode, previewStyle) != RET_OK) {
            FI_HILOGE("ModifyPreviewStyle failed");
        }
        if (ModifyMultiPreviewStyle(std::vector<PreviewStyle>(multiSelectedNodesSize, previewStyle)) != RET_OK) {
            FI_HILOGE("ModifyMultiPreviewStyle failed");
        }
    });
    return RET_OK;
}

void DragDrawing::DoDrawMouse()
{
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    MMI::PointerStyle pointerStyle;
    int32_t ret = MMI::InputManager::GetInstance()->GetPointerStyle(GLOBAL_WINDOW_ID, pointerStyle);
    if (ret != RET_OK) {
        FI_HILOGE("Get pointer style failed, ret:%{public}d", ret);
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    int32_t pointerStyleId = pointerStyle.id;
    if (pointerStyleId == MOUSE_DRAG_CURSOR_CIRCLE_STYLE) {
        int32_t positionX = g_drawingInfo.displayX - (g_drawingInfo.mouseWidth / CURSOR_CIRCLE_MIDDLE);
        int32_t positionY = g_drawingInfo.displayY - (g_drawingInfo.mouseHeight / CURSOR_CIRCLE_MIDDLE);
        mouseIconNode->SetBounds(positionX, positionY, g_drawingInfo.mouseWidth, g_drawingInfo.mouseHeight);
        mouseIconNode->SetFrame(positionX, positionY, g_drawingInfo.mouseWidth, g_drawingInfo.mouseHeight);
    } else {
        mouseIconNode->SetBounds(g_drawingInfo.displayX, g_drawingInfo.displayY,
            g_drawingInfo.mouseWidth, g_drawingInfo.mouseHeight);
        mouseIconNode->SetFrame(g_drawingInfo.displayX, g_drawingInfo.displayY,
            g_drawingInfo.mouseWidth, g_drawingInfo.mouseHeight);
    }
}

int32_t DragDrawing::UpdateDefaultDragStyle(DragCursorStyle style)
{
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return RET_ERR;
    }
    if (!g_drawingInfo.isCurrentDefaultStyle) {
        std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
        CHKPR(dragStyleNode, RET_ERR);
        CHKPR(g_drawingInfo.parentNode, RET_ERR);
        g_drawingInfo.parentNode->RemoveChild(dragStyleNode);
        CHKPR(rsUiDirector_, RET_ERR);
        rsUiDirector_->SendMessages();
    }
    g_drawingInfo.currentStyle = style;
    bool isPreviousDefaultStyle = g_drawingInfo.isCurrentDefaultStyle;
    g_drawingInfo.isPreviousDefaultStyle = isPreviousDefaultStyle;
    g_drawingInfo.isCurrentDefaultStyle = true;
    return RET_OK;
}

int32_t DragDrawing::UpdateValidDragStyle(DragCursorStyle style)
{
    g_drawingInfo.currentStyle = style;
    if (g_drawingInfo.isCurrentDefaultStyle) {
        if (!CheckNodesValid()) {
            FI_HILOGE("Check nodes valid failed");
            return RET_ERR;
        }
        std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
        CHKPR(dragStyleNode, RET_ERR);
        CHKPR(g_drawingInfo.parentNode, RET_ERR);
        g_drawingInfo.parentNode->AddChild(dragStyleNode);
    }
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
    g_drawingInfo.isCurrentDefaultStyle = false;
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
    DragDFX::WriteUpdateDragStyle(style, OHOS::HiviewDFX::HiSysEvent::EventType::BEHAVIOR);
    return RET_OK;
}

int32_t DragDrawing::ModifyPreviewStyle(std::shared_ptr<Rosen::RSCanvasNode> node, const PreviewStyle &previewStyle)
{
    CALL_DEBUG_ENTER;
    CHKPR(node, RET_ERR);
    if (float radius = 0.0F; ParserRadius(radius)) {
        node->SetCornerRadius(radius);
        FI_HILOGD("SetCornerRadius by radius:%{public}f", radius);
    }
    for (const auto &type : previewStyle.types) {
        switch (type) {
            case PreviewType::FOREGROUND_COLOR: {
                node->SetForegroundColor(previewStyle.foregroundColor);
                break;
            }
            case PreviewType::OPACITY: {
                node->SetAlpha(previewStyle.opacity / static_cast<float>(HEX_FF));
                break;
            }
            case PreviewType::RADIUS: {
                node->SetCornerRadius(previewStyle.radius);
                break;
            }
            case PreviewType::SCALE: {
                node->SetScale(previewStyle.scale);
                break;
            }
            default: {
                FI_HILOGE("Unsupported type");
                break;
            }
        }
    }
    return RET_OK;
}

int32_t DragDrawing::ModifyMultiPreviewStyle(const std::vector<PreviewStyle> &previewStyles)
{
    size_t multiSelectedNodesSize = g_drawingInfo.multiSelectedNodes.size();
    if (previewStyles.size() != multiSelectedNodesSize) {
        FI_HILOGE("Size of previewStyles:%{public}zu does not match multiSelectedNodesSize:%{public}zu",
            previewStyles.size(), multiSelectedNodesSize);
        return RET_ERR;
    }
    for (size_t i = 0; i < multiSelectedNodesSize; ++i) {
        if (ModifyPreviewStyle(g_drawingInfo.multiSelectedNodes[i], previewStyles[i]) != RET_OK) {
            FI_HILOGW("ModifyPreviewStyle No.%{public}zu failed", i);
        }
    }
    return RET_OK;
}

void DragDrawing::MultiSelectedAnimation(int32_t positionX, int32_t positionY, int32_t adjustSize)
{
    size_t multiSelectedNodesSize = g_drawingInfo.multiSelectedNodes.size();
    size_t multiSelectedPixelMapsSize = g_drawingInfo.multiSelectedPixelMaps.size();
    for (size_t i = 0; (i < multiSelectedNodesSize) && (i < multiSelectedPixelMapsSize); ++i) {
        std::shared_ptr<Rosen::RSCanvasNode> multiSelectedNode = g_drawingInfo.multiSelectedNodes[i];
        std::shared_ptr<Media::PixelMap> multiSelectedPixelMap = g_drawingInfo.multiSelectedPixelMaps[i];
        Rosen::RSAnimationTimingProtocol protocol;
        if (i == FIRST_PIXELMAP_INDEX) {
            protocol.SetDuration(SHORT_DURATION);
        } else {
            protocol.SetDuration(LONG_DURATION);
        }
        Rosen::RSNode::Animate(protocol, Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
            multiSelectedNode->SetBounds(positionX, positionY + adjustSize, multiSelectedPixelMap->GetWidth(),
                multiSelectedPixelMap->GetHeight());
            multiSelectedNode->SetFrame(positionX, positionY + adjustSize, multiSelectedPixelMap->GetWidth(),
                multiSelectedPixelMap->GetHeight());
        });
    }
}

void DragDrawing::InitMultiSelectedNodes()
{
    CALL_DEBUG_ENTER;
    size_t multiSelectedPixelMapsSize = g_drawingInfo.multiSelectedPixelMaps.size();
    for (size_t i = 0; i < multiSelectedPixelMapsSize; ++i) {
        std::shared_ptr<Media::PixelMap> multiSelectedPixelMap = g_drawingInfo.multiSelectedPixelMaps[i];
        std::shared_ptr<Rosen::RSCanvasNode> multiSelectedNode = Rosen::RSCanvasNode::Create();
        multiSelectedNode->SetBgImageWidth(multiSelectedPixelMap->GetWidth());
        multiSelectedNode->SetBgImageHeight(multiSelectedPixelMap->GetHeight());
        multiSelectedNode->SetBgImagePositionX(0);
        multiSelectedNode->SetBgImagePositionY(0);
        multiSelectedNode->SetForegroundColor(TRANSPARENT_COLOR_ARGB);
        auto rosenImage = std::make_shared<Rosen::RSImage>();
        rosenImage->SetPixelMap(multiSelectedPixelMap);
        rosenImage->SetImageRepeat(0);
        multiSelectedNode->SetBgImage(rosenImage);
        float alpha = DEFAULT_ALPHA;
        float degrees = DEFAULT_ANGLE;
        if (i == FIRST_PIXELMAP_INDEX) {
            alpha = FIRST_PIXELMAP_ALPHA;
            degrees = POSITIVE_ANGLE;
        } else if (i == SECOND_PIXELMAP_INDEX) {
            alpha = SECOND_PIXELMAP_ALPHA;
            degrees = NEGATIVE_ANGLE;
        }
        multiSelectedNode->SetRotation(degrees);
        multiSelectedNode->SetAlpha(alpha);
        g_drawingInfo.multiSelectedNodes.emplace_back(multiSelectedNode);
    }
}

void DragDrawing::ClearMultiSelectedData()
{
    CALL_DEBUG_ENTER;
    if (!g_drawingInfo.multiSelectedNodes.empty()) {
        g_drawingInfo.multiSelectedNodes.clear();
        g_drawingInfo.multiSelectedNodes.shrink_to_fit();
    }
    if (!g_drawingInfo.multiSelectedPixelMaps.empty()) {
        g_drawingInfo.multiSelectedPixelMaps.clear();
        g_drawingInfo.multiSelectedPixelMaps.shrink_to_fit();
    }
}

void DragDrawing::RotateDisplayXY(int32_t &displayX, int32_t &displayY)
{
    sptr<Rosen::Display> display = Rosen::DisplayManager::GetInstance().GetDisplayById(g_drawingInfo.displayId);
    if (display == nullptr) {
        FI_HILOGD("Get display info failed, display:%{public}d", g_drawingInfo.displayId);
        display = Rosen::DisplayManager::GetInstance().GetDisplayById(0);
        CHKPV(display);
    }
    switch (rotation_) {
        case Rosen::Rotation::ROTATION_0: {
            break;
        }
        case Rosen::Rotation::ROTATION_90: {
            int32_t temp = displayY;
            displayY = display->GetWidth() - displayX;
            displayX = temp;
            break;
        }
        case Rosen::Rotation::ROTATION_180: {
            displayX = display->GetWidth() - displayX;
            displayY = display->GetHeight() - displayY;
            break;
        }
        case Rosen::Rotation::ROTATION_270: {
            int32_t temp = displayX;
            displayX = display->GetHeight() - displayY;
            displayY = temp;
            break;
        }
        default: {
            FI_HILOGE("Invalid parameter, rotation:%{public}d", static_cast<int32_t>(rotation_));
            break;
        }
    }
}

int32_t DragDrawing::DoRotateDragWindow(float rotation)
{
    FI_HILOGD("rotation:%{public}f", rotation);
    CHKPR(g_drawingInfo.parentNode, RET_ERR);
    CHKPR(g_drawingInfo.pixelMap, RET_ERR);
    if ((g_drawingInfo.pixelMap->GetWidth() <= 0) || (g_drawingInfo.pixelMap->GetHeight() <= 0)) {
        FI_HILOGE("Invalid parameter pixelmap");
        return RET_ERR;
    }
    float pivotX = 0.0f;
    float pivotY = 0.0f;
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    if ((g_drawingInfo.currentStyle == DragCursorStyle::DEFAULT) ||
        ((g_drawingInfo.currentStyle == DragCursorStyle::MOVE) && (g_drawingInfo.currentDragNum == DRAG_NUM_ONE))) {
        pivotX = -g_drawingInfo.pixelMapX * 1.0 / g_drawingInfo.pixelMap->GetWidth();
        pivotY = (-g_drawingInfo.pixelMapY + adjustSize) * 1.0 / g_drawingInfo.pixelMap->GetHeight();
    } else {
        pivotX = -g_drawingInfo.pixelMapX * 1.0 / (g_drawingInfo.pixelMap->GetWidth() + adjustSize);
        pivotY = (-g_drawingInfo.pixelMapY + adjustSize) * 1.0 / (g_drawingInfo.pixelMap->GetHeight() + adjustSize);
    }
    g_drawingInfo.parentNode->SetPivot(pivotX, pivotY);
    g_drawingInfo.parentNode->SetRotation(rotation);
    if (g_drawingInfo.sourceType == MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        if (!CheckNodesValid()) {
            FI_HILOGE("Check nodes valid failed");
            return RET_ERR;
        }
        std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
        CHKPR(mouseIconNode, RET_ERR);
        mouseIconNode->SetPivot(DEFAULT_PIVOT, DEFAULT_PIVOT);
        mouseIconNode->SetRotation(rotation);
    }
    Rosen::RSTransaction::FlushImplicitTransaction();
    return RET_OK;
}

bool DragDrawing::ParserRadius(float &radius)
{
    FI_HILOGD("ExtraInfo size:%{public}zu, extraInfo:%{public}s, filterInfo size:%{public}zu, filterInfo:%{public}s",
        g_drawingInfo.extraInfo.size(), g_drawingInfo.extraInfo.c_str(), g_drawingInfo.filterInfo.size(),
        g_drawingInfo.filterInfo.c_str());
    if (g_drawingInfo.extraInfo.empty() || g_drawingInfo.filterInfo.empty()) {
        FI_HILOGD("ExtraInfo or filterInfo is empty");
        return false;
    }
    JsonParser extraInfoParser;
    extraInfoParser.json = cJSON_Parse(g_drawingInfo.extraInfo.c_str());
    if (!cJSON_IsObject(extraInfoParser.json)) {
        FI_HILOGE("ExtraInfo is not json object");
        return false;
    }
    cJSON *cornerRadius = cJSON_GetObjectItemCaseSensitive(extraInfoParser.json, "drag_corner_radius");
    if (!cJSON_IsNumber(cornerRadius)) {
        FI_HILOGE("Parser cornerRadius failed");
        return false;
    }

    JsonParser filterInfoParser;
    filterInfoParser.json = cJSON_Parse(g_drawingInfo.filterInfo.c_str());
    if (!cJSON_IsObject(filterInfoParser.json)) {
        FI_HILOGE("FilterInfo is not json object");
        return false;
    }
    cJSON *dipScale = cJSON_GetObjectItemCaseSensitive(filterInfoParser.json, "dip_scale");
    if (!cJSON_IsNumber(dipScale)) {
        FI_HILOGE("Parser dipScale failed");
        return false;
    }
    if (cornerRadius->valuedouble < 0 || dipScale->valuedouble < 0 || fabs(dipScale->valuedouble) < EPSILON ||
        std::numeric_limits<float>::max() / dipScale->valuedouble < cornerRadius->valuedouble) {
        FI_HILOGE("Invalid parameters, cornerRadius:%{public}f, dipScale:%{public}f",
            cornerRadius->valuedouble, dipScale->valuedouble);
        return false;
    }
    radius = cornerRadius->valuedouble * dipScale->valuedouble;
    return true;
}

void DrawSVGModifier::Draw(Rosen::RSDrawingContext& context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(stylePixelMap_);
    CHKPV(g_drawingInfo.pixelMap);
    float scalingValue = GetScaling();
    if (SCALE_THRESHOLD_EIGHT < scalingValue || fabsf(SCALE_THRESHOLD_EIGHT - scalingValue) < EPSILON) {
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
    adjustSize = (TWELVE_SIZE - EIGHT_SIZE) * scalingValue;
    dragStyleNode->SetBounds(svgTouchPositionX, adjustSize, stylePixelMap_->GetWidth() + adjustSize,
        stylePixelMap_->GetHeight());
    dragStyleNode->SetFrame(svgTouchPositionX, adjustSize, stylePixelMap_->GetWidth() + adjustSize,
        stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImageWidth(stylePixelMap_->GetWidth());
    dragStyleNode->SetBgImageHeight(stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImagePositionX(0);
    dragStyleNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(stylePixelMap_);
    rosenImage->SetImageRepeat(0);
    dragStyleNode->SetBgImage(rosenImage);
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
    int32_t adjustSize = TWELVE_SIZE * GetScaling();
    pixelMapNode->SetBounds(DEFAULT_POSITION_X, adjustSize, pixelMapWidth, pixelMapHeight);
    pixelMapNode->SetFrame(DEFAULT_POSITION_X, adjustSize, pixelMapWidth, pixelMapHeight);
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
    g_drawingInfo.mouseWidth = pixelMap->GetWidth();
    g_drawingInfo.mouseHeight = pixelMap->GetHeight();
    std::shared_ptr<Rosen::RSCanvasNode> mouseIconNode = g_drawingInfo.nodes[MOUSE_ICON_INDEX];
    CHKPV(mouseIconNode);
    mouseIconNode->SetBgImageWidth(pixelMap->GetWidth());
    mouseIconNode->SetBgImageHeight(pixelMap->GetHeight());
    mouseIconNode->SetBgImagePositionX(0);
    mouseIconNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    mouseIconNode->SetBgImage(rosenImage);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawDynamicEffectModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(g_drawingInfo.parentNode);
    g_drawingInfo.parentNode->SetAlpha(alpha_->Get());
    CHKPV(scale_);
    g_drawingInfo.parentNode->SetScale(scale_->Get(), scale_->Get());
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

void DrawStyleChangeModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    CHKPV(g_drawingInfo.pixelMap);
    float pixelMapWidth = g_drawingInfo.pixelMap->GetWidth();
    if (stylePixelMap_ == nullptr) {
        if (scale_ == nullptr) {
            return;
        }
        dragStyleNode->SetScale(scale_->Get());
        return;
    }
    float scalingValue = GetScaling();
    if ((1.0 * INT_MAX / EIGHT_SIZE) <= scalingValue) {
        return;
    }
    int32_t adjustSize = EIGHT_SIZE * scalingValue;
    int32_t svgTouchPositionX = 0;
    if ((pixelMapWidth + adjustSize) > stylePixelMap_->GetWidth()) {
        svgTouchPositionX = pixelMapWidth + adjustSize - stylePixelMap_->GetWidth();
    }
    dragStyleNode->SetBounds(svgTouchPositionX, (TWELVE_SIZE-EIGHT_SIZE)*scalingValue, stylePixelMap_->GetWidth(),
        stylePixelMap_->GetHeight());
    dragStyleNode->SetFrame(svgTouchPositionX, (TWELVE_SIZE-EIGHT_SIZE)*scalingValue, stylePixelMap_->GetWidth(),
        stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImageWidth(stylePixelMap_->GetWidth());
    dragStyleNode->SetBgImageHeight(stylePixelMap_->GetHeight());
    dragStyleNode->SetBgImagePositionX(0);
    dragStyleNode->SetBgImagePositionY(0);
    auto rosenImage = std::make_shared<Rosen::RSImage>();
    rosenImage->SetPixelMap(stylePixelMap_);
    rosenImage->SetImageRepeat(0);
    dragStyleNode->SetBgImage(rosenImage);
    Rosen::RSTransaction::FlushImplicitTransaction();
}

void DrawStyleChangeModifier::SetScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (scale_ == nullptr) {
        scale_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(scale);
        Rosen::RSModifier::AttachProperty(scale_);
    } else {
        scale_->Set(scale);
    }
}

void DrawStyleScaleModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    CHKPV(scale_);
    dragStyleNode->SetScale(scale_->Get());
}

void DrawStyleScaleModifier::SetScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (scale_ == nullptr) {
        scale_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(scale);
        Rosen::RSModifier::AttachProperty(scale_);
    } else {
        scale_->Set(scale);
    }
}

void DrawDragStopModifier::Draw(Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(scale_);
    if (!CheckNodesValid()) {
        FI_HILOGE("Check nodes valid failed");
        return;
    }
    CHKPV(g_drawingInfo.parentNode);
    g_drawingInfo.parentNode->SetAlpha(alpha_->Get());
    g_drawingInfo.parentNode->SetScale(scale_->Get(), scale_->Get());
    std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode = g_drawingInfo.nodes[DRAG_STYLE_INDEX];
    CHKPV(dragStyleNode);
    dragStyleNode->SetScale(styleScale_->Get());
    dragStyleNode->SetAlpha(styleAlpha_->Get());
}

void DrawDragStopModifier::SetAlpha(float alpha)
{
    CALL_DEBUG_ENTER;
    if (alpha_ == nullptr) {
        alpha_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(alpha);
        Rosen::RSModifier::AttachProperty(alpha_);
    } else {
        alpha_->Set(alpha);
    }
}

void DrawDragStopModifier::SetScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (scale_ == nullptr) {
        scale_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(scale);
        Rosen::RSModifier::AttachProperty(scale_);
    } else {
        scale_->Set(scale);
    }
}

void DrawDragStopModifier::SetStyleScale(float scale)
{
    CALL_DEBUG_ENTER;
    if (styleScale_ == nullptr) {
        styleScale_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(scale);
        Rosen::RSModifier::AttachProperty(styleScale_);
    } else {
        styleScale_->Set(scale);
    }
}

void DrawDragStopModifier::SetStyleAlpha(float alpha)
{
    CALL_DEBUG_ENTER;
    if (styleAlpha_ == nullptr) {
        styleAlpha_ = std::make_shared<Rosen::RSAnimatableProperty<float>>(alpha);
        Rosen::RSModifier::AttachProperty(styleAlpha_);
    } else {
        styleAlpha_->Set(alpha);
    }
}
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
