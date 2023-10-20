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

//! ExtraData data definitions of DRAG module.

use crate::{ input_binding, input_binding::CExtraData };
use fusion_data_rust::DragData;
use fusion_utils_rust::{ FusionResult, FusionErrorCode };

impl CExtraData {
    /// Create a CExtraData object.
    pub fn new(appended: bool) -> Self {
        CExtraData {
            appended,
            buffer: std::ptr::null(),
            buffer_size: 0usize,
            source_type: -1i32,
            pointer_id: -1i32,
        }
    }

    /// Set the appended property for CExtraData.
    pub fn set_appended(&mut self, appended: bool) -> &mut Self {
        self.appended = appended;
        self
    }

    /// Set the buffer property for CExtraData.
    pub fn set_buffer(&mut self, vec: &Vec<u8>) -> &mut Self {
        let vec_ptr = vec.as_ptr();
        self.buffer = vec_ptr;
        self.buffer_size = vec.len();
        self
    }

    /// Set the source type property for CExtraData.
    pub fn set_source_type(&mut self, source_type: i32) -> &mut Self {
        self.source_type = source_type;
        self
    }

    /// Set the pointer id property for CExtraData.
    pub fn set_pointer_id(&mut self, pointer_id: i32) -> &mut Self {
        self.pointer_id = pointer_id;
        self
    }
}

/// Struct extra data.
pub struct ExtraData {
    inner: CExtraData,
}

impl ExtraData {
    /// Create a ExtraData object.
    pub fn new(appended: bool) -> Self {
        Self {
            inner: CExtraData::new(appended)
        }
    }

    /// The extra data information is sent to the external subsystem.
    pub fn appended_extra_data(&mut self, allow_appended: bool, drag_data: DragData) -> FusionResult<()> {
        let buffer: &Vec<u8>= &drag_data.buffer;
        if buffer.is_empty() {
            return Err(FusionErrorCode::Fail);
        }
        self.inner.set_appended(allow_appended)
                  .set_buffer(buffer)
                  .set_source_type(drag_data.source_type)
                  .set_pointer_id(drag_data.pointer_id);

        // SAFETY:  No `None` here, cause `cextra_data` is valid.
        unsafe {
            input_binding::CAppendExtraData(&self.inner);
        }
        Ok(())
    }
}