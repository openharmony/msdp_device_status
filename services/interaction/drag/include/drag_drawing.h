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

#ifndef DRAG_DRAWING_H
#define DRAG_DRAWING_H

#include <vector>

#include "event_handler.h"
#include "event_runner.h"
#include "libxml/tree.h"
#include "libxml/parser.h"
#include "modifier/rs_extended_modifier.h"
#include "modifier/rs_modifier.h"

#include "vsync_receiver.h"
#include "drag_data.h"
#include "i_drag_animation.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
struct DrawingInfo;
class DragDrawing;
using DragExtFunc = void (*)(DragDrawing *drawingObj, DrawingInfo *drawingInfo);
class DrawSVGModifier : public Rosen::RSContentStyleModifier {
public:
    explicit DrawSVGModifier(std::shared_ptr<Media::PixelMap> stylePixelMap) : stylePixelMap_(stylePixelMap) {}
    ~DrawSVGModifier() = default;
    void Draw(Rosen::RSDrawingContext& context) const override;

private:
    std::shared_ptr<Media::PixelMap> stylePixelMap_ { nullptr };
};

class DrawPixelMapModifier : public Rosen::RSContentStyleModifier {
public:
    DrawPixelMapModifier() = default;
    ~DrawPixelMapModifier() = default;
    void Draw(Rosen::RSDrawingContext &context) const override;
};

class DrawMouseIconModifier : public Rosen::RSContentStyleModifier {
public:
    DrawMouseIconModifier() = default;
    ~DrawMouseIconModifier() = default;
    void Draw(Rosen::RSDrawingContext &context) const override;

private:
    void OnDraw(std::shared_ptr<Media::PixelMap> pixelMap, int32_t pointerStyleId) const;
};

class DrawDynamicEffectModifier : public Rosen::RSContentStyleModifier {
public:
    DrawDynamicEffectModifier() = default;
    ~DrawDynamicEffectModifier() = default;
    void Draw(Rosen::RSDrawingContext &context) const override;
    void SetAlpha(float alpha);
    void SetScale(float scale);

private:
    std::shared_ptr<Rosen::RSAnimatableProperty<float>> alpha_ { nullptr };
    std::shared_ptr<Rosen::RSAnimatableProperty<float>> scale_ { nullptr };
};

struct DrawingInfo {
    std::atomic_bool isRunning { false };
    std::atomic_bool isPreviousDefaultStyle { false };
    std::atomic_bool isCurrentDefaultStyle { false };
    bool isInitUiDirector { true };
    bool isExistScalingVallue { false };
    int32_t sourceType { -1 };
    int32_t currentDragNum { -1 };
    DragCursorStyle currentStyle { DragCursorStyle::DEFAULT };
    int32_t displayId { -1 };
    int32_t pixelMapX { -1 };
    int32_t pixelMapY { -1 };
    int32_t displayX { -1 };
    int32_t displayY { -1 };
    int32_t mouseWidth { 0 };
    int32_t mouseHeight { 0 };
    int32_t rootNodeWidth { -1 };
    int32_t rootNodeHeight { -1 };
    float scalingValue { 0.0 };
    std::vector<std::shared_ptr<Rosen::RSCanvasNode>> nodes;
    std::vector<std::shared_ptr<Rosen::RSCanvasNode>> mutilSelectedNodes;
    std::vector<std::shared_ptr<Media::PixelMap>> mutilSelectedPixelMaps;
    std::shared_ptr<Rosen::RSNode> rootNode { nullptr };
    std::shared_ptr<Rosen::RSNode> parentNode { nullptr };
    std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode { nullptr };
    std::shared_ptr<Media::PixelMap> pixelMap { nullptr };
    std::shared_ptr<Media::PixelMap> stylePixelMap { nullptr };
    std::string extraInfo;
    std::string filterInfo;
};

struct FilterInfo {
    std::string componentType;
    int32_t blurStyle;
    int32_t cornerRadius;
    float dipScale;
};

class DragDrawing : public IDragAnimation {
public:
    DragDrawing() = default;
    DISALLOW_COPY_AND_MOVE(DragDrawing);
    ~DragDrawing();

