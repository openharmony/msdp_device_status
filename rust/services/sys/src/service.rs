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

//! Implementation of device status service.

#![allow(dead_code)]
#![allow(unused_variables)]

use std::ffi::{ c_char, CString };
use std::os::fd::RawFd;
use std::sync::{ Mutex, Once };
use hilog_rust::{ error, hilog, HiLogLabel, LogType };
use fusion_utils_rust::{ call_debug_enter, FusionResult, FusionErrorCode };
use crate::binding::FusionNativeService;

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionService"
};

#[derive(Default)]
struct FusionServiceImpl {
    native_service: FusionNativeService,
}

impl FusionServiceImpl {
    fn on_start(&self)
    {
        call_debug_enter!("FusionServiceImpl:on_start");
        self.native_service.on_start();
    }

    fn on_stop(&self)
    {
        call_debug_enter!("FusionServiceImpl:on_stop");
        self.native_service.on_stop();
    }

    fn alloc_socket_fd(&self, program_name: &str, module_type: i32,
        client_fd: &mut RawFd, token_type: &mut i32) -> FusionResult<()>
    {
        call_debug_enter!("FusionServiceImpl:alloc_socket_fd");
        self.native_service.alloc_socket_fd(program_name, module_type, client_fd, token_type)
    }
}

/// Proxy for device status service.
#[derive(Default)]
pub struct FusionService {
    service_impl: Mutex<FusionServiceImpl>,
}

impl FusionService {
    /// Get the single instance of [`FusionService`].
    pub fn get_instance() -> Option<&'static Self> {
        static mut FUSION_SERVICE_PROXY: Option<FusionService> = None;
        static INIT_ONCE: Once = Once::new();
        unsafe {
            INIT_ONCE.call_once(|| {
                FUSION_SERVICE_PROXY = Some(Self::default());
            });
            FUSION_SERVICE_PROXY.as_ref()
        }
    }

    /// Called when service is starting.
    pub fn on_start(&self)
    {
        match self.service_impl.lock() {
            Ok(guard) => {
                guard.on_start();
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {}", err);
            }
        }
    }

    /// Called when service is stopping.
    pub fn on_stop(&self)
    {
        match self.service_impl.lock() {
            Ok(guard) => {
                guard.on_stop();
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {}", err);
            }
        }
    }

    /// Call to allocate socket pair for client/server communication.
    pub fn alloc_socket_fd(&self, program_name: &str, module_type: i32,
        client_fd: &mut RawFd, token_type: &mut i32) -> FusionResult<()>
    {
        match self.service_impl.lock() {
            Ok(guard) => {
                guard.alloc_socket_fd(program_name, module_type, client_fd, token_type)
            }
            Err(err) => {
                error!(LOG_LABEL, "lock error: {}", err);
                Err(FusionErrorCode::Fail)
            }
        }
    }
}
