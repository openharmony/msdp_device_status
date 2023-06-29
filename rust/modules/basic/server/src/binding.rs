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

//! Fusion drag-IPC binding

use std::default::Default;
use std::ffi::{ c_char };

/// type alias OnAllocSocketFd
pub type OnAllocSocketFd = unsafe extern "C" fn (
    program_name: *const c_char, module_type: i32, client_fd: *mut i32, token_type: *mut i32
) -> i32;

extern "C" {
    fn AllocSocketFd(program_name: *const c_char, module_type: i32,
        client_fd: *mut i32, token_type: *mut i32) -> i32;
}

/// struct FusionBasicOperations
#[derive(Clone)]
#[repr(C)]
pub struct FusionBasicOperations {
    pub on_alloc_socket_fd: Option<OnAllocSocketFd>,
}

impl Default for FusionBasicOperations {
    fn default() -> Self {
        Self {
            on_alloc_socket_fd: Some(AllocSocketFd),
        }
    }
}
