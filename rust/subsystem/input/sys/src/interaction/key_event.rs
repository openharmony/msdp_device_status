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

use crate::{ input_binding, input_binding::CKeyEvent };

/// KeyEvent packed the native CKeyEvent.
#[repr(C)]
pub struct KeyEvent(*const CKeyEvent);

impl KeyEvent {
    /// Create a KeyEvent object.
    pub fn new(key_event: *const CKeyEvent) -> Self {
        Self(key_event)
    }

    /// Extract a raw `CKeyEvent` pointer from this wrapper.
    /// # Safety
    pub unsafe fn as_inner(&self) -> *const CKeyEvent {
        self.0
    }

    /// Create an `KeyEvent` wrapper object from a raw `CKeyEvent` pointer.
    pub fn from_raw(c_key_event: *const CKeyEvent) -> Option<Self> {
        if c_key_event.is_null() {
            return None;
        }
        Some(Self(c_key_event))
    }
}

impl KeyEvent {
    /// Add flag to KeyEvent.
    pub fn add_flag(&self) {
        // SAFETY:
        // Rust KeyEvent always hold a valid native CKeyEvent.
        unsafe {
            input_binding::CKeyEventAddFlag(self.as_inner())
        }
    }

    /// Get the key code from the KeyEvent.
    pub fn key_code(&self) -> i32 {
        // SAFETY:
        // Rust KeyEvent always hold a valid native CKeyEvent.
        unsafe {
            input_binding::CGetKeyCode(self.as_inner())
        }
    }
}