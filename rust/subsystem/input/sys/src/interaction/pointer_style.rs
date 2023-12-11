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

use std::ffi::{ c_char, CString };

use hilog_rust::{ debug, error, hilog, HiLogLabel, LogType };

use fusion_utils_rust::{ FusionResult, FusionErrorCode };

use crate::input_binding;
use crate::input_binding::CPointerStyle;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "RustPointerStyle"
};
const INPUT_BINDING_OK: i32 = 0;

impl Default for CPointerStyle {
    fn default() -> Self {
        Self::new()
    }
}

impl CPointerStyle {
    /// Create a CPointerStyle object.
    pub fn new() -> Self {
        Self {
            size: -1,
            color: 0,
            id: 0,
        }
    }
}

/// Struct pointer style.
pub struct PointerStyle {
    inner: CPointerStyle,
}

impl Default for PointerStyle {
    fn default() -> Self {
        Self::new()
    }
}

impl PointerStyle {
    /// Create a PointerStyle object.
    pub fn new() -> Self {
        Self {
            inner: CPointerStyle::default()
        }
    }

    /// Get the pointer style from the PointerStyle.
    pub fn pointer_style(&mut self) -> FusionResult<()> {
        // SAFETY:  no `None` here, cause `CPointerStyle` is valid.
        unsafe {
            if input_binding::CGetPointerStyle(&mut self.inner) != INPUT_BINDING_OK {
                error!(LOG_LABEL, "Get pointer style failed");
                return Err(FusionErrorCode::Fail);
            }
            debug!(LOG_LABEL, "Get pointer style success");
            Ok(())
        }
    }
}