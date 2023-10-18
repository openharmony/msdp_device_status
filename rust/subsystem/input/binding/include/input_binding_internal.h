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

#ifndef INPUT_BINDING_INTERNAL_H
#define INPUT_BINDING_INTERNAL_H

#include "input_binding.h"

struct CPointerEvent {
    std::shared_ptr<OHOS::MMI::PointerEvent> event;

    CPointerEvent(std::shared_ptr<OHOS::MMI::PointerEvent> pointerEvent)
    {
        event = pointerEvent;
    }
};

struct CKeyEvent {
    std::shared_ptr<OHOS::MMI::KeyEvent> event;

    CKeyEvent(std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent)
    {
        event = keyEvent;
    }
};

struct CAxisEvent {
    std::shared_ptr<OHOS::MMI::AxisEvent> event;
};

struct CPointerStyle {
    int32_t size { -1 };
    int32_t color { 0 };
    int32_t id { 0 };
};

struct CExtraData {
    bool appended { false };
    uint8_t* buffer;
    size_t bufferSize;
    int32_t sourceType { -1 };
    int32_t pointerId { -1 };
};

class DragMonitorConsumer : public OHOS::MMI::IInputEventConsumer {
public:
    explicit DragMonitorConsumer(void (*cb)(CPointerEvent *));

    void OnInputEvent(std::shared_ptr<OHOS::MMI::PointerEvent> pointerEvent) const override;
    void OnInputEvent(std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) const override;
    void OnInputEvent(std::shared_ptr<OHOS::MMI::AxisEvent> axisEvent) const override;

private:
    void (*callback_)(CPointerEvent *);
};

#endif // INPUT_BINDING_INTERNAL_H