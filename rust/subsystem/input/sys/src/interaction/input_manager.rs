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

#![allow(dead_code)]

use crate::{ input_binding, input_binding::OnPointerEventCallback };
use crate::fusion_utils_rust::{ FusionResult, FusionErrorCode };
use std::ffi::{ c_char, CString };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "RustInputManager"
};
const INPUT_BINDING_OK: i32 = 0;

/// struct ExtraData
pub struct InputManager;

impl InputManager {
    /// add monitor.
    pub fn add_monitor(callback: OnPointerEventCallback) -> FusionResult<i32> {
        // SAFETY: no `None` here, cause `callback` is valid
        unsafe {
            if input_binding::CAddMonitor(callback) != INPUT_BINDING_OK {
                error!(LOG_LABEL, "failed to add monitor");
                return Err(FusionErrorCode::Fail);
            }
            Ok(0)
        }
    }

    /// set pointer visible.
    pub fn set_pointer_visible(visible: bool) -> FusionResult<i32> {
        // SAFETY: set pointer event visible
        unsafe {
            if input_binding::CSetPointerVisible(visible) != INPUT_BINDING_OK {
                error!(LOG_LABEL, "failed to set pointer visible");
                return Err(FusionErrorCode::Fail);
            }
            Ok(0)
        }
    }

    /// enable input device.
    pub fn enable_input_device(enable: bool) -> FusionResult<i32> {
        // SAFETY: enable input device
        unsafe {
            if input_binding::CEnableInputDevice(enable) != INPUT_BINDING_OK {
                error!(LOG_LABEL, "failed to enable input device");
                return Err(FusionErrorCode::Fail);
            }
            Ok(0)
        }
    }

    /// remove input event filter.
    pub fn remove_input_event_filter(filter_id: i32) -> FusionResult<i32> {
        // SAFETY: remove input event filter
        unsafe {
            if input_binding::CRemoveInputEventFilter(filter_id) != INPUT_BINDING_OK {
                error!(LOG_LABEL, "failed to remove input event filter");
                return Err(FusionErrorCode::Fail);
            }
            Ok(0)
        }
    }

    /// remove monitor.
    pub fn remove_monitor(monitor_id: i32) {
        // SAFETY: remove monitor
        unsafe {
            input_binding::CRemoveMonitor(monitor_id);
        }
    }

    /// remove interceptor.
    pub fn remove_interceptor(interceptor_id: i32) {
        // SAFETY: remove interceptor
        unsafe {
            input_binding::CRemoveInterceptor(interceptor_id);
        }
    }

    /// set pointer location.
    pub fn set_pointer_location(physical_x: i32, physical_y: i32) {
        // SAFETY: remove monitor
        unsafe {
            input_binding::CSetPointerLocation(physical_x, physical_y);
        }
    }
}