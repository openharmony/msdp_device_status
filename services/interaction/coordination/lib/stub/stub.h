#ifndef STUB_H
#define STUB_H

struct libinput_event;
struct libinput_device;
inline int libinput_event_get_type(struct libinput_event *) { return 0; }
inline struct  libinput_device *libinput_event_get_device(struct libinput_event *) { return 0; }
#define LIBINPUT_EVENT_POINTER_MOTION 1
#define LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE 2
#define LIBINPUT_EVENT_POINTER_BUTTON 3
#define LIBINPUT_EVENT_POINTER_AXIS 4

#endif