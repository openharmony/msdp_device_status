/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "coordination_message.h"
#include "drag_data.h"
#include "i_coordination_listener.h"
#include "nocopyable.h"

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
     * @brief 开启/关闭键鼠穿越管理接口。
     * @param enabled 开启/关闭。
     * @param callback 开启/关闭键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t EnableCoordination(bool enabled, std::function<void(std::string, CoordinationMessage)> callback);

    /**
     * @brief 启动跨设备键鼠穿越。
     * @param sinkDeviceId 键鼠穿越目标设备描述符（networkID）
     * @param srcDeviceId 键鼠穿越待穿越输入外设标识符（设备ID句柄）
     * @param callback 启动跨设备键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t StartCoordination(const std::string &sinkDeviceId, int32_t srcDeviceId,
        std::function<void(std::string, CoordinationMessage)> callback);

    /**
     * @brief 停止跨设备键鼠穿越。
     * @param callback 停止跨设备键鼠穿越，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t StopCoordination(std::function<void(std::string, CoordinationMessage)> callback);

    /**
     * @brief 获取指定设备键鼠穿越状态。
     * @param deviceId 指定设备描述符。
     * @param callback 获取穿越管理设备状态，此回调被调用
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t GetCoordinationState(const std::string &deviceId, std::function<void(bool)> callback);

    /**
     * @brief 开始拖拽接口。
     * @param dragData 拖拽传入数据
     * @param dragData 拖拽结束回调
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback);

    /**
     * @brief 结束拖拽接口。
     * @param dragResult 传出参数，标识拖拽调用结果 0-成功,1-失败,2-取消
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 9
     */
    int32_t StopDrag(int32_t &dragResult);
    
    /**
     * @brief 更新拖拽中的光标样式。
     * @param style 指定光标样式。
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 10
     */
    int32_t UpdateDragStyle(int32_t style);

    /**
     * @brief 更新拖拽中的角标文本信息。
     * @param message 角标文本信息。
     * @return 返回值如果是0表示接口调用成功，返回其他值表示接口调用失败。
     * @since 10
     */
    int32_t UpdateDragMessage(const std::u16string &message);

    /**
     * @brief 获取拖拽目标窗口PID
     * @return 返回值如果是-1则是无效值，为大于等于0的值为正确值
     * @since 10
     */
    int32_t GetDragTargetPid();

private:
    InteractionManager() = default;
    DISALLOW_COPY_AND_MOVE(InteractionManager);
    static InteractionManager *instance_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS

#define InteractionMgr OHOS::Msdp::DeviceStatus::InteractionManager::GetInstance()

#endif // INTERACTION_MANAGER_H
