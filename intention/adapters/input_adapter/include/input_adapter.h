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

#ifndef INPUT_ADAPTER_H
#define INPUT_ADAPTER_H

#include "input_manager.h"
#include "i_input_event_consumer.h"
#include "i_input_event_filter.h"

namespace OHOS {
namespace Msdp {
namespace DeviceStatus {
class InputAdapter {
    struct PointerFilter : public MMI::IInputEventFilter {
        explicit PointerFilter(std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : callback_(cb) {}
        bool OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        bool OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override
        {
            return false;
        }
        inline void UpdateCurrentFilterId(int32_t filterId)
        {
            filterId_ = filterId;
        }
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_;
        mutable int32_t filterId_ { -1 };
    };

    class InterceptorConsumer : public MMI::IInputEventConsumer {
    public:
        explicit InterceptorConsumer(std::function<void (std::shared_ptr<MMI::PointerEvent>)> pointerCallback,
            std::function<void (std::shared_ptr<MMI::KeyEvent>)> keyCallback)
            : pointerCallback_(pointerCallback),
              keyCallback_(keyCallback) {}
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> pointerCallback_;
        std::function<void (std::shared_ptr<MMI::keyEvent>)> keyCallback_;
    };

    class MonitorConsumer : public MMI::IInputEventConsumer {
    public:
        explicit MonitorConsumer(std::function<void (std::shared_ptr<MMI::PointerEvent>)> cb) : callback_(cb) {}
        void OnInputEvent(std::shared_ptr<MMI::KeyEvent> keyEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::PointerEvent> pointerEvent) const override;
        void OnInputEvent(std::shared_ptr<MMI::AxisEvent> axisEvent) const override;
    private:
        std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback_;
    };
public:
    int32_t AddMonitor(std::function<void (std::shared_ptr<MMI::PointerEvent>)> callback);
    int32_t AddInterceptor(std::function<void (std::shared_ptr<MMI::PointerEvent>)> pointerCallback,
        std::function<void (std::shared_ptr<MMI::KeyEvent>)> keyCallback, uint32_t deviceTags);
    int32_t AddFilter(std::function<void(std::shared_ptr<MMI::PointerEvent>)> callback);
    void RemoveMonitor();
    void RemoveInterceptor();
    void RemoveFilter(int32_t filterId);
private:
    int32_t monitorId_ { -1 };
    int32_t interceptorId_ { -1 };
    Channel<CooperateEvent>::Sender sender_;
};
} // namespace DeviceStatus
} // namespace Msdp
} // namespace OHOS
#endif // INPUT_ADAPTER_H