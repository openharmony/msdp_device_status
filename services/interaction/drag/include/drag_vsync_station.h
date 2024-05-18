/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef DRAG_VSYNC_STATION_H
#define DRAG_VSYNC_STATION_H

#include <memory>
#include <mutex>
#include <vector>

#include "vsync_receiver.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
using DragFrameCallback = std::function<void(uint64_t)>;

enum FrameRequestType {
    TYPE_FLUSH_DRAG_POSITION = 1,
    TYPE_PERFORM_ANIMATION = 2,
    REQUEST_TYPE_MAX
};

class DragVSyncStation {
public:
    int32_t RequestFrame(int32_t frameType, std::shared_ptr<DragFrameCallback> callback,
        std::shared_ptr<AppExecFwk::EventHandler> handler);
    void StopVSyncRequest();
    uint64_t GetVSyncPeriod();

private:
    int32_t Init(std::shared_ptr<AppExecFwk::EventHandler> hander);
    void OnVSyncInner(uint64_t nanoTimestamp);
    static void OnVSync(uint64_t nanoTimestamp, void *client);
    std::shared_ptr<Rosen::VSyncReceiver> receiver_ {nullptr};
    std::map<int32_t, std::shared_ptr<DragFrameCallback>> vSyncCallbacks_;
    Rosen::VSyncReceiver::FrameCallback frameCallback_;
    uint64_t vSyncPeriod_ {0};
    std::mutex mtx_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_VSYNC_STATION_H