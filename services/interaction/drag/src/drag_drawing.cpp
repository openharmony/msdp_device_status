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
#include "window.h"

#include "devicestatus_define.h"
#include "drag_data_adapter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
namespace {
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, MSDP_DOMAIN_ID, "DragDrawing" };
constexpr int32_t BASELINE_DENSITY = 160;
constexpr int32_t DEVICE_INDEPENDENT_PIXELS = 40;
constexpr int32_t FILE_SIZE_MAX = 0x5000;
constexpr int32_t IMAGE_WIDTH = 400;
constexpr int32_t IMAGE_HEIGHT = 500;
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
    int32_t physicalX { -1 };
    int32_t physicalY { -1 };
    int32_t rootNodeWidth { -1 };
    int32_t rootNodeHeight { -1 };
    sptr<OHOS::Rosen::Window> dragWindow { nullptr };
    std::vector<std::shared_ptr<OHOS::Rosen::RSCanvasNode>> nodes;
    std::shared_ptr<OHOS::Rosen::RSNode> rootNode { nullptr };
    std::shared_ptr<OHOS::Rosen::RSSurfaceNode> surfaceNode { nullptr };
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap { nullptr };
}dragDringInfo;
} // namespace

static bool CheckFileExtendName(const std::string &filePath, const std::string &checkExtension)
{
    std::string::size_type pos = filePath.find_last_of('.');
    if (pos == std::string::npos) {
        FI_HILOGE("File is not find extension");
        return false;
    }
    return (filePath.substr(pos + 1, filePath.npos) == checkExtension);
}

static int32_t GetFileSize(const std::string &filePath)
{
    struct stat statbuf = { 0 };
    if (stat(filePath.c_str(), &statbuf) != 0) {
        FI_HILOGE("Get file size error");
        return RET_ERR;
    }
    return statbuf.st_size;
}

static bool IsValidPath(const std::string &rootDir, const std::string &filePath)
{
    return (filePath.compare(0, rootDir.size(), rootDir) == 0);
}

static bool IsValidSvgPath(const std::string &filePath)
{
    return IsValidPath(SVG_PATH, filePath);
}

static bool IsFileExists(const std::string &fileName)
{
    return (access(fileName.c_str(), F_OK) == 0);
}

int32_t DragDrawing::InitPicture(const PictureResourse &pictureResourse, int32_t sourceType, int32_t displayId)
{
    CALL_DEBUG_ENTER;
    CHKPR(pictureResourse.pixelMap, RET_ERR);
    if ((sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) &&
      (sourceType != OHOS::MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN)) {
        FI_HILOGE("Invalid sourceType:%{public}d", sourceType);
        return RET_ERR;
    }
    dragDringInfo.pixelMap = pictureResourse.pixelMap;
    sourceType_ = sourceType;
    dragDringInfo.pixelMapX = pictureResourse.x;
    dragDringInfo.pixelMapY = pictureResourse.y;
    dragDringInfo.displayId = displayId;
    int32_t ret = InitPictureLayer(0, 0);
    if (ret != RET_OK) {
        FI_HILOGE("Draw Picture and mouse Icon failed");
        return RET_ERR;
    }
    InitAnimation();
    return RET_OK;
}

void DragDrawing::Draw(int32_t displayId, int32_t physicalX, int32_t physicalY)
{
    CALL_DEBUG_ENTER;

    if (displayId < 0) {
        FI_HILOGE("Invalid display:%{public}d", displayId);
        return;
    }
    CHKPV(rsUiDirector_);
    dragDringInfo.displayId = displayId;
    if (physicalX < 0) {
        physicalX = 0;
    }

    if (physicalY < 0) {
        physicalY = 0;
    }
    
    dragDringInfo.physicalX = physicalX;
    dragDringInfo.physicalY = physicalY;
    if (dragDringInfo.dragWindow != nullptr) {
        dragDringInfo.dragWindow->MoveTo(physicalX + dragDringInfo.pixelMapX, physicalY + dragDringInfo.pixelMapY);
        dragDringInfo.dragWindow->Show();
        return;
    }
    CreateDragWindow(physicalX + dragDringInfo.pixelMapX, physicalY + dragDringInfo.pixelMapY);
    dragDringInfo.dragWindow->Show();
}

