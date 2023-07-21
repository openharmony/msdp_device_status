/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

use crate::{
    input_binding, input_binding::CPointerEvent
};

/// PointerEvent packed the native CPointerEvent
#[repr(C)]
pub struct PointerEvent(*const CPointerEvent);

impl PointerEvent {
    /// Create a PointerEvent object
    pub fn new(pointer_event: *const CPointerEvent) -> Self {
        Self(pointer_event)
    }

    /// Extract a raw `CPointerEvent` pointer from this wrapper
    /// # Safety
    pub unsafe fn as_inner(&self) -> *const CPointerEvent {
        self.0
    }

    /// Create an `PointerEvent` wrapper object from a raw `CPointerEvent` pointer.
    pub fn from_raw(c_pointer_event: *const CPointerEvent) -> Option<Self> {
        if c_pointer_event.is_null() {
            return None;
        }
        Some(Self(c_pointer_event))
    }
}

impl PointerEvent {
    /// get pointer id
    pub fn pointer_id(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetPointerId(self.as_inner())
        }
    }

    /// get pointer action
    pub fn pointer_acttion(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetPointerAction(self.as_inner())
        }
    }

    /// get target window id
    pub fn target_window_id(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetTargetWindowId(self.as_inner())
        }
    }

    /// get source type
    pub fn source_type(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetSourceType(self.as_inner())
        }
    }

    /// get taget display id
    pub fn taget_display_id(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetTargetDisplayId(self.as_inner())
        }
    }

    /// get display x
    pub fn display_x(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetDisplayX(self.as_inner())
        }
    }

    /// get display y
    pub fn display_y(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetDisplayY(self.as_inner())
        }
    }

    /// get device id
    pub fn device_id(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetDeviceId(self.as_inner())
        }
    }

    /// get window pid
    pub fn window_pid(&self) -> i32 {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CGetWindowPid(self.as_inner())
        }
    }

    /// pointer add flag
    pub fn add_flag(&self) {
        // SAFETY:
        // Rust PointerEvent always hold a valid native CPointerEvent.
        unsafe {
            input_binding::CPointerEventAddFlag(self.as_inner())
        }
    }
}