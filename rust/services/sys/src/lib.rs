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

//! Drag Server implementation.

#![allow(dead_code)]
#![allow(unused_variables)]

extern crate fusion_data_rust;
extern crate fusion_utils_rust;

use std::ffi::{ c_char, c_int, CString };
use std::ops::{ Deref, DerefMut };
use std::os::fd::RawFd;
use std::sync::Mutex;
use hilog_rust::{ info, error, hilog, HiLogLabel, LogType };
use fusion_data_rust::{ FusionResult };
use fusion_utils_rust::{ call_debug_enter };

const LOG_LABEL: HiLogLabel = HiLogLabel {
    log_type: LogType::LogCore,
    domain: 0xD002220,
    tag: "FusionService"
};

/// Represent service instance.
#[repr(C)]
pub struct FusionService {
    _private: [u8; 0],
}

extern "C" {
    /// Call to create an instance of service.
    pub fn FusionServiceNew() -> *mut FusionService;
    /// Call to increase reference count of the service instance.
    pub fn FusionServiceRef(service: *mut FusionService) -> *mut FusionService;
    /// Call to decrease reference count of the service instance.
    pub fn FusionServiceUnref(service: *mut FusionService) -> *mut FusionService;
    /// Called on dump operation.
    pub fn FusionService_OnDump(service: *mut FusionService);
    /// Called when service is starting.
    pub fn FusionService_OnStart(service: *mut FusionService);
    /// Called when service is stopping.
    pub fn FusionService_OnStop(service: *mut FusionService);
    /// Call to allocate socket pair for client/server communication.
    pub fn FusionService_AllocSocketFd(service: *mut FusionService,
        program_name: *const c_char, module_type: i32,
        client_fd: *mut i32, token_type: *mut i32) -> i32;
}

struct FusionServiceHolder {
    service: *mut FusionService,
}

impl FusionServiceHolder {
    fn is_valid(&self) -> bool
    {
        !self.service.is_null()
    }
}

impl Default for FusionServiceHolder {
    fn default() -> Self
    {
        Self {
            service: unsafe {
                FusionServiceNew()
            },
        }
    }
}

impl Drop for FusionServiceHolder {
    fn drop(&mut self)
    {
        unsafe {
            FusionServiceUnref(self.service);
        }
    }
}

impl Clone for FusionServiceHolder {
    fn clone(&self) -> Self
    {
        Self {
            service: unsafe {
                FusionServiceRef(self.service)
            },
        }
    }
}

/// Proxy for device status service.
#[derive(Default)]
pub struct FusionServiceProxy {
    service_holder: Mutex<FusionServiceHolder>,
}

impl FusionServiceProxy {
    unsafe fn is_none_global_instance() -> bool
    {
        if let Ok(guard) = FUSION_SERVICE.lock() {
            if let Some(proxy) = guard.deref() {
                if let Ok(holder_guard) = proxy.service_holder.lock() {
                    !holder_guard.is_valid()
                } else {
                    error!(LOG_LABEL, "Lock error of holder");
                    false
                }
            } else {
                true
            }
        } else {
            error!(LOG_LABEL, "Lock error of proxy");
            false
        }
    }

    unsafe fn set_global_instance()
    {
        if let Ok(mut guard) = FUSION_SERVICE.lock() {
            let proxy_option = guard.deref_mut();
            let need_set = match proxy_option {
                Some(proxy) => {
                    if let Ok(holder_guard) = proxy.service_holder.lock() {
                        !holder_guard.is_valid()
                    } else {
                        error!(LOG_LABEL, "Lock error of holder");
                        false
                    }
                }
                None => {
                    true
                }
            };
            if need_set {
                info!(LOG_LABEL, "new FusionServiceProxy instance");
                *proxy_option = Some(FusionServiceProxy::default())
            }
        } else {
            error!(LOG_LABEL, "Lock error of proxy");
        }
    }

    /// TODO: add documentation.
    pub fn get_instance() -> &'static Mutex<Option<FusionServiceProxy>> {
        unsafe {
            if Self::is_none_global_instance() {
                Self::set_global_instance();
            }
            &FUSION_SERVICE
        }
    }

    /// TODO: add documentation.
    pub fn get_mut_instance() -> &'static mut Mutex<Option<FusionServiceProxy>> {
        unsafe {
            if Self::is_none_global_instance() {
                Self::set_global_instance();
            }
            &mut FUSION_SERVICE
        }
    }

    /// Called when service is starting.
    pub fn on_start(&self)
    {
        call_debug_enter!("FusionServiceProxy:on_start");
        match self.service_holder.lock() {
            Ok(guard) => {
                unsafe {
                    FusionService_OnStart(guard.service)
                };
            }
            Err(_) => {
                error!(LOG_LABEL, "Locking error");
            }
        }
    }

    /// Called when service is stopping.
    pub fn on_stop(&self)
    {
        call_debug_enter!("FusionServiceProxy:on_stop");
        match self.service_holder.lock() {
            Ok(guard) => {
                unsafe {
                    FusionService_OnStop(guard.service)
                };
            }
            Err(_) => {
                error!(LOG_LABEL, "Locking error");
            }
        }
    }

    /// Call to allocate socket pair for client/server communication.
    pub fn alloc_socket_fd(&self, program_name: &str, module_type: i32,
        client_fd: &mut RawFd, token_type: &mut i32) -> FusionResult<i32>
    {
        call_debug_enter!("FusionServiceProxy:alloc_socket_fd");
        match self.service_holder.lock() {
            Ok(guard) => {
                let mut fd: c_int = 0;

                let ret = unsafe {
                    FusionService_AllocSocketFd(guard.service, program_name.as_ptr() as *const c_char,
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
            Err(_) => {
                error!(LOG_LABEL, "Locking error");
                Err(-1)
            }
        }
    }
}

impl Clone for FusionServiceProxy {
    fn clone(&self) -> Self
    {
        let service = match self.service_holder.lock() {
            Ok(guard) => {
                unsafe {
                    FusionServiceRef(guard.service)
                }
            }
            Err(_) => {
                error!(LOG_LABEL, "Locking error");
                std::ptr::null_mut()
            }
        };
        Self {
            service_holder: Mutex::new(FusionServiceHolder {
                service,
            }),
        }
    }
}

unsafe impl Send for FusionServiceProxy {}

static mut FUSION_SERVICE: Mutex<Option<FusionServiceProxy>> = Mutex::new(None);
