/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DRAG_MANAGER_TEST_H
#define DRAG_MANAGER_TEST_H

#include <gtest/gtest.h>

#include "delegate_tasks.h"
#include "device_manager.h"
#include "devicestatus_define.h"
#include "devicestatus_delayed_sp_singleton.h"
#include "drag_client.h"
#include "drag_manager.h"
#include "i_context.h"
#include "socket_session_manager.h"
#include "timer_manager.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class ContextService final : public IContext {
    ContextService();
    ~ContextService();
    DISALLOW_COPY_AND_MOVE(ContextService);
public:
    IDelegateTasks& GetDelegateTasks() override;
    IDeviceManager& GetDeviceManager() override;
    ITimerManager& GetTimerManager() override;
    IDragManager& GetDragManager() override;

#ifdef OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
    IPluginManager& GetPluginManager() override;
    ISocketSessionManager& GetSocketSessionManager() override;
    IInputAdapter& GetInput() override;
    IDSoftbusAdapter& GetDSoftbus() override;
    IDDPAdapter& GetDP() override;
    static ContextService* GetInstance();
#endif // OHOS_BUILD_ENABLE_INTENTION_FRAMEWORK
};

class DragServerTest : public testing::Test {
public:
    static void SetUpTestCase();
    void SetUp();
    void TearDown();
    static std::shared_ptr<Media::PixelMap> CreatePixelMap(int32_t width, int32_t height);
    static std::optional<DragData> CreateDragData(int32_t sourceType, int32_t pointerId, int32_t dragNum,
        bool hasCoordinateCorrected, int32_t shadowNum);
    void AssignToAnimation(PreviewAnimation &animation);
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_TEST_H
