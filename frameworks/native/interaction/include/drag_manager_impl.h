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

#ifndef DRAG_MANAGER_IMPL_H
#define DRAG_MANAGER_IMPL_H

#include <functional>
#include <list>
#include <mutex>
#include <string>

#include "client.h"
#include "devicestatus_client.h"
#include "devicestatus_define.h"
#include "drag_data.h"
#include "i_drag_listener.h"
#include "i_start_drag_listener.h"
#include "i_subscript_listener.h"
#include "include/util.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManagerImpl {
public:
    DragManagerImpl() = default;
    ~DragManagerImpl() = default;

    int32_t StartDrag(const DragData &dragData, std::shared_ptr<IStartDragListener> listener);
    int32_t StopDrag(const DragDropResult &dropResult);
    int32_t OnNotifyResult(const StreamClient& client, NetPacket& pkt);
    int32_t OnNotifyHideIcon(const StreamClient& client, NetPacket& pkt);
    int32_t OnStateChangedMessage(const StreamClient& client, NetPacket& pkt);
    int32_t OnDragStyleChangedMessage(const StreamClient& client, NetPacket& pkt);
    int32_t AddDraglistener(DragListenerPtr listener);
    int32_t RemoveDraglistener(DragListenerPtr listener);
    int32_t AddSubscriptListener(SubscriptListenerPtr listener);
    int32_t RemoveSubscriptListener(SubscriptListenerPtr listener);
    int32_t SetDragWindowVisible(bool visible);
    int32_t UpdateDragStyle(DragCursorStyle style);
    int32_t UpdateShadowPic(const ShadowInfo &shadowInfo);
    int32_t GetDragTargetPid();
    int32_t GetDragState(DragState &dragState);
    int32_t GetUdKey(std::string &udKey);
    int32_t GetShadowOffset(int32_t& offsetX, int32_t& offsetY, int32_t& width, int32_t& height);
    int32_t GetDragData(DragData &dragData);

private:
    std::mutex mtx_;
    std::atomic_bool hasRegistered_ { false };
    std::atomic_bool hasSubscriptRegistered_ { false };
    std::list<DragListenerPtr> dragListener_;
    std::shared_ptr<IStartDragListener> StartDragListener_;
    std::list<SubscriptListenerPtr> subscriptListener_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MANAGER_IMPL_H