    int32_t Init(const DragData &dragData);
    void Draw(int32_t displayId, int32_t displayX, int32_t displayY);
    int32_t UpdateDragStyle(DragCursorStyle style);
    int32_t UpdateShadowPic(const ShadowInfo &shadowInfo);
    int32_t UpdatePreviewStyle(const PreviewStyle &previewStyle);
    int32_t UpdatePreviewStyleWithAnimation(const PreviewStyle &previewStyle, const PreviewAnimation &animation);
    int32_t StartVsync();
    void OnDragSuccess();
    void OnDragFail();
    void EraseMouseIcon();
    void DestroyDragWindow();
    void UpdateDrawingState();
    void UpdateDragWindowState(bool visible);
    void OnStartDrag(const DragAnimationData &dragAnimationData, std::shared_ptr<Rosen::RSCanvasNode> shadowNode,
        std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode) override;
    void OnDragStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode,
        std::shared_ptr<Media::PixelMap> stylePixelMap) override;
    void OnStopDragSuccess(std::shared_ptr<Rosen::RSCanvasNode> shadowNode,
        std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode) override;
    void OnStopDragFail(std::shared_ptr<Rosen::RSSurfaceNode> surfaceNode,
        std::shared_ptr<Rosen::RSNode> rootNode) override;
    void OnStopAnimation() override;
    void SetTextEditorAreaFlag(bool textEditorAreaFlag);
    int32_t EnterTextEditorArea(bool enable);
    bool GetAllowDragState();
    void SetScreenId(uint64_t screenId);

private:
    int32_t CheckDragData(const DragData &dragData);
    int32_t InitLayer();
    void InitCanvas(int32_t width, int32_t height);
    void CreateWindow(int32_t displayX, int32_t displayY);
    int32_t DrawShadow(std::shared_ptr<Rosen::RSCanvasNode> shadowNode);
    int32_t DrawMouseIcon();
    int32_t DrawStyle(std::shared_ptr<Rosen::RSCanvasNode> dragStyleNode,
        std::shared_ptr<Media::PixelMap> stylePixelMap);
    void RunAnimation(float endAlpha, float endScale);
    int32_t InitVSync(float endAlpha, float endScale);
    void OnVsync();
    void InitDrawingInfo(const DragData &dragData);
    int32_t InitDragAnimationData(DragAnimationData &dragAnimationData);
    void RemoveModifier();
    int32_t UpdateSvgNodeInfo(xmlNodePtr curNode, int32_t extendSvgWidth);
    xmlNodePtr GetRectNode(xmlNodePtr curNode);
    xmlNodePtr UpdateRectNode(int32_t extendSvgWidth, xmlNodePtr curNode);
    void UpdateTspanNode(xmlNodePtr curNode);
    int32_t ParseAndAdjustSvgInfo(xmlNodePtr curNode);
    std::shared_ptr<Media::PixelMap> DecodeSvgToPixelMap(const std::string &filePath);
    int32_t GetFilePath(std::string &filePath);
    bool NeedAdjustSvgInfo();
    void SetDecodeOptions(Media::DecodeOptions &decodeOpts);
    bool ParserFilterInfo(FilterInfo &filterInfo);
    void ProcessFilter();
    static float RadiusVp2Sigma(float radiusVp, float dipScale);
    void DoDrawMouse();
    int32_t UpdateDefaultDragStyle(DragCursorStyle style);
    int32_t UpdateValidDragStyle(DragCursorStyle style);
    int32_t SetNodesLocation(int32_t positionX, int32_t positionY);
    int32_t CreateEventRunner(int32_t positionX, int32_t positionY);
    int32_t ModifyPreviewStyle(std::shared_ptr<Rosen::RSCanvasNode> node, const PreviewStyle &previewStyle);
    void MutilSelectedAnimation(int32_t positionX, int32_t positionY, int32_t adjustSize);
    void InitMutilSelectedNodes();
    void ClearMutilSelectedData();

private:
    int64_t startNum_ { -1 };
    std::shared_ptr<Rosen::RSCanvasNode> canvasNode_ { nullptr };
    std::shared_ptr<DrawSVGModifier> drawSVGModifier_ { nullptr };
    std::shared_ptr<DrawPixelMapModifier> drawPixelMapModifier_ { nullptr };
    std::shared_ptr<DrawMouseIconModifier> drawMouseIconModifier_ { nullptr };
    std::shared_ptr<DrawDynamicEffectModifier> drawDynamicEffectModifier_ { nullptr };
    std::shared_ptr<Rosen::RSUIDirector> rsUiDirector_ { nullptr };
    std::shared_ptr<Rosen::VSyncReceiver> receiver_ { nullptr };
    std::shared_ptr<AppExecFwk::EventHandler> handler_ { nullptr };
    void* dragExtHandle_ { nullptr };
    bool needDestroyDragWindow_ { false };
    uint64_t screenId_ { 0 };
    bool textEditorAreaFlag_ { false };
    int32_t resetPixelMapX_ { 0 };
    int32_t resetPixelMapY_ { 0 };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_DRAWING_H