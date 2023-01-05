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

#ifndef STUB_H
#define STUB_H

#define LIBINPUT_EVENT_POINTER_MOTION 1
#define LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE 2
#define LIBINPUT_EVENT_POINTER_BUTTON 3
#define LIBINPUT_EVENT_POINTER_AXIS 4

struct libinput_event;
struct libinput_device;
static inline int libinput_event_get_type(struct libinput_event *)
{
    return 0;
}
static inline struct libinput_device *libinput_event_get_device(struct libinput_event *)
{
    return 0;
}
#endif