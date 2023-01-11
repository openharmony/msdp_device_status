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
#ifndef DRAG_MANAGER_IMPL_H
#define DRAG_MANAGER_IMPL_H

#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <optional>

#include "nocopyable.h"
#include "client.h"
#include "drag_data.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class DragManagerImpl final {
public:
    static DragManagerImpl &GetInstance();

    DISALLOW_COPY_AND_MOVE(DragManagerImpl);

    ~DragManagerImpl() = default;

    int32_t StartDrag(const DragData &dragData, std::function<void(int32_t&)> callback);

    int32_t StopDrag(int32_t &dragResult);

    bool InitClient();

private:
    void SetCallback(std::function<void(int32_t&)> callback);
    std::function<void(int32_t&)> GetCallback();
    
private:
    DragManagerImpl() = default;
    std::mutex mtx_;
    IClientPtr client_ { nullptr };
    std::function<void(int32_t&)> stopCallback_ { nullptr };
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#define DragMgrImpl OHOS::Msdp::DeviceStatus::DragManagerImpl::GetInstance()
#endif // DRAG_MANAGER_IMPL_H
