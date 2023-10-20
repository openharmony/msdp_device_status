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

#ifndef STATE_CHANGE_NOTIFY_H
#define STATE_CHANGE_NOTIFY_H

#include <list>
#include <memory>
#include <mutex>

#include "drag_data.h"
#include "stream_session.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class StateChangeNotify final {
public:
    StateChangeNotify() = default;
    ~StateChangeNotify() = default;

    struct MessageInfo {
        MessageId msgId { MessageId::INVALID };
        SessionPtr session { nullptr };
        DragState state { DragState::ERROR };
        bool operator==(std::shared_ptr<MessageInfo> info)
        {
            if (info == nullptr || info->session == nullptr) {
                return false;
            }
            return session->GetPid() == info->session->GetPid();
        }
    };

    void AddNotifyMsg(std::shared_ptr<MessageInfo> info);
    void RemoveNotifyMsg(std::shared_ptr<MessageInfo> info);
    int32_t StateChangedNotify(DragState state);

private:
    void OnStateChangedNotify(SessionPtr session, MessageId msId, DragState state);

private:
    std::list<std::shared_ptr<MessageInfo>> msgInfos_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // STATE_CHANGE_NOTIFY_H
