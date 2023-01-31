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

#ifndef DRAG_MESSAGE_H
#define DRAG_MESSAGE_H

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
enum class DragMessage {
    MSG_DRAG_STATE_ERROR,
    MSG_DRAG_STATE_START,
    MSG_DRAG_STATE_STOP,
    MSG_DRAG_STATE_CANCEL
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // DRAG_MESSAGE_H
