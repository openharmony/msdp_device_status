/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef I_STOP_DRAG_LISTENER_H
#define I_STOP_DRAG_LISTENER_H

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class IStopDragListener {
public:
    IStopDragListener() = default;
    virtual ~IStopDragListener() = default;
    virtual void OnDragEndMessage() = 0;
};
}
} // namespace Msdp
} // namespace OHOS
#endif // I_STOP_DRAG_LISTENER_H