int32_t DragDrawing::UpdateDragStyle(int32_t style)
{
    CALL_DEBUG_ENTER;
    CHKPR(rsUiDirector_, RET_ERR);
    if (style < 0) {
        FI_HILOGE("Invalid style:%{public}d", style);
        return RET_ERR;
    }
    if (dragDringInfo.currentStyle == style) {
        FI_HILOGD("No need update drag style");
        return RET_OK;
    }
    dragDringInfo.currentStyle = style;
    DrawStyle();
    rsUiDirector_->SendMessages();
    return RET_OK;
}

void DragDrawing::DestroyPointerWindow()
{
    CALL_DEBUG_ENTER;
    sourceType_ = -1;
    dragDringInfo.currentStyle = -1;
    startNum_ = 181154000809;
    if (dragDringInfo.pixelMap != nullptr) {
        dragDringInfo.pixelMap = nullptr;
    }
    if (dragDringInfo.rootNode != nullptr) {
        dragDringInfo.rootNode->ClearChildren();
        dragDringInfo.rootNode = nullptr;
    }
    if (rsUiDirector_ != nullptr) {
        rsUiDirector_ = nullptr;
    }
    if (dragDringInfo.dragWindow != nullptr) {
        dragDringInfo.dragWindow->Hide();
        dragDringInfo.dragWindow->Destroy();
        dragDringInfo.dragWindow = nullptr;
    }
}
void DragDrawing::InitAnimation()
{
    CALL_DEBUG_ENTER;
    runner_ = AppExecFwk::EventRunner::Create(false);
    handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
    handler_->PostTask(std::bind(&DragDrawing::InitVSync, this), 800);
}

void DragDrawing::OnDragSuccess()
{
    CALL_DEBUG_ENTER;
    CHKPV(runner_);
    CHKPV(rsUiDirector_);
    CHKPV(dragDringInfo.rootNode);
    if (drawDynamicEffectModifier_ == nullptr){
        drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    }
    dragDringInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(1);
    drawDynamicEffectModifier_->SetScale(1);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(1000);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(0);
        drawDynamicEffectModifier_->SetScale(10);
    });
    runner_->Run();
}

void DragDrawing::OnDragFail()
{
    CALL_DEBUG_ENTER;
    CHKPV(runner_);
    CHKPV(rsUiDirector_);
    CHKPV(dragDringInfo.rootNode);
    if (drawDynamicEffectModifier_ == nullptr){
        drawDynamicEffectModifier_ = std::make_shared<DrawDynamicEffectModifier>();
    }
    dragDringInfo.rootNode->AddModifier(drawDynamicEffectModifier_);
    drawDynamicEffectModifier_->SetAlpha(1);
    drawDynamicEffectModifier_->SetScale(1);

    OHOS::Rosen::RSAnimationTimingProtocol protocol;
    protocol.SetDuration(1000);
    OHOS::Rosen::RSNode::Animate(protocol, OHOS::Rosen::RSAnimationTimingCurve::EASE_IN_OUT, [&]() {
        drawDynamicEffectModifier_->SetAlpha(0);
        drawDynamicEffectModifier_->SetScale(0);
    });
    runner_->Run();
}

void DragDrawing::EraseMouseIcon()
{
    CALL_DEBUG_ENTER;
    CHKPV(dragDringInfo.rootNode);
    if (sourceType_ != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGE("Touch type not need erase mouse icon");
        return;
    }
    dragDringInfo.rootNode->RemoveChild(dragDringInfo.nodes[2]);
    rsUiDirector_->SendMessages();
}

void DragDrawing::DrawPicture()
{
    CALL_DEBUG_ENTER;
    if (drawPixelMapModifier_ != nullptr){
        dragDringInfo.nodes[0]->RemoveModifier(drawPixelMapModifier_);
    }
    drawPixelMapModifier_ = std::make_shared<DrawPixelMapModifier>();
    dragDringInfo.nodes[0]->AddModifier(drawPixelMapModifier_);
}

void DragDrawing::DrawStyle()
{
    CALL_DEBUG_ENTER;
    if (drawSVGModifier_ != nullptr){
        dragDringInfo.nodes[1]->RemoveModifier(drawSVGModifier_);
    }
    auto drawSVGModifier_ = std::make_shared<DrawSVGModifier>();
    dragDringInfo.nodes[1]->AddModifier(drawSVGModifier_);
} 

