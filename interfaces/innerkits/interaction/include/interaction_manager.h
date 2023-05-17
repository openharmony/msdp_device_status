/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef INTERACTION_MANAGER_H
#define INTERACTION_MANAGER_H

#include <functional>
#include <memory>

#include "nocopyable.h"

#include "coordination_message.h"
#include "drag_data.h"
#include "i_coordination_listener.h"
#include "i_drag_listener.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class InteractionManager {
public:

    static InteractionManager *GetInstance();
    virtual ~InteractionManager() = default;

    /**
     * @brief 注册键鼠穿越管理事件监听。
     * @param listener 穿越管理事件监听回调。
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t RegisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener);

    /**
     * @brief 注销键鼠穿越管理事件监听。
     * @param listener 事件监听回调.
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t UnregisterCoordinationListener(std::shared_ptr<ICoordinationListener> listener = nullptr);

    /**
     * @brief 准备键鼠穿越接口。
     * @param callback 开启/关闭键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t PrepareCoordination(std::function<void(const std::string&, CoordinationMessage)> callback);

    /**
     * @brief 取消准备键鼠穿越接口。
     * @param callback 开启/关闭键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t UnprepareCoordination(std::function<void(const std::string&, CoordinationMessage)> callback);

    /**
     * @brief 启动跨设备键鼠穿越。
     * @param remoteNetworkId 键鼠穿越目标设备描述符（networkID）
     * @param startDeviceId 键鼠穿越待穿越输入外设标识符（设备ID句柄）
     * @param callback 启动跨设备键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t ActivateCoordination(const std::string &remoteNetworkId, int32_t startDeviceId,
        std::function<void(const std::string&, CoordinationMessage)> callback);

    /**
     * @brief 停止跨设备键鼠穿越。
     * @param isUnchained 跨设备连接是否断开。
     * @param callback 停止跨设备键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t DeactivateCoordination(std::function<void(const std::string&, CoordinationMessage)> callback);

    /**
     * @brief 获取指定设备键鼠穿越状态。
     * @param deviceId 指定设备描述符。
     * @param callback 获取穿越管理设备状态，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t GetCoordinationState(const std::string &deviceId, std::function<void(bool)> callback);

    /**
     * @brief 开始拖拽目标。
     * @param dragData 拖拽附加数据
     * @param callback 拖拽结果信息回调函数
     * @return 返回0表示调用成功，否则，表示调用失败
     * @since 10
     */
    int32_t StartDrag(const DragData &dragData, std::function<void(const DragNotifyMsg&)> callback);

    /**
     * @brief 结束拖拽。
     * @param result 标识拖拽调用结果 0-成功,1-失败,2-取消
     * @param hasCustomAnimation 标识是否在拖拽成功时做默认动效，true表示做应用自定义动效，false表示做默认动效
     * @return 返回0表示调用成功，否则，表示调用失败
     * @since 10
     */
    int32_t StopDrag(DragResult result, bool hasCustomAnimation);

    /**
     * @brief 更新拖拽中的光标样式。
     * @param style 指定光标样式。
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 10
     */
    int32_t UpdateDragStyle(DragCursorStyle style);

    /**
     * @brief 获取拖拽目标窗口PID
     * @return 返回值如果是-1则是无效值，为大于等于0的值为正确值
     * @since 10
     */
    int32_t GetDragTargetPid();

    /**
     * @brief 获取拖拽目标窗口统一数据密钥
     * @param UdKey 拖拽目标窗口统一数据密钥。
     * @return 返回值如果是-1则是无效值，为大于等于0的值为正确值
     * @since 10
     */
    int32_t GetUdKey(std::string &udKey);

    /**
     * @brief 注册拖拽状态监听。。
     * @param listener 拖拽状态监听。
     * @return 返回值0表示接口调用成功，否则，表示接口调用失败。
     * @since 10
     */
    int32_t AddDraglistener(std::shared_ptr<IDragListener> listener);

    /**
     * @brief 取消注册拖拽状态监听。
     * @param listener 拖拽状态监听，如果为空，表示取消所有监听。
     * @return 返返回值0表示接口调用成功，否则，表示接口调用失败。
     * @since 10
     */
    int32_t RemoveDraglistener(std::shared_ptr<IDragListener> listener = nullptr);

    /**
     * @brief 设置拖拽窗口显示或者隐藏
     * @param visible 设置拖拽窗口的是否显示，true表示显示，false表示隐藏。
     * @return 返回值0表示接口调用成功，否则，表示接口调用失败。
     * @since 10
     */
    int32_t SetDragWindowVisible(bool visible);

    /**
     * @brief 获取触控点或鼠标光标相对于阴影缩略图左上角的位置。
     * @param offsetX 要查询的x值。
     * @param offsetY 要查询的y值。
     * @param width 要查询缩略图的宽
     * @param height 要查询缩略图的高
     * @return 返回值0表示接口调用成功，否则，表示接口调用失败。
     * @since 10
     */
    int32_t GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height);

private:
    InteractionManager() = default;
    DISALLOW_COPY_AND_MOVE(InteractionManager);
    static InteractionManager *instance_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#define INTERACTION_MGR OHOS::Msdp::DeviceStatus::InteractionManager::GetInstance()

#endif // INTERACTION_MANAGER_H
