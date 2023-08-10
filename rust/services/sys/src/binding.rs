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

//! Binding of native service implementation.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, c_int, CString };
use std::os::fd::RawFd;
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_data_rust::FusionResult;
use fusion_utils_rust::call_debug_enter;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionNativeService"
};

/// Represent service instance.
#[repr(C)]
struct NativeService {
    _private: [u8; 0],
}

extern "C" {
    /// Call to create an instance of service.
    fn NativeService_New() -> *mut NativeService;
    /// Call to increase reference count of the service instance.
    fn NativeService_Ref(service: *mut NativeService) -> *mut NativeService;
    /// Call to decrease reference count of the service instance.
    fn NativeService_Unref(service: *mut NativeService) -> *mut NativeService;
    /// Called on dump operation.
    fn NativeService_OnDump(service: *mut NativeService);
    /// Called when service is starting.
    fn NativeService_OnStart(service: *mut NativeService);
    /// Called when service is stopping.
    fn NativeService_OnStop(service: *mut NativeService);
    /// Call to allocate socket pair for client/server communication.
    fn NativeService_AllocSocketFd(service: *mut NativeService,
        program_name: *const c_char, module_type: i32,
        client_fd: *mut i32, token_type: *mut i32) -> i32;
}

pub struct FusionNativeService {
    service: *mut NativeService,
}

impl FusionNativeService {
    pub fn is_valid(&self) -> bool
    {
        !self.service.is_null()
    }

    pub fn on_start(&self)
    {
        call_debug_enter!("FusionNativeService:on_start");
        unsafe { NativeService_OnStart(self.service) };
    }

    pub fn on_stop(&self)
    {
        call_debug_enter!("FusionNativeService:on_stop");
        unsafe { NativeService_OnStop(self.service) };
    }

    pub fn alloc_socket_fd(&self, program_name: &str, module_type: i32,
        client_fd: &mut RawFd, token_type: &mut i32) -> FusionResult<i32>
    {
        call_debug_enter!("FusionNativeService:alloc_socket_fd");
        let mut fd: c_int = 0;

        let ret = unsafe {
            NativeService_AllocSocketFd(self.service, program_name.as_ptr() as *const c_char,
                module_type, &mut fd, token_type)
        };
        if ret == 0 {
            *client_fd = fd as RawFd;
            Ok(0)
        } else {
            error!(LOG_LABEL, "Failed to allocate socket pair");
            Err(ret)
        }
    }
}

impl Default for FusionNativeService {
    fn default() -> Self
    {
        Self {
            service: unsafe {
                NativeService_New()
            },
        }
    }
}

impl Drop for FusionNativeService {
    fn drop(&mut self)
    {
        unsafe {
            NativeService_Unref(self.service);
        }
    }
}

impl Clone for FusionNativeService {
    fn clone(&self) -> Self
    {
        Self {
            service: unsafe {
                NativeService_Ref(self.service)
            },
        }
    }
}