void DragDrawing::DrawMouseIcon()
{
    CALL_DEBUG_ENTER;
    if (sourceType_ != OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        FI_HILOGD("Touch type not need draw mouse icon");
        return;
    }
    if (drawMouseIconModifier_ != nullptr){
        dragDringInfo.nodes[2]->RemoveModifier(drawMouseIconModifier_);
    }
    drawMouseIconModifier_ = std::make_shared<DrawMouseIconModifier>();
    dragDringInfo.nodes[2]->AddModifier(drawMouseIconModifier_);
    return;
}

int32_t DragDrawing::InitVSync()
{
    CALL_DEBUG_ENTER;
    CHKPR(handler_, RET_ERR);
    CHKPR(runner_, RET_ERR);
    auto rsClient = std::static_pointer_cast<OHOS::Rosen::RSRenderServiceClient>(
        OHOS::Rosen::RSIRenderClient::CreateRenderServiceClient());
    if (receiver_ == nullptr) {
        receiver_ = rsClient->CreateVSyncReceiver("DragDrawing", handler_);
    }
    auto ret = receiver_->Init();
    if (ret) {
        FI_HILOGE("Receiver init failed");
        runner_->Stop();
        return RET_ERR;
    }

    int32_t freq = 30; //30 或者 60
    OHOS::Rosen::VSyncReceiver::FrameCallback fcb = {
        .userData_ = this,
        .callback_ = std::bind(&DragDrawing::OnVsync, this),
    };
    int32_t changefreq = static_cast<int32_t>((1000.0 / freq) / 16);
    ret = receiver_->SetVSyncRate(fcb, changefreq);
    if (ret) {
        runner_->Stop();
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
    FI_HILOGE("XJP OnVsync: %{public}d", hasRunningAnimation);
    if (!hasRunningAnimation) {
        FI_HILOGE("XJP stop runner_, OnVsync: %{public}d", hasRunningAnimation);
        runner_->Stop();
        return;
    }
    rsUiDirector_->SendMessages();
    startNum_ += 16666667;
}

int32_t DragDrawing::InitLayer()
{
    CALL_DEBUG_ENTER;
    if (dragDringInfo.dragWindow == nullptr) {
        FI_HILOGE("dragWindow is nullptr");
        return RET_ERR;
    }
    dragDringInfo.surfaceNode = dragDringInfo.dragWindow->GetSurfaceNode();
    if (dragDringInfo.surfaceNode == nullptr) {
        dragDringInfo.dragWindow->Destroy();
        dragDringInfo.dragWindow = nullptr;
        FI_HILOGE("Draw drag window failed, get surface node is nullptr");
        return RET_ERR;
    }
    auto surface = dragDringInfo.surfaceNode->GetSurface();
    if (surface == nullptr) {
        dragDringInfo.dragWindow->Destroy();
        dragDringInfo.dragWindow = nullptr;
        FI_HILOGE("Draw drag window failed, get surface is nullptr");
        return RET_ERR;
    }
    rsUiDirector_ = OHOS::Rosen::RSUIDirector::Create();
    rsUiDirector_->Init();
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    rsUiDirector_->SetRSSurfaceNode(dragDringInfo.surfaceNode);
    InitCanvas(IMAGE_WIDTH, IMAGE_HEIGHT);
    return RET_OK;
}

void DragDrawing::InitCanvas(int32_t width, int32_t height)
{
    CALL_DEBUG_ENTER;
    if (dragDringInfo.rootNode == nullptr) {
        dragDringInfo.rootNode = OHOS::Rosen::RSRootNode::Create();
    }
    dragDringInfo.rootNode->SetBounds(dragDringInfo.physicalX, dragDringInfo.physicalY, width, height);
    dragDringInfo.rootNode->SetFrame(dragDringInfo.physicalX, dragDringInfo.physicalY, width, height);
    dragDringInfo.rootNode->SetBackgroundColor(SK_ColorTRANSPARENT);

    dragDringInfo.nodes.emplace_back(OHOS::Rosen::RSCanvasNode::Create());
    dragDringInfo.nodes[0]->SetBounds(0, 40, dragDringInfo.pixelMap->GetWidth(), dragDringInfo.pixelMap->GetHeight());
    dragDringInfo.nodes[0]->SetFrame(0, 40, dragDringInfo.pixelMap->GetWidth(), dragDringInfo.pixelMap->GetHeight());


    dragDringInfo.nodes.emplace_back(OHOS::Rosen::RSCanvasNode::Create());
    dragDringInfo.nodes[1]->SetBounds(0, 0, 40, 40);
    dragDringInfo.nodes[1]->SetFrame(0, 0, 40, 40);

    if (sourceType_ == OHOS::MMI::PointerEvent::SOURCE_TYPE_MOUSE) {
        dragDringInfo.nodes.emplace_back(OHOS::Rosen::RSCanvasNode::Create());
        dragDringInfo.nodes[2]->SetBounds(0 - dragDringInfo.pixelMapX, 0 - dragDringInfo.pixelMapY, 40, 40);
        dragDringInfo.nodes[2]->SetFrame(0 - dragDringInfo.pixelMapX, 0 - dragDringInfo.pixelMapY, 40, 40);
        dragDringInfo.rootNode->AddChild(dragDringInfo.nodes[0], -1);
        dragDringInfo.rootNode->AddChild(dragDringInfo.nodes[1], -1);
        dragDringInfo.rootNode->AddChild(dragDringInfo.nodes[2], -1);
        rsUiDirector_->SetRoot(dragDringInfo.rootNode->GetId());
        return;
    }
    dragDringInfo.rootNode->AddChild(dragDringInfo.nodes[0], -1);
    dragDringInfo.rootNode->AddChild(dragDringInfo.nodes[1], -1);
    rsUiDirector_->SetRoot(dragDringInfo.rootNode->GetId());
}

int32_t DragDrawing::InitPictureLayer(int32_t x, int32_t y)
{
    CALL_DEBUG_ENTER;
    CreateDragWindow(0, 0);
    int32_t ret = InitLayer();
    if (ret != RET_OK) {
        FI_HILOGE("Init layer failed");
        return RET_ERR;
    }
    DrawPicture();
    DrawMouseIcon();
    return RET_OK;
}

void DragDrawing::CreateDragWindow(int32_t physicalX, int32_t physicalY)
{
    CALL_DEBUG_ENTER;
    sptr<OHOS::Rosen::WindowOption> option = new (std::nothrow) OHOS::Rosen::WindowOption();
    CHKPV(option);
    option->SetWindowType(OHOS::Rosen::WindowType::WINDOW_TYPE_POINTER);
    option->SetWindowMode(OHOS::Rosen::WindowMode::WINDOW_MODE_FLOATING);
    OHOS::Rosen::Rect rect = {
        .posX_ = physicalX,
        .posY_ = physicalY,
        .width_ = IMAGE_WIDTH,
        .height_ = IMAGE_HEIGHT,
    };
    option->SetWindowRect(rect);
    option->SetFocusable(false);
    option->SetTouchable(true);
    std::string windowName = "drag window";
    dragDringInfo.dragWindow = OHOS::Rosen::Window::Create(windowName, option, nullptr);
    
}

void DrawSVGModifier::Draw(OHOS::Rosen::RSDrawingContext& context) const
{
    CALL_DEBUG_ENTER;
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    std::string filePath = "";
    if (dragDringInfo.currentStyle == 0) {
        filePath = FORBID_DRAG_PATH;
    }
    else if (dragDringInfo.currentStyle == 1) {
        filePath = COPY_ONE_DRAG_PATH;
    }
    else {
        filePath = COPY_DRAG_PATH;
    }
    
    if (!IsValidSvgFile(filePath)) {
        FI_HILOGE("Svg file is valid");
        return;
    }
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap = DecodeSvgToPixelMap(filePath);
    CHKPV(pixelMap);
    auto rosenImage = std::make_shared<OHOS::Rosen::RSImage>();
    rosenImage->SetPixelMap(pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t svgTouchPositionX = dragDringInfo.pixelMap->GetWidth() + 8 - pixelMap->GetWidth();
    dragDringInfo.nodes[0]->SetBounds(0, 8, dragDringInfo.pixelMap->GetWidth(), dragDringInfo.pixelMap->GetHeight());
    dragDringInfo.nodes[0]->SetFrame(0, 8, dragDringInfo.pixelMap->GetWidth(), dragDringInfo.pixelMap->GetHeight());
    dragDringInfo.nodes[1]->SetBounds(svgTouchPositionX, 0, pixelMap->GetWidth(), pixelMap->GetHeight());
    dragDringInfo.nodes[1]->SetFrame(svgTouchPositionX, 0, pixelMap->GetWidth(), pixelMap->GetHeight());
    dragDringInfo.nodes[1]->SetBgImageWidth(pixelMap->GetWidth());
    dragDringInfo.nodes[1]->SetBgImageHeight(40);
    dragDringInfo.nodes[1]->SetBgImagePositionX(0);
    dragDringInfo.nodes[1]->SetBgImagePositionY(0);
    dragDringInfo.nodes[1]->SetBgImage(rosenImage);
    dragDringInfo.rootNodeWidth = dragDringInfo.pixelMap->GetWidth() + 8;
    dragDringInfo.rootNodeHeight = pixelMap->GetHeight() + dragDringInfo.pixelMap->GetHeight() + 8;
    dragDringInfo.rootNode->SetBounds(0, 0, dragDringInfo.rootNodeWidth, dragDringInfo.rootNodeHeight);
    dragDringInfo.rootNode->SetFrame(0, 0, dragDringInfo.rootNodeWidth, dragDringInfo.rootNodeHeight);
    dragDringInfo.dragWindow->Resize(dragDringInfo.rootNodeWidth, dragDringInfo.rootNodeHeight);
}

bool DrawSVGModifier::IsValidSvgFile(const std::string &filePath) const
{
    CALL_DEBUG_ENTER;
    if (filePath.empty()) {
        FI_HILOGE("FilePath is empty");
        return false;
    }
    char realPath[PATH_MAX] = {};
    if (realpath(filePath.c_str(), realPath) == nullptr) {
        FI_HILOGE("The realpath return nullptr, realPath:%{public}s", realPath);
        return false;
    }
    if (!IsValidSvgPath(realPath)) {
        FI_HILOGE("File path is error");
        return false;
    }
    if (!IsFileExists(realPath)) {
        FI_HILOGE("File is not existent");
        return false;
    }
    if (!CheckFileExtendName(realPath, "svg")) {
        FI_HILOGE("Unable to parse files other than json format");
        return false;
    }
    int32_t fileSize = GetFileSize(realPath);
    if ((fileSize <= 0) || (fileSize > FILE_SIZE_MAX)) {
        FI_HILOGE("File size out of read range");
        return false;
    }
    return true;
}

int32_t DrawSVGModifier::UpdateSvgNodeInfo(xmlNodePtr &curNode, int32_t strSize) const
{
    CALL_DEBUG_ENTER;
    if(xmlStrcmp(curNode->name, BAD_CAST "svg")) {
        FI_HILOGE("Svg file format invalid");
        return RET_ERR;
    }
    int32_t number = 0;
    std::string srcSvgWidth = (char *)xmlGetProp(curNode, BAD_CAST "width");
    number = std::stoi(srcSvgWidth) + strSize;
    std::string tgtSvgWidth  = std::to_string(number);
    xmlSetProp(curNode, BAD_CAST "width", BAD_CAST tgtSvgWidth.c_str());
    std::string srcViewBox = (char *)xmlGetProp(curNode, BAD_CAST "viewBox");
    std::istringstream strSteam(srcViewBox);
    std::string word;
    std::string tgtViewBox;
    int32_t i = 0;
    int32_t size = srcViewBox.size();
    while ((strSteam >> word) && (i < size)) {
        if (i == 2) {
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
    //寻找到最深的父节点
    while (curNode != NULL) {
        if(!xmlStrcmp(curNode->name, BAD_CAST "g")) {
            while(!xmlStrcmp(curNode->name, BAD_CAST "g")) {
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
        if(!xmlStrcmp(curNode->name, BAD_CAST "rect")) {
            std::string srcRectWidth = (char *)xmlGetProp(curNode, BAD_CAST "width");
            number = std::stoi(srcRectWidth) + strSize;
            std::string tgtRectWidth = std::to_string(number);
            xmlSetProp(curNode,BAD_CAST "width", BAD_CAST tgtRectWidth.c_str());
        }
        if(!xmlStrcmp(curNode->name, BAD_CAST "text")) {
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
        if(!xmlStrcmp(curNode->name, BAD_CAST "tspan")) {
            std::string tgtTspanValue = std::to_string(dragDringInfo.currentStyle);
            xmlNodeSetContent(curNode, BAD_CAST tgtTspanValue.c_str());
        }
        curNode = curNode->next;
    }
}

int32_t DrawSVGModifier::ParseAndAdjustSvgInfo(xmlNodePtr &curNode) const
{
    CALL_DEBUG_ENTER;
    std::string strStyle = std::to_string(dragDringInfo.currentStyle);
    int32_t strSize = (strStyle.size() - 1) * 8;
    xmlKeepBlanksDefault(0);
    //svg节点处理
    int32_t ret = UpdateSvgNodeInfo(curNode, strSize);
    if (ret != RET_OK) {
        FI_HILOGE("Update svg node info failed");
        return RET_ERR;
    }
    //寻找rect的同层节点
    curNode = FindRectNode(curNode);
    //rect节点处理
    curNode = UpdateRectNode(curNode, strSize);
    //tspan节点处理
    UpdateTspanNode(curNode);
    return RET_OK;
}

std::shared_ptr<OHOS::Media::PixelMap> DrawSVGModifier::DecodeSvgToPixelMap(
    const std::string &filePath) const
{
    CALL_DEBUG_ENTER;
    xmlDocPtr xmlDoc = xmlReadFile(filePath.c_str(), 0, XML_PARSE_NOBLANKS);
    if (dragDringInfo.currentStyle != 0) {
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
    std::string content = (char *)xmlbuff;
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
    if (dragDringInfo.pixelMap == nullptr) {
        FI_HILOGE("DragDringInfo.pixelMap is nullptr");
        return;
    }
    rosenImage->SetPixelMap(dragDringInfo.pixelMap);
    rosenImage->SetImageRepeat(0);
    int32_t pixelMapWidth = dragDringInfo.pixelMap->GetWidth();
    int32_t pixelMapHeight = dragDringInfo.pixelMap->GetHeight();
    dragDringInfo.nodes[0]->SetBoundsWidth(pixelMapWidth);
    dragDringInfo.nodes[0]->SetBoundsHeight(pixelMapHeight);
    dragDringInfo.nodes[0]->SetBgImageWidth(pixelMapWidth);
    dragDringInfo.nodes[0]->SetBgImageHeight(pixelMapHeight);
    dragDringInfo.nodes[0]->SetBgImagePositionX(0);
    dragDringInfo.nodes[0]->SetBgImagePositionY(0);
    dragDringInfo.nodes[0]->SetBgImage(rosenImage);
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
    //DisplayById需要获取
    auto displayInfo = OHOS::Rosen::DisplayManager::GetInstance().GetDisplayById(dragDringInfo.displayId);
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
    dragDringInfo.nodes[2]->SetBoundsWidth(decodeOpts.desiredSize.width);
    dragDringInfo.nodes[2]->SetBoundsHeight(decodeOpts.desiredSize.height);
    dragDringInfo.nodes[2]->SetBgImageWidth(decodeOpts.desiredSize.width);
    dragDringInfo.nodes[2]->SetBgImageHeight(decodeOpts.desiredSize.height);
    dragDringInfo.nodes[2]->SetBgImagePositionX(0);
    dragDringInfo.nodes[2]->SetBgImagePositionY(0);
    dragDringInfo.nodes[2]->SetBgImage(rosenImage);
}

void DrawDynamicEffectModifier::Draw(OHOS::Rosen::RSDrawingContext &context) const
{
    CALL_DEBUG_ENTER;
    CHKPV(alpha_);
    CHKPV(scale_);
    auto rsSurface = OHOS::Rosen::RSSurfaceExtractor::ExtractRSSurface(dragDringInfo.surfaceNode);
    if (rsSurface == nullptr) {
        return;
    }
    dragDringInfo.surfaceNode->SetScale(scale_->Get(), scale_->Get());
    //焦点坐标待确认
    dragDringInfo.surfaceNode->SetPivot(0.5f, 0.5f);
    dragDringInfo.rootNode->SetAlpha(alpha_->Get());
    OHOS::Rosen::RSTransaction::FlushImplicitTransaction();
    auto frame = rsSurface->RequestFrame(dragDringInfo.rootNodeWidth, dragDringInfo.rootNodeHeight);
    if (frame == nullptr) {
        FI_HILOGE("Failed to create frame");
        return;
    }
    FI_HILOGE("XJP, alpha_:%{public}f, scale_:%{public}f", alpha_->Get(), scale_->Get());
    frame->SetDamageRegion(0, 0, dragDringInfo.rootNodeWidth, dragDringInfo.rootNodeHeight);
    rsSurface->FlushFrame(frame);
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